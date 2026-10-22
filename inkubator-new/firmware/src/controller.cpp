// controller.cpp – uproszczona wersja
#include "controller.h"
#include "config_manager.h"
#include "tray_controller.h"

float heater_pwm_pct = 0.0f;
bool heater_manual_mode[2] = {false, false};
float heater_manual_pct[2] = {0.0f, 0.0f};
bool diffuser_manual_mode = false;
bool diffuser_manual_on[2] = {false, false};
bool diffuser_on = false;
int diffuser_active_idx = -1;
String heater_profile_name = "balanced";

// Moc grzałek liczymy w całym kodzie w procentach (float 0..100). Skalę sprzętu zna tylko
// ta funkcja (LEDC: 25 kHz, 10 bitów, 0..HEATER_PWM_MAX). Niepełną wartość wypełnienia
// rozkładamy w czasie (dithering), więc efektywna rozdzielczość jest dużo lepsza niż 1/1023.
static float heater_dither_err[4] = {0.0f, 0.0f, 0.0f, 0.0f};

void heater_write_pct(int ch, float pct) {
    if (ch < 0 || ch >= 4) return;
    pct = constrain(pct, 0.0f, 100.0f);
    if (pct <= 0.0f) {
        heater_dither_err[ch] = 0.0f;
        ledcWrite(ch, 0);
        return;
    }
    const float exact = pct * HEATER_PWM_MAX / 100.0f + heater_dither_err[ch];
    int duty = (int)floorf(exact + 0.5f);
    duty = constrain(duty, 0, HEATER_PWM_MAX);
    heater_dither_err[ch] = constrain(exact - duty, -1.0f, 1.0f);
    ledcWrite(ch, duty);
}

// ----------------------------------------------------------------------------
//  Czujnik sterujący i nastawy
//  Można wskazać, który DS18B20 steruje grzałką (control_sensor_index), także taki,
//  który dotyka skorupy jaj (control_sensor_on_shell). Wtedy nastawą jest temperatura
//  skorupy z profilu dnia, a dla czujnika powietrza temperatura powietrza z profilu.
//  Nastawy z profilu są używane tylko przy use_program_setpoint = true.
// ----------------------------------------------------------------------------
float control_temperature_value(const SensorData& data) {
    const SensorConfig& sc = configMgr.cfg.sensors;
    int idx = constrain(sc.control_sensor_index, 0, MAX_SENSORS - 1);
    if (idx >= sc.ds18b20_count) idx = 0;
    return data.temp[idx];
}

float control_setpoint_temp() {
    const float fallback = configMgr.cfg.system.target_temp;
    if (!configMgr.cfg.system.use_program_setpoint) return fallback;
    const bool shell = configMgr.cfg.sensors.control_sensor_on_shell;
    float sum = 0.0f;
    int n = 0;
    for (int i = 0; i < MAX_TRAYS; i++) {
        const TrayState& st = tray_states[i];
        if (!st.active || !st.incubation_profile) continue;
        const float v = shell ? st.current_day_data.shell_temp : st.current_day_data.air_temp_target;
        if (v > 30.0f && v < 42.0f) { sum += v; n++; }
    }
    return n > 0 ? sum / n : fallback;
}

// Zakres wilgotności: wartość idealna oraz granice dopuszczalne (z konfiguracji lub z profilu dnia).
static void control_humidity_band(float& ideal, float& lo, float& hi) {
    ideal = configMgr.cfg.system.target_humidity;
    const float hyst = max(0.5f, configMgr.cfg.system.humidity_hysteresis);
    lo = ideal - hyst;
    hi = ideal + hyst;
    if (!configMgr.cfg.system.use_program_setpoint) return;
    float si = 0.0f, sl = 0.0f, sh = 0.0f;
    int n = 0;
    for (int i = 0; i < MAX_TRAYS; i++) {
        const TrayState& st = tray_states[i];
        if (!st.active || !st.incubation_profile) continue;
        const IncubationDay& d = st.current_day_data;
        if (d.humidity_set > 0.0f && d.humidity_max > d.humidity_min) {
            si += d.humidity_set; sl += d.humidity_min; sh += d.humidity_max; n++;
        }
    }
    if (n > 0) { ideal = si / n; lo = sl / n; hi = sh / n; }
}

static void heaters_control_simple(const SensorData& data) {
    float target = control_setpoint_temp();
    float current = control_temperature_value(data);
    if (current < -50 || current > 80) { heater_pwm_pct = 0; return; }
    float diff = target - current;
    float hyst = configMgr.cfg.system.temp_hysteresis;
    if (diff > hyst) heater_pwm_pct = min(100.0f, heater_pwm_pct + 5.0f);
    else if (diff < -hyst) heater_pwm_pct = max(0.0f, heater_pwm_pct - 3.0f);
    else heater_pwm_pct = max(0.0f, heater_pwm_pct - 0.5f);
    for (int ch = 0; ch < 2; ch++) {
        int pin = (ch == 0) ? configMgr.cfg.system.heater_pin_1 : configMgr.cfg.system.heater_pin_2;
        if (pin < 0) continue;
        heater_write_pct(ch, heater_manual_mode[ch] ? heater_manual_pct[ch] : heater_pwm_pct);
    }
}

bool set_heater_profile(const String& mode) {
    if (mode == "balanced" || mode == "fast" || mode == "eco") {
        heater_profile_name = mode;
        return true;
    }
    return false;
}

bool set_diffuser_profile(const String& mode) {
    if (mode == "balanced" || mode == "fast" || mode == "eco") {
        strcpy(configMgr.cfg.diffusers.profile, mode.c_str());
        return true;
    }
    return false;
}

#include "calibration.h"

// ============================================================================
//  TRYB 1: REGULATOR ADAPTACYJNY Z UCZENIEM
//  Przeniesiony z inkubator-mini. Włączany przez control_mode = 1
//  (komenda setControlMode); tryb 0 zostaje prostym regulatorem z histerezą.
// ============================================================================

bool learning_mode = false;
String learning_profile = "";

static unsigned long diffuser_change_ms = 0;
static int diffuser_next_idx = 0;

#define TEMP_LEARN_SAMPLES 180
#define TEMP_LEARN_STABLE_SAMPLES 120
#define TEMP_LEARN_STDDEV_THRESHOLD 0.08f
#define TEMP_LEARN_ERROR_THRESHOLD 0.15f

#define HUM_LEARN_SAMPLES 36
#define HUM_LEARN_STDDEV_THRESHOLD 0.8f
#define HUM_LEARN_ERROR_THRESHOLD 1.0f
#define HUM_LEARN_MIN_CYCLE_MS 1000
#define HUM_LEARN_MAX_CYCLE_MS 1800000   // naturalne cykle dyfuzora trwają minuty

struct TempSample { float temp; int pwm_raw; float target; };
static TempSample temp_history[TEMP_LEARN_SAMPLES] = {};
static int temp_history_head = 0;
static int temp_history_count = 0;
static bool temp_learned_valid = false;
static int temp_learned_pwm_raw = -1;
static float temp_learned_target = 0.0f;
static bool temp_stable = false;
static float temp_last_target = NAN;

struct HumSample { float humidity; float target; };
static HumSample hum_history[HUM_LEARN_SAMPLES] = {};
static int hum_history_head = 0;
static int hum_history_count = 0;
static unsigned long hum_on_durations[HUM_LEARN_SAMPLES] = {};
static unsigned long hum_off_durations[HUM_LEARN_SAMPLES] = {};
static int hum_duration_count = 0;
static bool hum_learned_valid = false;
static int hum_learned_on_ms = 0;
static int hum_learned_off_ms = 0;
static float hum_learned_target = 0.0f;
static float hum_last_target = NAN;
static bool hum_stable = false;
static bool hum_last_diffuser_on = false;
static unsigned long hum_last_diffuser_change_ms = 0;
static bool learning_mode_previous = false;
static float heater_delta_avg = 0.7f;

struct LearningRecord {
    bool active = false;
    unsigned long start_ms = 0;
    unsigned long duration_ms = 0;
    unsigned long interval_ms = 1000UL;
    unsigned long last_sample_ms = 0;
    int sample_count = 0;
};
static LearningRecord learning_record;

int heater_pin_for_index(int idx) {
    switch (idx) {
        case 0: return configMgr.cfg.system.heater_pin_1;
        case 1: return configMgr.cfg.system.heater_pin_2;
        default: return -1;
    }
}

static inline void diffuser_write_pin(int pin, bool on) {
    if (pin < 0) return;
    digitalWrite(pin, on ? LOW : HIGH);
}

static void record_temp_sample(float temp, float target, int pwm_raw) {
    temp_history[temp_history_head].temp = temp;
    temp_history[temp_history_head].target = target;
    temp_history[temp_history_head].pwm_raw = pwm_raw;
    temp_history_head = (temp_history_head + 1) % TEMP_LEARN_SAMPLES;
    if (temp_history_count < TEMP_LEARN_SAMPLES) temp_history_count++;
}

static bool temp_history_stable(float target) {
    if (temp_history_count < TEMP_LEARN_STABLE_SAMPLES) return false;
    float mean = 0.0f;
    for (int i = 0; i < TEMP_LEARN_STABLE_SAMPLES; i++) {
        int idx = (temp_history_head - 1 - i + TEMP_LEARN_SAMPLES) % TEMP_LEARN_SAMPLES;
        mean += temp_history[idx].temp;
    }
    mean /= TEMP_LEARN_STABLE_SAMPLES;

    float variance = 0.0f;
    float error_sum = 0.0f;
    for (int i = 0; i < TEMP_LEARN_STABLE_SAMPLES; i++) {
        int idx = (temp_history_head - 1 - i + TEMP_LEARN_SAMPLES) % TEMP_LEARN_SAMPLES;
        float diff = temp_history[idx].temp - mean;
        variance += diff * diff;
        error_sum += fabs(temp_history[idx].target - temp_history[idx].temp);
    }
    float stddev = sqrt(variance / TEMP_LEARN_STABLE_SAMPLES);
    float avg_error = error_sum / TEMP_LEARN_STABLE_SAMPLES;
    return stddev < TEMP_LEARN_STDDEV_THRESHOLD && avg_error < TEMP_LEARN_ERROR_THRESHOLD &&
           fabs(mean - target) < TEMP_LEARN_ERROR_THRESHOLD;
}

static int calculate_learned_pwm() {
    if (temp_history_count == 0) return -1;
    float sum = 0.0f;
    int count = 0;
    for (int i = 0; i < TEMP_LEARN_SAMPLES; i++) {
        int idx = (temp_history_head - 1 - i + TEMP_LEARN_SAMPLES) % TEMP_LEARN_SAMPLES;
        if (temp_history[idx].pwm_raw >= 0) {
            sum += temp_history[idx].pwm_raw;
            count++;
        }
    }
    return count > 0 ? (int)round(sum / count) : -1;
}

static void record_hum_sample(float humidity, float target) {
    hum_history[hum_history_head].humidity = humidity;
    hum_history[hum_history_head].target = target;
    hum_history_head = (hum_history_head + 1) % HUM_LEARN_SAMPLES;
    if (hum_history_count < HUM_LEARN_SAMPLES) hum_history_count++;
}

static bool hum_history_stable(float target) {
    if (hum_history_count < HUM_LEARN_SAMPLES) return false;
    float mean = 0.0f;
    for (int i = 0; i < HUM_LEARN_SAMPLES; i++) {
        int idx = (hum_history_head - 1 - i + HUM_LEARN_SAMPLES) % HUM_LEARN_SAMPLES;
        mean += hum_history[idx].humidity;
    }
    mean /= HUM_LEARN_SAMPLES;

    float variance = 0.0f;
    float error_sum = 0.0f;
    for (int i = 0; i < HUM_LEARN_SAMPLES; i++) {
        int idx = (hum_history_head - 1 - i + HUM_LEARN_SAMPLES) % HUM_LEARN_SAMPLES;
        float diff = hum_history[idx].humidity - mean;
        variance += diff * diff;
        error_sum += fabs(hum_history[idx].target - hum_history[idx].humidity);
    }
    float stddev = sqrt(variance / HUM_LEARN_SAMPLES);
    float avg_error = error_sum / HUM_LEARN_SAMPLES;
    return stddev < HUM_LEARN_STDDEV_THRESHOLD && avg_error < HUM_LEARN_ERROR_THRESHOLD &&
           fabs(mean - target) < HUM_LEARN_ERROR_THRESHOLD;
}

static void record_hum_cycle(bool on, unsigned long duration_ms) {
    if (hum_duration_count >= HUM_LEARN_SAMPLES) {
        memmove(&hum_on_durations[0], &hum_on_durations[1], sizeof(hum_on_durations) - sizeof(hum_on_durations[0]));
        memmove(&hum_off_durations[0], &hum_off_durations[1], sizeof(hum_off_durations) - sizeof(hum_off_durations[0]));
        hum_duration_count = HUM_LEARN_SAMPLES - 1;
    }
    hum_on_durations[hum_duration_count] = on ? duration_ms : 0;
    hum_off_durations[hum_duration_count] = on ? 0 : duration_ms;
    hum_duration_count++;
}

static void calculate_learned_hum_cycle() {
    unsigned long total_on = 0, total_off = 0;
    int count_on = 0, count_off = 0;
    for (int i = 0; i < HUM_LEARN_SAMPLES; i++) {
        if (hum_on_durations[i] > 0) { total_on += hum_on_durations[i]; count_on++; }
        if (hum_off_durations[i] > 0) { total_off += hum_off_durations[i]; count_off++; }
    }
    if (count_on > 0 && count_off > 0) {
        int on_ms = (int)(total_on / count_on);
        int off_ms = (int)(total_off / count_off);
        hum_learned_on_ms = constrain(on_ms, HUM_LEARN_MIN_CYCLE_MS, HUM_LEARN_MAX_CYCLE_MS);
        hum_learned_off_ms = constrain(off_ms, HUM_LEARN_MIN_CYCLE_MS, HUM_LEARN_MAX_CYCLE_MS);
    }
}

void reset_learning_state() {
    temp_history_head = 0;
    temp_history_count = 0;
    temp_learned_valid = false;
    temp_learned_pwm_raw = -1;
    temp_learned_target = 0.0f;
    temp_stable = false;
    temp_last_target = NAN;
    hum_history_head = 0;
    hum_history_count = 0;
    memset(hum_on_durations, 0, sizeof(hum_on_durations));
    memset(hum_off_durations, 0, sizeof(hum_off_durations));
    hum_duration_count = 0;
    hum_learned_valid = false;
    hum_learned_on_ms = 0;
    hum_learned_off_ms = 0;
    hum_learned_target = 0.0f;
    hum_last_target = NAN;
    hum_stable = false;
    hum_last_diffuser_on = false;
    hum_last_diffuser_change_ms = 0;
    learning_record.active = false;
    learning_record.start_ms = 0;
    learning_record.duration_ms = 0;
    learning_record.last_sample_ms = 0;
    learning_record.sample_count = 0;
}

void clear_learning_state() { reset_learning_state(); }

void start_learning_record(unsigned long duration_sec) {
    reset_learning_state();
    learning_record.active = true;
    learning_record.start_ms = millis();
    learning_record.duration_ms = duration_sec * 1000UL;
    learning_record.interval_ms = 1000UL;
    learning_record.last_sample_ms = 0;
    learning_record.sample_count = 0;
    learning_mode = true;
    learning_profile = heater_profile_name;
    Serial.printf("[LEARN] Recording started for %lu sec\n", duration_sec);
}

void stop_learning_record() {
    if (!learning_record.active) return;
    learning_record.active = false;
    learning_mode = false;
    learning_profile = "";
    Serial.printf("[LEARN] Recording stopped, samples=%d\n", learning_record.sample_count);
}

bool is_learning_record_active() { return learning_record.active; }

unsigned long learning_record_elapsed_sec() {
    if (!learning_record.active) return 0;
    return (millis() - learning_record.start_ms) / 1000UL;
}

int learning_record_samples_count() { return learning_record.sample_count; }

void handle_learning_record(const SensorData& data) {
    if (!learning_record.active) return;
    unsigned long now = millis();
    if (learning_record.last_sample_ms == 0) learning_record.last_sample_ms = now;
    if (now - learning_record.last_sample_ms < learning_record.interval_ms) return;

    float target = control_setpoint_temp();
    float hum_ideal, hum_lo, hum_hi;
    control_humidity_band(hum_ideal, hum_lo, hum_hi);
    float hum_target = hum_ideal;
    int pwm_raw = (int)round(constrain(heater_pwm_pct, 0.0f, 100.0f) * 10.23f);
    record_temp_sample(control_temperature_value(data), target, pwm_raw);
    if (data.humidity >= 0.0f) record_hum_sample(data.humidity, hum_target);
    learning_record.sample_count++;
    learning_record.last_sample_ms = now;

    if (learning_record.duration_ms > 0 && now - learning_record.start_ms >= learning_record.duration_ms) {
        stop_learning_record();
    }
}

static void heaters_control_adaptive(const SensorData& data) {
    const float temp_lower = control_temperature_value(data);
    const float target = control_setpoint_temp();

    // Drugi czujnik traktujemy jako górny czujnik komory tylko wtedy, gdy jest skonfigurowany,
    // nie jest czujnikiem akwarium i czujnik sterujący nie leży na skorupie jaj. W przeciwnym
    // razie regulator pracuje na jednym czujniku.
    const int control_idx = (configMgr.cfg.sensors.control_sensor_index >= configMgr.cfg.sensors.ds18b20_count)
                                ? 0 : constrain(configMgr.cfg.sensors.control_sensor_index, 0, MAX_SENSORS - 1);
    const int other_idx = (control_idx == 0) ? 1 : 0;
    const bool has_upper = configMgr.cfg.sensors.ds18b20_count >= 2 &&
                           configMgr.cfg.sensors.aquarium_sensor_index != other_idx &&
                           !configMgr.cfg.sensors.control_sensor_on_shell;
    const bool lower_ok = temp_lower > -50.0f && temp_lower < 80.0f;
    const bool upper_ok = !has_upper || (data.temp[other_idx] > -50.0f && data.temp[other_idx] < 80.0f);
    const float temp_upper = has_upper ? data.temp[other_idx] : temp_lower;

    // Bezpieczeństwo: błędny odczyt wyłącza grzałki
    if (!lower_ok || !upper_ok) {
        heater_pwm_pct = 0.0f;
        for (int ch = 0; ch < MAX_HEATERS_HW; ch++) {
            if (heater_pin_for_index(ch) >= 0) heater_write_pct(ch, 0.0f);
        }
        return;
    }

    // Adaptacyjna średnia różnicy dolny-górny
    if (has_upper) {
        float current_delta = temp_lower - temp_upper;
        if (current_delta > -2.0f && current_delta < 5.0f) {
            heater_delta_avg = 0.9f * heater_delta_avg + 0.1f * current_delta;
        }
    }

    const float diff_lower = target - temp_lower;
    const float adaptive_threshold = TEMP_DEADBAND + constrain(heater_delta_avg * 0.25f, 0.0f, 1.5f);

    if (learning_mode && !learning_mode_previous) reset_learning_state();
    learning_mode_previous = learning_mode;

    if (!isnan(temp_last_target) && fabs(target - temp_last_target) > 0.1f) {
        temp_learned_valid = false;
        temp_stable = false;
    }
    temp_last_target = target;

    const bool was_temp_stable = temp_stable;
    if (learning_mode && temp_history_stable(target)) {
        int learned_raw = calculate_learned_pwm();
        if (learned_raw >= 0) {
            temp_learned_pwm_raw = learned_raw;
            temp_learned_target = target;
            temp_learned_valid = true;
            temp_stable = true;
            if (!was_temp_stable) {
                configMgr.learning_state.has_value = true;
                configMgr.learning_state.learned_pwm_pct = temp_learned_pwm_raw / 10.23f;
                configMgr.learning_state.learned_target_temp = target;
                configMgr.learning_state.sample_count = max(temp_history_count, hum_history_count);
                configMgr.learning_state.confirmed_cycles++;
                configMgr.learning_state.last_updated_ms = millis();
                configMgr.save_learning_data();
            }
        }
    }

    float pwm_target = 0.0f;
    bool use_learned_pwm = false;
    if ((temp_learned_valid && fabs(temp_learned_target - target) < 0.1f) ||
        (!temp_learned_valid && configMgr.learning_state.has_value &&
         fabs(configMgr.learning_state.learned_target_temp - target) < 0.1f)) {
        if (diff_lower <= 0.5f) use_learned_pwm = true;
    }

    if (use_learned_pwm) {
        int learned_raw = temp_learned_valid ? temp_learned_pwm_raw
                                             : (int)round(configMgr.learning_state.learned_pwm_pct * 10.23f);
        float learned_target = temp_learned_valid ? temp_learned_target : configMgr.learning_state.learned_target_temp;
        if (fabs(learned_target - target) < 0.1f) {
            float error = target - temp_lower;
            float correction_gain = 20.0f;
            if (fabs(error) > 0.4f) correction_gain = 45.0f;
            else if (fabs(error) > 0.15f) correction_gain = 30.0f;
            int correction = (int)round(error * correction_gain);
            int pwm_raw = constrain(learned_raw + correction, 0, 1023);
            pwm_target = constrain(pwm_raw / 10.23f, 0.0f, 100.0f);
        }
    } else {
        if (diff_lower > adaptive_threshold) {
            pwm_target = 100.0f;
        } else if (diff_lower > 0.1f) {
            float range = adaptive_threshold - 0.1f;
            if (range < 0.1f) range = 0.1f;
            float progress = (adaptive_threshold - diff_lower) / range;
            pwm_target = 100.0f * (1.0f - progress);
        } else {
            float error_upper = target - temp_upper;
            pwm_target = (error_upper > 0.3f) ? 15.0f : 5.0f;
        }
    }

    // Ograniczenia bezpieczeństwa
    if (temp_lower > target + 0.4f) pwm_target = 0.0f;
    if (temp_upper > target + 1.0f) pwm_target = 0.0f;
    if (temp_lower >= configMgr.cfg.system.alarm_max_temp) pwm_target = 0.0f;

    // Łagodny start po uruchomieniu
    if (data.elapsed_ms < HEATER_RAMP_MS) {
        float ramp_limit = min(100.0f, (float)data.elapsed_ms / HEATER_RAMP_MS * 100.0f);
        if (pwm_target > ramp_limit) pwm_target = ramp_limit;
    }

    float step = (fabs(pwm_target - heater_pwm_pct) > 15.0f) ? 10.0f : 5.0f;
    if (pwm_target > heater_pwm_pct) heater_pwm_pct = min(100.0f, heater_pwm_pct + step);
    else if (pwm_target < heater_pwm_pct) heater_pwm_pct = max(0.0f, heater_pwm_pct - step);

    for (int ch = 0; ch < MAX_HEATERS_HW; ch++) {
        int pin = heater_pin_for_index(ch);
        if (pin < 0) continue;
        heater_write_pct(ch, heater_manual_mode[ch] ? heater_manual_pct[ch] : heater_pwm_pct);
    }
}

// ============================================================================
//  TRYB 2: REGULATOR PI Z WYPRZEDZENIEM (dla grzałek o małej bezwładności)
//  Regulator proporcjonalno-całkujący usuwa stały uchyb (temperatura nie "wisi"
//  poniżej celu), a człon całkujący ma ograniczenie przeciwdziałające nawijaniu.
//  Jeśli uczenie ma zapisaną moc dla danej nastawy, całka startuje od tej wartości.
//  Wzmocnienia są ostrożne (powolna całka); trzeba je dostroić na sprzęcie.
// ============================================================================
#define PI_KP 15.0f   // [% mocy na 1 °C błędu]
#define PI_KI 0.05f   // [% mocy na 1 °C·s], czas zdwojenia ok. 5 minut

static float pi_integral = 0.0f;
static unsigned long pi_last_ms = 0;

static void heaters_control_pi(const SensorData& data) {
    const float temp = control_temperature_value(data);
    const float target = control_setpoint_temp();

    if (!(temp > -50.0f && temp < 80.0f)) {
        heater_pwm_pct = 0.0f;
        pi_integral = 0.0f;
        pi_last_ms = 0;
        for (int ch = 0; ch < MAX_HEATERS_HW; ch++) {
            if (heater_pin_for_index(ch) >= 0) heater_write_pct(ch, 0.0f);
        }
        return;
    }

    const unsigned long now = millis();
    float dt = 1.0f;
    float feed_forward = 0.0f;
    if (configMgr.learning_state.has_value && fabs(configMgr.learning_state.learned_target_temp - target) < 0.2f) {
        feed_forward = constrain(configMgr.learning_state.learned_pwm_pct, 0.0f, 100.0f);
    }
    if (pi_last_ms == 0 || now - pi_last_ms > 10000UL) {
        pi_integral = feed_forward;      // start lub długa przerwa: startujemy od mocy z uczenia
    } else {
        dt = (now - pi_last_ms) / 1000.0f;
    }
    pi_last_ms = now;

    const float error = target - temp;
    const float p_term = PI_KP * error;
    float output = p_term + pi_integral;

    // Całkujemy tylko wtedy, gdy wyjście nie jest nasycone w stronę błędu
    const bool sat_high = output >= 100.0f && error > 0.0f;
    const bool sat_low = output <= 0.0f && error < 0.0f;
    if (!sat_high && !sat_low) {
        pi_integral = constrain(pi_integral + PI_KI * error * dt, 0.0f, 100.0f);
    }
    output = constrain(p_term + pi_integral, 0.0f, 100.0f);

    // Ograniczenia bezpieczeństwa
    if (temp > target + 0.4f) {
        output = 0.0f;
        pi_integral *= 0.9f;             // po przegrzaniu szybko zmniejszamy całkę
    }
    if (temp >= configMgr.cfg.system.alarm_max_temp) output = 0.0f;

    // Łagodny start po uruchomieniu urządzenia
    if (data.elapsed_ms < HEATER_RAMP_MS) {
        const float ramp_limit = min(100.0f, (float)data.elapsed_ms / HEATER_RAMP_MS * 100.0f);
        if (output > ramp_limit) output = ramp_limit;
    }

    heater_pwm_pct = output;
    for (int ch = 0; ch < MAX_HEATERS_HW; ch++) {
        if (heater_pin_for_index(ch) < 0) continue;
        heater_write_pct(ch, heater_manual_mode[ch] ? heater_manual_pct[ch] : heater_pwm_pct);
    }
}

// ============================================================================
//  DYFUZORY: STEROWANIE ZAKRESOWE (oszczędza styki włączników)
//  - w zakresie dopuszczalnym [cel - histereza, cel + histereza] nic nie robimy,
//  - poniżej dolnej granicy włączamy, a wyłączamy w połowie drogi między wartością idealną
//    (cel) a górną granicą: kompromis między dokładnością a liczbą przełączeń,
//  - minimalny czas pracy i przerwy (diffusers.on_time_sec / off_time_sec) chroni styki,
//  - dyfuzory włączamy na przemian, a oba naraz tylko przy dużym niedoborze wilgotności.
//  Działa tak samo we wszystkich trybach regulatora. Uczenie tylko obserwuje naturalne
//  cykle i nigdy nie wymusza przełączania.
// ============================================================================
unsigned long diffuser_switch_count = 0;
static bool diffuser_state_known = false;

static void diffusers_band_control(const SensorData& data) {
    DiffuserConfig& dc = configMgr.cfg.diffusers;
    if (dc.count == 0) return;

    // Tryb ręczny
    if (diffuser_manual_mode) {
        int manual_on_count = 0;
        for (int i = 0; i < dc.count && i < MAX_DIFFUSERS; i++) {
            if (dc.pins[i] < 0) continue;
            const bool on_this = !!diffuser_manual_on[i];
            diffuser_write_pin(dc.pins[i], on_this);
            if (on_this) manual_on_count++;
        }
        diffuser_on = (manual_on_count > 0);
        diffuser_active_idx = -1;
        return;
    }

    if (data.humidity < 0.0f) return;   // brak odczytu: stan bez zmian

    float ideal, rh_min, rh_max;                             // idealna oraz granice dopuszczalne
    control_humidity_band(ideal, rh_min, rh_max);
    const float hyst = max(0.5f, (rh_max - rh_min) / 2.0f);
    const unsigned long now = millis();
    const unsigned long min_on = max(1UL, (unsigned long)dc.on_time_sec) * 1000UL;
    const unsigned long min_off = max(1UL, (unsigned long)dc.off_time_sec) * 1000UL;

    bool want_on = diffuser_on;
    if (!diffuser_on && data.humidity <= rh_min) want_on = true;
    else if (diffuser_on && data.humidity >= ideal + 0.5f * (rh_max - ideal)) want_on = false;

    const unsigned long since = now - diffuser_change_ms;
    const bool may_change = !diffuser_state_known || since >= (diffuser_on ? min_on : min_off);

    if (want_on != diffuser_on && may_change) {
        if (learning_mode) {
            record_hum_cycle(diffuser_on, now - hum_last_diffuser_change_ms);
            if (hum_duration_count >= 4) {
                calculate_learned_hum_cycle();
                if (hum_learned_on_ms > 0 && hum_learned_off_ms > 0 &&
                    (configMgr.learning_state.diffuser_on_ms != hum_learned_on_ms ||
                     configMgr.learning_state.diffuser_off_ms != hum_learned_off_ms)) {
                    configMgr.learning_state.has_value = true;
                    configMgr.learning_state.diffuser_on_ms = hum_learned_on_ms;
                    configMgr.learning_state.diffuser_off_ms = hum_learned_off_ms;
                    configMgr.learning_state.learned_target_humidity = ideal;
                    configMgr.learning_state.last_updated_ms = now;
                    configMgr.save_learning_data();
                }
            }
        }
        diffuser_on = want_on;
        diffuser_change_ms = now;
        hum_last_diffuser_change_ms = now;
        diffuser_state_known = true;
        diffuser_switch_count++;

        if (diffuser_on) {
            const int cnt = min(dc.count, MAX_DIFFUSERS);
            const bool far_below = data.humidity <= rh_min - 0.5f * hyst;
            if (far_below || cnt < 2) {
                diffuser_active_idx = -1;                    // wszystkie naraz
            } else {
                diffuser_active_idx = diffuser_next_idx % cnt;   // na przemian
                diffuser_next_idx = (diffuser_active_idx + 1) % cnt;
            }
        } else {
            diffuser_active_idx = -1;
        }
    }

    for (int i = 0; i < dc.count && i < MAX_DIFFUSERS; i++) {
        if (dc.pins[i] < 0) continue;
        const bool on_this = diffuser_on && (diffuser_active_idx < 0 || i == diffuser_active_idx);
        diffuser_write_pin(dc.pins[i], on_this);
    }
}


// Wybór regulatora: 0 = prosty z histerezą (domyślny), 1 = adaptacyjny z uczeniem, 2 = PI.
void heaters_control(const SensorData& data) {
    switch (configMgr.cfg.system.control_mode) {
        case 1: heaters_control_adaptive(data); break;
        case 2: heaters_control_pi(data); break;
        default: heaters_control_simple(data); break;
    }
}

void diffusers_control(const SensorData& data) {
    diffusers_band_control(data);   // te same zasady we wszystkich trybach regulatora
}

// ============================================================================
//  KALIBRACJA – FUNKCJE POMOCNICZE
// ============================================================================

void calibration_tick() {
    if (calState.running) {
        calState.elapsedMs = millis() - calState.startMs;
    }
    if (calState.seqRunning) {
        unsigned long now = millis();
        if (now - calState.seqStartMs > 2000) {
            calState.seqStartMs = now;
            calState.seqCount++;
            if (calState.seqCount >= calState.seqMax) {
                calState.seqRunning = false;
            }
            // Wykonaj obrót
            tray_manual_move(0, 15000, now);
        }
    }
}

// ============================================================================
//  GETTERY DLA KALIBRACJI
// ============================================================================
int getCalibrationPWM() { return calState.pwmPercent; }
String getCalibrationPhase() { return calState.phase; }
unsigned long getCalibrationElapsed() { return calState.elapsedMs; }
bool isCalibrationRunning() { return calState.running; }
bool isSequenceRunning() { return calState.seqRunning; }
int getSequenceProgress() { return calState.seqCount; }
int getSequenceMax() { return calState.seqMax; }