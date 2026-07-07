// tray_controller.cpp
#include "tray_controller.h"
#include "config_manager.h"
#include "incubation_profile.h"
#include <ArduinoJson.h>

TrayState tray_states[MAX_TRAYS];
unsigned long long global_start_millis = 0;

extern IncubationProfiles g_incubation_profiles;
extern bool rtc_ok;
extern unsigned long rtc_get_unix_time();

static void tray_update_day_from_rtc(int tray_idx, unsigned long long now_unix);

void tray_init(unsigned long long base_start_millis) {
    global_start_millis = base_start_millis;

    for (int i = 0; i < MAX_TRAYS; i++) {
        TrayConfig& cfg_t = configMgr.cfg.trays[i];
        TrayState& st = tray_states[i];

        st.index = i;
        st.active = cfg_t.active && (i < configMgr.cfg.tray_count);
        st.pin = cfg_t.pin;
        st.pin_pwm_channel = -1;
        st.pin_active = false;
        st.current_pwm = 0;
        st.manual_move_until_ms = 0;
        st.has_last_rotation = false;
        st.last_rotation_day = 0;
        st.last_rotation_hour = 0;
        st.last_rotation_minute = 0;
        st.last_rotation_elapsed_ms = 0;
        st.rotation_event_pending = false;

        st.incubation_profile = nullptr;
        st.current_day_data = IncubationDay();
        st.current_day = 1;
        st.days_remaining = 0;
        st.in_lockdown = false;
        st.should_rotate = true;
        st.target_temp = 37.8;
        st.target_humidity = 50.0f;
        st.target_humidity_min = 40.0f;
        st.target_humidity_max = 60.0f;

        st.start_millis = base_start_millis;
        st.last_day_check_ms = 0ULL;
        st.next_rotation_ms = 0ULL;
        st.rotation_start_ms = 0ULL;
        st.currently_rotating = false;
        st.manual_move_until_ms = 0ULL;

        if (st.pin >= 0) {
            pinMode(st.pin, OUTPUT);
            digitalWrite(st.pin, LOW);
        }

        String speciesId = String(cfg_t.species);
        speciesId.toLowerCase();
        st.incubation_profile = getSpeciesProfile(g_incubation_profiles, speciesId);
        if (st.incubation_profile) {
            st.current_day_data = getDayForSpecies(g_incubation_profiles, speciesId, st.current_day);
            st.target_temp = st.current_day_data.shell_temp;
            st.target_humidity = st.current_day_data.humidity_set;
            st.target_humidity_min = st.current_day_data.humidity_min;
            st.target_humidity_max = st.current_day_data.humidity_max;
            Serial.printf("[TRAY %d] Załadowano profil: %s (%d dni)\n",
                i, st.incubation_profile->common_name.c_str(), st.incubation_profile->incubation_days);
        } else {
            Serial.printf("[TRAY %d] Brak profilu dla gatunku: %s\n", i, cfg_t.species);
        }

        if (cfg_t.start_unix > 0 && rtc_ok) {
            unsigned long long now_unix = (unsigned long long)rtc_get_unix_time();
            if (now_unix > 0) {
                tray_update_day_from_rtc(i, now_unix);
            }
        }

        Serial.printf("[TRAY %d] %s | pin: %d | %s\n",
            i, cfg_t.label, st.pin, st.active ? "AKTYWNA" : "NIEAKTYWNA");
    }
}

void tray_update_all(unsigned long long now_millis) {
    for (int i = 0; i < MAX_TRAYS; i++) {
        tray_increment_day(i, now_millis);
        tray_update_state(tray_states[i], now_millis);
    }
}

void tray_update_state(TrayState& st, unsigned long long now_millis) {
    if (!st.active) {
        st.manual_move_until_ms = 0ULL;
        st.currently_rotating = false;
        digitalWrite(st.pin, LOW);
        st.current_day = 0;
        st.in_lockdown = false;
        st.should_rotate = false;
        return;
    }

    bool useProfile = (st.incubation_profile != nullptr && !st.incubation_profile->days.empty());
    if (useProfile) {
        int totalDays = st.incubation_profile->incubation_days;
        if (st.current_day > totalDays) st.current_day = totalDays;
        if (st.current_day < 1) st.current_day = 1;

        String speciesKey = String(configMgr.cfg.trays[st.index].species);
        speciesKey.toLowerCase();
        st.current_day_data = getDayForSpecies(g_incubation_profiles, speciesKey, st.current_day);
        st.target_temp = st.current_day_data.shell_temp;
        st.target_humidity = st.current_day_data.humidity_set;
        st.target_humidity_min = st.current_day_data.humidity_min;
        st.target_humidity_max = st.current_day_data.humidity_max;
        st.should_rotate = st.current_day_data.turning;
        st.in_lockdown = !st.current_day_data.turning;
    }

    const TrayConfig& cfg = configMgr.cfg.trays[st.index];
    unsigned long interval = cfg.rotation_interval_ms;
    unsigned long duration = cfg.rotation_duration_ms;
    if (interval < 1000UL) interval = 1000UL;
    if (duration < 100UL) duration = 100UL;

    if (st.manual_move_until_ms > 0ULL) {
        if (now_millis < st.manual_move_until_ms) {
            st.currently_rotating = true;
            digitalWrite(st.pin, HIGH);
            return;
        }
        st.manual_move_until_ms = 0ULL;
        st.currently_rotating = false;
        digitalWrite(st.pin, LOW);
        return;
    }

    if (!st.should_rotate || st.in_lockdown) {
        st.currently_rotating = false;
        digitalWrite(st.pin, LOW);
        return;
    }

    if (!st.currently_rotating && st.next_rotation_ms == 0ULL) {
        st.next_rotation_ms = now_millis + interval;
    }

    if (!st.currently_rotating && now_millis >= st.next_rotation_ms) {
        st.currently_rotating = true;
        st.rotation_start_ms = now_millis;
        digitalWrite(st.pin, HIGH);
        st.next_rotation_ms = now_millis + interval;
    }

    if (st.currently_rotating && now_millis - st.rotation_start_ms >= duration) {
        st.currently_rotating = false;
        digitalWrite(st.pin, LOW);
    }
}

void tray_manual_move(int tray_idx, unsigned long duration_ms, unsigned long long now_millis) {
    if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;
    TrayState& st = tray_states[tray_idx];
    st.manual_move_until_ms = now_millis + duration_ms;
    st.currently_rotating = true;
    digitalWrite(st.pin, HIGH);
}

void tray_stop(int tray_idx, unsigned long long now_millis) {
    if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;
    TrayState& st = tray_states[tray_idx];
    st.manual_move_until_ms = 0;
    st.currently_rotating = false;
    digitalWrite(st.pin, LOW);
}

void tray_set_species(int tray_idx, const char* species_id) {
    if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;
    strcpy(configMgr.cfg.trays[tray_idx].species, species_id);
    String lookup = String(species_id);
    lookup.toLowerCase();
    tray_states[tray_idx].incubation_profile = getSpeciesProfile(g_incubation_profiles, lookup);
    configMgr.save();
}

bool tray_set_day(int tray_idx, int day, unsigned long long now_millis) {
    if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return false;
    tray_states[tray_idx].current_day = day;
    return true;
}

void tray_set_active(int tray_idx, bool active) {
    if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;
    tray_states[tray_idx].active = active;
    configMgr.cfg.trays[tray_idx].active = active;
    configMgr.save();
}

void tray_update_days(unsigned long long now_millis) {
    for (int i = 0; i < MAX_TRAYS; i++) {
        tray_increment_day(i, now_millis);
    }
}

void tray_increment_day(int tray_idx, unsigned long long now_millis) {
    if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;
    TrayState& st = tray_states[tray_idx];
    if (!st.active || !st.incubation_profile) return;

    const TrayConfig& cfg = configMgr.cfg.trays[tray_idx];
    if (cfg.start_unix > 0 && rtc_ok) {
        unsigned long long now_unix = (unsigned long long)rtc_get_unix_time();
        if (now_unix > 0) {
            tray_update_day_from_rtc(tray_idx, now_unix);
            return;
        }
    }

    if (st.last_day_check_ms == 0ULL) {
        st.last_day_check_ms = now_millis;
        if (st.start_millis == 0ULL) st.start_millis = now_millis;
        return;
    }

    const unsigned long long one_day_ms = 86400000ULL;
    if (now_millis - st.last_day_check_ms < one_day_ms) return;

    unsigned long long elapsed = now_millis - st.last_day_check_ms;
    int days_passed = elapsed / one_day_ms;
    if (days_passed <= 0) return;

    int total_days = st.incubation_profile->incubation_days;
    int new_day = st.current_day + days_passed;
    if (new_day > total_days) new_day = total_days;
    if (new_day != st.current_day) {
        st.current_day = new_day;
        Serial.printf("[TRAY %d] Dzień inkubacji zaktualizowany do %d\n", st.index, st.current_day);
    }
    st.last_day_check_ms += (unsigned long long)days_passed * one_day_ms;
}

static void tray_update_day_from_rtc(int tray_idx, unsigned long long now_unix) {
    if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;
    TrayState& st = tray_states[tray_idx];
    const TrayConfig& cfg = configMgr.cfg.trays[tray_idx];
    if (!st.active || !st.incubation_profile) return;
    if (cfg.start_unix == 0) return;
    if (now_unix < cfg.start_unix) {
        st.current_day = 1;
    } else {
        unsigned long long elapsed_seconds = now_unix - cfg.start_unix;
        int elapsed_days = elapsed_seconds / 86400UL;
        int new_day = elapsed_days + 1;
        if (new_day < 1) new_day = 1;
        if (new_day > st.incubation_profile->incubation_days) {
            new_day = st.incubation_profile->incubation_days;
        }
        if (new_day != st.current_day) {
            st.current_day = new_day;
            Serial.printf("[TRAY %d] Dzień zaktualizowany z RTC na %d\n", st.index, st.current_day);
        }
    }

    String speciesKey = String(configMgr.cfg.trays[tray_idx].species);
    speciesKey.toLowerCase();
    st.current_day_data = getDayForSpecies(g_incubation_profiles, speciesKey, st.current_day);
    st.target_temp = st.current_day_data.shell_temp;
    st.target_humidity = st.current_day_data.humidity_set;
    st.target_humidity_min = st.current_day_data.humidity_min;
    st.target_humidity_max = st.current_day_data.humidity_max;
    st.should_rotate = st.current_day_data.turning;
    st.in_lockdown = !st.current_day_data.turning;
}

void tray_update_days_from_rtc() {
    if (!rtc_ok) return;
    unsigned long long now_unix = (unsigned long long)rtc_get_unix_time();
    if (now_unix == 0) return;
    for (int i = 0; i < MAX_TRAYS; i++) {
        tray_update_day_from_rtc(i, now_unix);
    }
}

void tray_set_start_unix(int tray_idx, unsigned long long unix) {
    if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;
    configMgr.cfg.trays[tray_idx].start_unix = unix;
    configMgr.save();
    if (rtc_ok) {
        unsigned long long now_unix = (unsigned long long)rtc_get_unix_time();
        if (now_unix > 0) {
            tray_update_day_from_rtc(tray_idx, now_unix);
        }
    }
}

void tray_reset_day(int tray_idx) {
    if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;
    if (rtc_ok) {
        unsigned long long now_unix = (unsigned long long)rtc_get_unix_time();
        if (now_unix > 0) {
            tray_set_start_unix(tray_idx, now_unix);
            return;
        }
    }
    // Fallback: ustaw dzień na 1, pozostaw start_unix w 0
    configMgr.cfg.trays[tray_idx].start_unix = 0;
    configMgr.save();
    tray_states[tray_idx].current_day = 1;
}

unsigned long long tray_get_start_unix(int tray_idx) {
    if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return 0;
    return configMgr.cfg.trays[tray_idx].start_unix;
}

void tray_get_chamber_humidity(float& out_min, float& out_max) {
    out_min = 45.0f; out_max = 65.0f;
}

void tray_get_chamber_humidity_target(float& out_target_humidity) {
    out_target_humidity = 55.0f;
}

void tray_get_chamber_temp(float& out_target_temp) {
    out_target_temp = 37.7f;
}

String tray_status_json(unsigned long long now_millis) {
    DynamicJsonDocument doc(2048);
    JsonArray trays = doc["trays"].to<JsonArray>();
    for (int i = 0; i < MAX_TRAYS; i++) {
        JsonObject t = trays.add<JsonObject>();
        TrayState& st = tray_states[i];
        t["index"] = i;
        t["active"] = st.active;
        t["current_day"] = st.current_day;
        t["target_temp"] = st.target_temp;
        t["target_humidity"] = st.target_humidity;
        t["should_rotate"] = st.should_rotate;
        t["in_lockdown"] = st.in_lockdown;
        t["start_unix"] = configMgr.cfg.trays[i].start_unix;
        if (st.incubation_profile) {
            t["species_id"] = st.incubation_profile->id;
            t["species_name"] = st.incubation_profile->common_name;
            t["total_days"] = st.incubation_profile->incubation_days;
        }
    }
    String output;
    serializeJson(doc, output);
    return output;
}