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

void heaters_control(const SensorData& data) {
    float target = configMgr.cfg.system.target_temp;
    float current = data.temp[0];
    if (current < -50 || current > 80) { heater_pwm_pct = 0; return; }
    float diff = target - current;
    float hyst = configMgr.cfg.system.temp_hysteresis;
    if (diff > hyst) heater_pwm_pct = min(100.0f, heater_pwm_pct + 5.0f);
    else if (diff < -hyst) heater_pwm_pct = max(0.0f, heater_pwm_pct - 3.0f);
    else heater_pwm_pct = max(0.0f, heater_pwm_pct - 0.5f);
    for (int ch = 0; ch < 2; ch++) {
        int pin = (ch == 0) ? configMgr.cfg.system.heater_pin_1 : configMgr.cfg.system.heater_pin_2;
        if (pin < 0) continue;
        int pwm = heater_manual_mode[ch] ? heater_manual_pct[ch] * 10.23f : heater_pwm_pct * 10.23f;
        ledcWrite(ch, constrain(pwm, 0, 1023));
    }
}

void diffusers_control(const SensorData& data) {
    if (configMgr.cfg.diffusers.count == 0) return;
    int pin = configMgr.cfg.diffusers.pins[0];
    if (pin < 0) return;
    if (diffuser_manual_mode) {
        digitalWrite(pin, diffuser_manual_on[0] ? LOW : HIGH);
        diffuser_on = diffuser_manual_on[0];
        return;
    }
    float hum = data.humidity;
    if (hum < 0) return;
    float target = configMgr.cfg.system.target_humidity;
    float hyst = configMgr.cfg.system.humidity_hysteresis;
    if (hum < target - hyst) {
        diffuser_on = true;
    } else if (hum > target + hyst) {
        diffuser_on = false;
    }
    digitalWrite(pin, diffuser_on ? LOW : HIGH);
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

void reset_learning_state() {}
void clear_learning_state() {}
void start_learning_record(unsigned long duration_sec) {}
void stop_learning_record() {}
bool is_learning_record_active() { return false; }
unsigned long learning_record_elapsed_sec() { return 0; }
int learning_record_samples_count() { return 0; }
void handle_learning_record(const SensorData& data) {}

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