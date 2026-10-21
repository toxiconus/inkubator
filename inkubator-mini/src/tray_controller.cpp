#include "tray_controller.h"
#include "config_manager.h"
#include <RTClib.h>
#include <ArduinoJson.h>

extern RTC_DS3231 rtc;
extern bool rtc_ok;
extern unsigned long long incubation_start_unix;
extern unsigned long long get_incubation_elapsed_ms();
extern void save_rotation_state();

static bool tray_signal_active_low(const TrayState& st) {
  if (st.index < 0 || st.index >= MAX_TRAYS) {
    return false;
  }
  return configMgr.cfg.trays[st.index].signal_active_low;
}

static uint8_t tray_off_duty(const TrayState& st) {
  return tray_signal_active_low(st) ? 255 : 0;
}

static uint8_t tray_gpio_off_level(const TrayState& st) {
  return tray_signal_active_low(st) ? HIGH : LOW;
}

static uint8_t tray_pwm_duty_from_speed(const TrayState& st, int pwm_speed_0_255) {
  int speed = constrain(pwm_speed_0_255, 0, 255);
  return tray_signal_active_low(st) ? (uint8_t)(255 - speed) : (uint8_t)speed;
}

static void tray_apply_pwm_on(TrayState& st, int pwm_speed_0_255) {
  st.pin_active = true;
  if (st.pin_pwm_channel >= 0) {
    ledcWrite(st.pin_pwm_channel, tray_pwm_duty_from_speed(st, pwm_speed_0_255));
  }
}

static void tray_force_output_off(TrayState& st) {
  st.pin_active = false;
  st.current_pwm = 0;
  if (st.pin_pwm_channel >= 0) {
    ledcWrite(st.pin_pwm_channel, tray_off_duty(st));
  }
}

TrayState tray_states[MAX_TRAYS];
unsigned long long global_start_millis = 0;

void tray_init(unsigned long long base_start_millis) {
  global_start_millis = base_start_millis;

  for (int i = 0; i < MAX_TRAYS; i++) {
    TrayConfig& cfg_t = configMgr.cfg.trays[i];
    TrayState& st = tray_states[i];

    st.index = i;
    st.active = cfg_t.active && (i < configMgr.cfg.tray_count);
    st.pin = cfg_t.pin;
    st.species_info = get_species(cfg_t.species);
    st.manual_move_until_ms = 0;
    st.has_last_rotation = false;
    st.last_rotation_day = 0;
    st.last_rotation_hour = 0;
    st.last_rotation_minute = 0;
    st.last_rotation_elapsed_ms = 0;
    st.rotation_event_pending = false;

    // Start bazowy (offset dnia stosujemy w obliczeniu current_day)
    st.start_millis = 0ULL;

    const unsigned long init_interval_ms = (cfg_t.rotation_interval_ms < 1000UL) ? 1000UL : cfg_t.rotation_interval_ms;

    if (st.pin >= 0) {
      pinMode(st.pin, OUTPUT);
      digitalWrite(st.pin, tray_gpio_off_level(st));
      st.pin_pwm_channel = -1;  // Zainicjalizuj w setup()
      // Pierwszy obrót ustalony dopiero w tray_update_state().
      st.next_rotation_ms = 0ULL;
      st.rotation_start_ms = 0;
      st.currently_rotating = false;
      st.current_pwm = 0;
    }

    Serial.printf("[TRAY %d] %s | %s | offset: %d dni | pin: %d | %s\n",
      i,
      cfg_t.label,
      st.species_info ? st.species_info->name : "???",
      cfg_t.start_day_offset,
      st.pin,
      st.active ? "AKTYWNA" : "NIEAKTYWNA");
  }
}

void tray_update_state(TrayState& st, unsigned long long now_millis) {
  if (!st.active || !st.species_info) {
    // Nie napędzaj tacy, gdy jest nieaktywna lub nie ma gatunku.
    st.manual_move_until_ms = 0;
    st.currently_rotating = false;
    tray_force_output_off(st);
    st.current_day = 0;
    st.in_lockdown = false;
    st.should_rotate = false;
    return;
  }

  const SpeciesInfo* sp = st.species_info;
  TrayConfig& cfg = configMgr.cfg.trays[st.index];

  // Oblicz upływ czasu od startu tacy.
  long long elapsed_ms = 0;
  if (cfg.start_unix > 0ULL && rtc_ok) {
    DateTime now = rtc.now();
    unsigned long long now_unix = now.unixtime();
    if (now_unix >= cfg.start_unix) {
      elapsed_ms = (long long)((now_unix - cfg.start_unix) * 1000ULL);
    } else {
      elapsed_ms = 0;
    }
  } else {
    elapsed_ms = (long long)now_millis;
  }

  if (elapsed_ms < 0) elapsed_ms = 0;

  int elapsed_day = (int)(elapsed_ms / 86400000LL) + 1;
  st.current_day = elapsed_day + configMgr.cfg.trays[st.index].start_day_offset;

  if (st.current_day < 1) st.current_day = 1;

  if (st.current_day > sp->total_days) {
    st.current_day = sp->total_days;
  }

  st.days_remaining = (st.current_day >= sp->total_days) ? 0 : (sp->total_days - st.current_day + 1);
  st.today = get_day(sp, st.current_day);

  if (!st.today) return;

  st.in_lockdown = st.today->lockdown;
  st.should_rotate = st.today->turn && !st.today->lockdown;

  st.target_humidity_min = st.today->humidity_min;
  st.target_humidity_max = st.today->humidity_max;
  st.target_temp = st.today->temp_set;
  st.target_humidity = st.today->humidity_set;

  // --- PWM ROTATION STATE MACHINE ---
  const unsigned long safe_interval_ms = (cfg.rotation_interval_ms < 1000UL) ? 1000UL : cfg.rotation_interval_ms;
  const unsigned long safe_duration_ms = (cfg.rotation_duration_ms < 100UL) ? 100UL : cfg.rotation_duration_ms;
  
  // Ręczny ruch z UI
  if (st.manual_move_until_ms > 0) {
    if (now_millis < st.manual_move_until_ms) {
      st.current_pwm = cfg.rotation_pwm_speed;
      tray_apply_pwm_on(st, st.current_pwm);
      return;
    } else {
      // Manual właśnie wygasł: wymuś OFF i nie wpadaj w auto-rotate w tej samej iteracji.
      st.manual_move_until_ms = 0;
      st.currently_rotating = false;
      tray_force_output_off(st);
      return;
    }
  }
  
  if (!st.should_rotate || st.in_lockdown) {
    st.manual_move_until_ms = 0;
    st.currently_rotating = false;
    tray_force_output_off(st);
    return;
  }
  
  // Jeśli pierwszy auto-obrót jeszcze nie ustalono, ustaw go na bezwzględny czas
  // w osi inkubacji: teraz + interval.
  if (!st.currently_rotating && st.next_rotation_ms == 0ULL) {
    st.next_rotation_ms = now_millis + (unsigned long long)safe_interval_ms;
  }

  // Czekamy na następny ruch
  if (!st.currently_rotating && now_millis >= st.next_rotation_ms) {
    st.currently_rotating = true;
    st.rotation_start_ms = now_millis;
    st.current_pwm = cfg.rotation_pwm_speed;
    st.pin_active = true;

    // Zapamiętaj znacznik czasu obrotu (dzień + HH:MM czasu inkubacji).
    unsigned long long elapsed_from_start = (unsigned long long)elapsed_ms;
    unsigned long long day_ms = elapsed_from_start % 86400000ULL;
    st.has_last_rotation = true;
    st.last_rotation_day = (int)(elapsed_from_start / 86400000ULL) + 1;
    st.last_rotation_hour = (int)(day_ms / 3600000ULL);
    st.last_rotation_minute = (int)((day_ms % 3600000ULL) / 60000ULL);
    st.last_rotation_elapsed_ms = elapsed_from_start;
    st.rotation_event_pending = true;

    tray_apply_pwm_on(st, st.current_pwm);
    Serial.printf("[TRAY %d] RUCH START (PWM: %d)\n", st.index, st.current_pwm);
  }
  
  // Ruch w trakcie
  if (st.currently_rotating) {
    unsigned long long elapsed = now_millis - st.rotation_start_ms;
    if (elapsed >= safe_duration_ms) {
      st.currently_rotating = false;
      tray_force_output_off(st);
      st.next_rotation_ms = now_millis + safe_interval_ms;
      Serial.printf("[TRAY %d] RUCH STOP (nastepny za %lu ms)\n", st.index, safe_interval_ms);
    }
  }
}

void tray_update_all(unsigned long long now_millis) {
  for (int i = 0; i < MAX_TRAYS; i++) {
    tray_update_state(tray_states[i], now_millis);
  }
}

void tray_set_species(int tray_idx, const char* species_id) {
  if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;

  strlcpy(configMgr.cfg.trays[tray_idx].species, species_id, 12);
  tray_states[tray_idx].species_info = get_species(species_id);
  configMgr.save();
}

bool tray_set_day(int tray_idx, int day, unsigned long long now_millis) {
  if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return false;
  if (day < 1) day = 1;

  const SpeciesInfo* sp = tray_states[tray_idx].species_info;
  if (!sp) return false;
  if (day > sp->total_days) day = sp->total_days;

  TrayState& st = tray_states[tray_idx];
  TrayConfig& cfg = configMgr.cfg.trays[tray_idx];

  long long elapsed_ms = 0LL;
  if (cfg.start_unix > 0ULL && rtc_ok) {
    DateTime now = rtc.now();
    unsigned long long now_unix = now.unixtime();
    if (now_unix >= cfg.start_unix) {
      elapsed_ms = (long long)((now_unix - cfg.start_unix) * 1000ULL);
    }
  } else {
    elapsed_ms = (now_millis >= st.start_millis) ?
                 (long long)(now_millis - st.start_millis) :
                 0LL;
  }
  int base_day = (int)(elapsed_ms / 86400000LL) + 1;

  // Offset koryguje dzień do wartości żądanej niezależnie od uptime.
  configMgr.cfg.trays[tray_idx].start_day_offset = day - base_day;

  // Resetuj ostatni obrót, bo zmiana dnia oznacza nową bazę czasową dla tej tacy.
  st.has_last_rotation = false;
  st.last_rotation_day = 0;
  st.last_rotation_hour = 0;
  st.last_rotation_minute = 0;
  st.last_rotation_elapsed_ms = 0;
  st.rotation_event_pending = false;
  save_rotation_state();

  configMgr.save();
  tray_update_state(st, now_millis);

  return true;
}

void tray_manual_move(int tray_idx, unsigned long duration_ms, unsigned long long now_millis) {
  if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;
  TrayState& st = tray_states[tray_idx];
  if (!st.active) return;
  TrayConfig& cfg = configMgr.cfg.trays[tray_idx];
  const unsigned long safe_interval_ms = (cfg.rotation_interval_ms < 1000UL) ? 1000UL : cfg.rotation_interval_ms;
  
  st.manual_move_until_ms = now_millis + (unsigned long long)duration_ms;
  // Po ręcznym ruchu odłóż auto-obrót o pełny interwał.
  st.next_rotation_ms = st.manual_move_until_ms + (unsigned long long)safe_interval_ms;
  st.currently_rotating = true;
  st.current_pwm = cfg.rotation_pwm_speed;

  // Zapamiętaj ręczne uruchomienie obrotu
  unsigned long long elapsed_from_start = (now_millis >= st.start_millis) ? (now_millis - st.start_millis) : 0ULL;
  unsigned long long day_ms = elapsed_from_start % 86400000ULL;
  st.has_last_rotation = true;
  st.last_rotation_day = (int)(elapsed_from_start / 86400000ULL) + 1;
  st.last_rotation_hour = (int)(day_ms / 3600000ULL);
  st.last_rotation_minute = (int)((day_ms % 3600000ULL) / 60000ULL);
  st.last_rotation_elapsed_ms = elapsed_from_start;
  st.rotation_event_pending = true;
  
  tray_apply_pwm_on(st, st.current_pwm);
  Serial.printf("[TRAY %d] Ręczny ruch: %lu ms, PWM=%d\n", tray_idx, duration_ms, st.current_pwm);
}

void tray_stop(int tray_idx, unsigned long long now_millis) {
  if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;

  TrayState& st = tray_states[tray_idx];

  st.manual_move_until_ms = 0;
  st.currently_rotating = false;
  // Po STOP odłóż auto-obrót o pełny interwał (nie 1s – inaczej auto-harmonogram
  // natychmiast wznawia ruch po ręcznym zatrzymaniu).
  TrayConfig& cfg_stop = configMgr.cfg.trays[tray_idx];
  const unsigned long stop_interval_ms = (cfg_stop.rotation_interval_ms < 1000UL) ? 1000UL : cfg_stop.rotation_interval_ms;
  st.next_rotation_ms = now_millis + (unsigned long long)stop_interval_ms;
  tray_force_output_off(st);
  Serial.printf("[TRAY %d] STOP natychmiastowy\n", tray_idx);
}

void tray_set_active(int tray_idx, bool active) {
  if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;

  configMgr.cfg.trays[tray_idx].active = active;
  tray_states[tray_idx].active = active;

  if (!active) {
    TrayState& st = tray_states[tray_idx];
    st.manual_move_until_ms = 0;
    st.currently_rotating = false;
    tray_force_output_off(st);
  }

  configMgr.save();
}

void tray_get_chamber_humidity(float& out_min, float& out_max) {
  out_min = 50.0f;
  out_max = 65.0f;

  bool any_lockdown = false;
  bool any_active = false;

  for (int i = 0; i < MAX_TRAYS; i++) {
    TrayState& st = tray_states[i];
    if (!st.active || !st.today) continue;

    any_active = true;
    if (st.in_lockdown) {
      any_lockdown = true;
      out_min = max(out_min, st.target_humidity_min);
      out_max = max(out_max, st.target_humidity_max);
    }
  }

  if (!any_lockdown && any_active) {
    // Gdy żadna taca nie jest w lockdownie, przyjmujemy zakres wspólny
    // dla wszystkich aktywnych tac. Jeśli zakresy są rozłączne, używamy
    // wartości pośredniej, by nadal zapewnić stabilność.
    float hi_min = 40.0f, lo_max = 80.0f;
    for (int i = 0; i < MAX_TRAYS; i++) {
      TrayState& st = tray_states[i];
      if (!st.active || !st.today) continue;
      hi_min = max(hi_min, st.target_humidity_min);
      lo_max = min(lo_max, st.target_humidity_max);
    }
    if (hi_min <= lo_max) {
      out_min = hi_min;
      out_max = lo_max;
    } else {
      // Zakresy są rozłączne; wybieramy szerszy kompromis zamiast bardzo wąskiego 2-stopniowego przedziału.
      out_min = max(40.0f, lo_max - 5.0f);
      out_max = min(80.0f, hi_min + 5.0f);
      if (out_min > out_max) {
        out_min = min(lo_max, hi_min);
        out_max = max(lo_max, hi_min);
      }
    }
  }
}

void tray_get_chamber_humidity_target(float& out_target_humidity) {
  float sum = 0.0f;
  int count = 0;

  for (int i = 0; i < MAX_TRAYS; i++) {
    TrayState& st = tray_states[i];
    if (!st.active || !st.today) continue;
    if (isfinite(st.target_humidity)) {
      sum += st.target_humidity;
      count++;
    }
  }

  if (count == 0) {
    out_target_humidity = NAN;
    return;
  }

  out_target_humidity = sum / (float)count;
}

void tray_get_chamber_temp(float& out_target_temp) {
  float sum = 0.0f;
  int count = 0;

  for (int i = 0; i < MAX_TRAYS; i++) {
    TrayState& st = tray_states[i];
    if (!st.active || !st.today) continue;
    sum += st.target_temp;
    count++;
  }

  // Fallback do ustawienia globalnego, gdy brak aktywnych tac.
  if (count == 0) {
    out_target_temp = configMgr.cfg.system.target_temp;
    return;
  }

  out_target_temp = sum / (float)count;
}

String tray_status_json(unsigned long long now_millis) {
  tray_update_all(now_millis);

  JsonDocument doc;
  JsonArray traysArr = doc["trays"].to<JsonArray>();

  for (int i = 0; i < MAX_TRAYS; i++) {
    TrayState& st = tray_states[i];
    TrayConfig& cfg = configMgr.cfg.trays[i];
    JsonObject t = traysArr.add<JsonObject>();

    long long ms_to_start = (long long)st.start_millis - (long long)now_millis;
    int hours_to_start = (ms_to_start > 0) ? (int)(ms_to_start / 3600000LL) : 0;

    t["index"] = i;
    t["active"] = st.active;
    t["label"] = configMgr.cfg.trays[i].label;
    t["species_id"] = st.species_info ? st.species_info->id : "?";
    t["species_name"] = st.species_info ? st.species_info->name : "?";
    t["total_days"] = st.species_info ? st.species_info->total_days : 0;
    t["current_day"] = st.current_day;
    t["days_remaining"] = st.days_remaining;
    t["hours_to_start"] = hours_to_start;
    t["in_lockdown"] = st.in_lockdown;
    t["should_rotate"] = st.should_rotate;
    t["pin"] = st.pin;
    t["pin_active"] = st.pin_active;
    t["hum_min"] = st.target_humidity_min;
    t["hum_max"] = st.target_humidity_max;
    t["target_humidity"] = st.target_humidity;
    t["target_temp"] = st.target_temp;
    t["notes"] = (st.today && st.today->notes[0]) ? st.today->notes : "";
    t["start_day_offset"] = configMgr.cfg.trays[i].start_day_offset;
    t["start_unix"] = cfg.start_unix;
    t["manual_move_active"] = (st.manual_move_until_ms > 0 && now_millis < st.manual_move_until_ms);
    t["manual_move_remaining_ms"] = (st.manual_move_until_ms > now_millis) 
                                    ? (unsigned long)(st.manual_move_until_ms - now_millis) : 0;
    t["moving"] = st.currently_rotating || (st.manual_move_until_ms > 0 && now_millis < st.manual_move_until_ms);
    t["has_last_rotation"] = st.has_last_rotation;
    t["last_rotation_day"] = st.last_rotation_day;
    t["last_rotation_hour"] = st.last_rotation_hour;
    t["last_rotation_minute"] = st.last_rotation_minute;
    t["last_rotation_elapsed_ms"] = st.last_rotation_elapsed_ms;
    if (st.has_last_rotation && cfg.start_unix > 0ULL && rtc_ok) {
      unsigned long long unix_rotation = cfg.start_unix + (st.last_rotation_elapsed_ms / 1000ULL);
      t["last_rotation_unix"] = unix_rotation;
    } else if (st.has_last_rotation && incubation_start_unix > 0ULL) {
      unsigned long long unix_rotation = incubation_start_unix + (st.last_rotation_elapsed_ms / 1000ULL);
      t["last_rotation_unix"] = unix_rotation;
    } else if (st.has_last_rotation) {
      unsigned long long now_sec = millis() / 1000ULL;
      unsigned long long unix_rotation = now_sec - (get_incubation_elapsed_ms() / 1000ULL) + (st.last_rotation_elapsed_ms / 1000ULL);
      t["last_rotation_unix"] = unix_rotation;
    } else {
      t["last_rotation_unix"] = 0ULL;
    }
  }

  float ch_min, ch_max;
  tray_get_chamber_humidity(ch_min, ch_max);
  JsonObject chamber = doc["chamber"].to<JsonObject>();
  chamber["humidity_min"] = ch_min;
  chamber["humidity_max"] = ch_max;
  float ch_target_humidity;
  tray_get_chamber_humidity_target(ch_target_humidity);
  if (isfinite(ch_target_humidity)) chamber["target_humidity"] = ch_target_humidity;

  String output;
  serializeJson(doc, output);
  return output;
}
