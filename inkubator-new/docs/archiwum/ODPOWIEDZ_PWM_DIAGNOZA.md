# Diagnoza PWM tacy

## 1. Wymuszona logika active-high
Źródło: firmware/include/tray_controller.h (linia ~47)

```cpp
static inline bool tray_signal_active_low(const TrayState& st) {
  (void)st;
  // Ten sterownik w tym projekcie pracuje stabilnie w logice active-high.
  // Wymuszenie eliminuje efekt: ciągły wolny obrót / pisk po STOP.
  return false;
}
```

## 2. Cała funkcja tray_off_duty
Źródło: firmware/include/tray_controller.h (linia ~55)

```cpp
static inline uint8_t tray_off_duty(const TrayState& st) {
  return tray_signal_active_low(st) ? 255 : 0;
}
```

Dla obecnego `return false` daje to jednoznacznie `OFF = 0`.

## 3. Funkcja kończąca ruch po timerze (auto stop)
Źródło: firmware/include/tray_controller.h (linia ~216)

```cpp
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
```

`tray_force_output_off` wykonuje `ledcWrite(..., tray_off_duty(...))`:

```cpp
static inline void tray_force_output_off(TrayState& st) {
  st.pin_active = false;
  st.current_pwm = 0;
  if (st.pin_pwm_channel >= 0) {
    ledcWrite(st.pin_pwm_channel, tray_off_duty(st));
  }
  if (st.pin >= 0) {
    digitalWrite(st.pin, tray_gpio_off_level(st));
  }
}
```

## 4. Funkcja natychmiastowego STOP (manual)
Źródło: firmware/include/tray_controller.h (linia ~297)

```cpp
void tray_stop(int tray_idx, unsigned long long now_millis) {
  if (tray_idx < 0 || tray_idx >= MAX_TRAYS) return;

  TrayState& st = tray_states[tray_idx];

  st.manual_move_until_ms = 0;
  st.currently_rotating = false;
  TrayConfig& cfg_stop = configMgr.cfg.trays[tray_idx];
  const unsigned long stop_interval_ms = (cfg_stop.rotation_interval_ms < 1000UL) ? 1000UL : cfg_stop.rotation_interval_ms;
  st.next_rotation_ms = now_millis + (unsigned long long)stop_interval_ms;
  tray_force_output_off(st);
  Serial.printf("[TRAY %d] STOP natychmiastowy\n", tray_idx);
}
```

## 5. Aktualne wartości rotation_pwm_speed z configu
Źródło: firmware/data/config.json

- trays[0].rotation_pwm_speed = 102
- trays[1].rotation_pwm_speed = 102
- trays[2].rotation_pwm_speed = 102 (taca nieaktywna)

Czyli nie 255 i nie 128, tylko 102 (około 40%).

## Wniosek diagnostyczny

- Scenariusz A: mało prawdopodobny, bo przy `return false` funkcja `tray_off_duty` zwraca 0, nie 255.
- Scenariusz B: w kodzie STOP po timerze jest i wywołuje `ledcWrite(..., 0)` przez `tray_force_output_off`, więc sam brak wywołania też nie wygląda na źródło.

Najbardziej prawdopodobne źródło problemu pozostaje po stronie sterownika/pinów (np. zachowanie mostka H przy PWM=0, stan linii kierunku, floating lub wymagane twarde odcięcie EN/IN).
