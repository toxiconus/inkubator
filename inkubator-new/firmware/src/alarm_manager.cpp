// alarm_manager.cpp
#include "alarm_manager.h"
#include "config_manager.h"
#include "controller.h"
#include <Arduino.h>

extern float heater_pwm_pct;
void event_log(const char* type, const String& text);

static char g_alarm_code[20] = "";
static char g_alarm_msg[96] = "";
static unsigned long g_alarm_since = 0;

const char* alarm_code() { return g_alarm_code; }
const char* alarm_message() { return g_alarm_msg; }
unsigned long alarm_since_ms() { return g_alarm_since; }

static void set_alarm(const char* code, const String& msg) {
    const bool changed = strcmp(g_alarm_code, code) != 0;
    if (changed) {
        // koniec poprzedniego alarmu i początek nowego trafiają do dziennika zdarzeń
        if (g_alarm_code[0]) event_log("alarm_off", String(g_alarm_code));
        strncpy(g_alarm_code, code, sizeof(g_alarm_code) - 1);
        g_alarm_code[sizeof(g_alarm_code) - 1] = '\0';
        g_alarm_since = millis();
        if (code[0]) event_log("alarm_on", msg);
    }
    strncpy(g_alarm_msg, msg.c_str(), sizeof(g_alarm_msg) - 1);
    g_alarm_msg[sizeof(g_alarm_msg) - 1] = '\0';
}

void check_alarms(const SensorData& data) {
    // Czujnik alarmowy to ten sam, który steruje grzałką (przy błędzie odczytu sterowanie i tak wyłącza grzanie)
    float temp = data.temp[0];
    float hum = data.humidity;
    const char* code = "";
    String msg = "";

    // === ALARMY TEMPERATURY ===
    if (isnan(temp) || temp < -50 || temp > 80) {
        code = "sensor";
        msg = "Błąd czujnika temperatury";
    } else if (temp > effective_alarm_max_temp()) {
        code = "temp_high";
        msg = "Temperatura zbyt wysoka (" + String(temp, 1) + " °C)";
    } else if (temp < effective_alarm_min_temp()) {
        code = "temp_low";
        msg = "Temperatura zbyt niska (" + String(temp, 1) + " °C)";
    }

    // === ALARMY WILGOTNOŚCI ===
    if (!code[0] && !isnan(hum) && hum >= 0 && hum <= 100) {
        if (hum > effective_alarm_max_humidity()) {
            code = "hum_high";
            msg = "Wilgotność zbyt wysoka (" + String(hum, 0) + " %)";
        } else if (hum < effective_alarm_min_humidity()) {
            code = "hum_low";
            msg = "Wilgotność zbyt niska (" + String(hum, 0) + " %)";
        }
    }

    // === AWARIA GRZAŁKI ===
    // (a) temperatura spada mimo mocy > 80 %
    static unsigned long last_temp_check = 0;
    static float temp_history[5] = {NAN, NAN, NAN, NAN, NAN};
    static bool heater_fault = false;
    // (b) grzałka pracuje na ponad 95 % przez 15 minut, a temperatura jest wyraźnie poniżej nastawy
    static unsigned long full_power_since = 0;

    if (millis() - last_temp_check > 60000) {
        last_temp_check = millis();
        for (int i = 0; i < 4; i++) temp_history[i] = temp_history[i + 1];
        temp_history[4] = temp;

        bool all_valid = true;
        for (int i = 0; i < 5; i++) {
            if (isnan(temp_history[i]) || temp_history[i] < -50.0f || temp_history[i] > 80.0f) { all_valid = false; break; }
        }
        heater_fault = false;
        if (all_valid && heater_pwm_pct > 80.0f) {
            float avg_old = (temp_history[0] + temp_history[1] + temp_history[2]) / 3.0f;
            float avg_new = (temp_history[3] + temp_history[4]) / 2.0f;
            if (avg_new < avg_old - 0.5f) heater_fault = true;
        }
        const float setpoint = control_setpoint_temp();
        if (heater_pwm_pct > 95.0f && !isnan(temp) && temp < setpoint - 1.0f) {
            if (full_power_since == 0) full_power_since = millis();
            if (millis() - full_power_since > 15UL * 60UL * 1000UL) heater_fault = true;
        } else {
            full_power_since = 0;
        }
    }
    if (!code[0] && heater_fault) {
        code = "heater";
        msg = "Możliwa awaria grzałki: temperatura nie rośnie mimo grzania";
    }

    // Histereza czasowa: alarm włącza się po 3 kolejnych odczytach (ok. 15 s) i wyłącza po 3 odczytach bez niego,
    // żeby wartość kołysząca się na granicy nie zalewała dziennika zdarzeń ani nie migała na ekranie.
    static int streak = 0;
    static char pending[20] = "";
    if (strcmp(pending, code) == 0) { if (streak < 100) streak++; }
    else { strncpy(pending, code, sizeof(pending) - 1); pending[sizeof(pending) - 1] = '\0'; streak = 1; }
    if (streak >= 3 || strcmp(g_alarm_code, code) == 0) set_alarm(code, msg);
}
