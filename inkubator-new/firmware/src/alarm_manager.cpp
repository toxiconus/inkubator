// alarm_manager.cpp
#include "alarm_manager.h"
#include "config_manager.h"
#include "controller.h"
#include <Arduino.h>

extern float heater_pwm_pct;

void check_alarms(const SensorData& data) {
    float temp = data.temp[0];
    float hum = data.humidity;
    bool alarm_active = false;
    String alarm_msg = "";

    // === ALARMY TEMPERATURY ===
    if (isnan(temp) || temp < -50 || temp > 80) {
        alarm_msg = "⚠️ BŁĄD CZUJNIKA TEMPERATURY!";
        alarm_active = true;
    } else {
        if (temp > configMgr.cfg.system.alarm_max_temp) {
            alarm_msg = "🔴 ALARM: Temperatura ZBYT WYSOKA! (" + String(temp, 1) + "°C)";
            alarm_active = true;
        } else if (temp < configMgr.cfg.system.alarm_min_temp) {
            alarm_msg = "🔴 ALARM: Temperatura ZBYT NISKA! (" + String(temp, 1) + "°C)";
            alarm_active = true;
        }
    }

    // === ALARMY WILGOTNOŚCI ===
    if (!isnan(hum) && hum >= 0 && hum <= 100) {
        if (hum > configMgr.cfg.system.alarm_max_humidity) {
            alarm_msg = "🔴 ALARM: Wilgotność ZBYT WYSOKA! (" + String(hum, 1) + "%)";
            alarm_active = true;
        } else if (hum < configMgr.cfg.system.alarm_min_humidity) {
            alarm_msg = "🔴 ALARM: Wilgotność ZBYT NISKA! (" + String(hum, 1) + "%)";
            alarm_active = true;
        }
    }

    static unsigned long last_temp_check = 0;
    static float temp_history[5] = {NAN, NAN, NAN, NAN, NAN};
    if (millis() - last_temp_check > 60000) {
        last_temp_check = millis();
        for (int i = 0; i < 4; i++) {
            temp_history[i] = temp_history[i + 1];
        }
        temp_history[4] = temp;

        bool all_valid = true;
        for (int i = 0; i < 5; i++) {
            if (isnan(temp_history[i]) || temp_history[i] < -50.0f || temp_history[i] > 80.0f) {
                all_valid = false;
                break;
            }
        }

        if (all_valid && heater_pwm_pct > 80.0f) {
            float avg_old = (temp_history[0] + temp_history[1] + temp_history[2]) / 3.0f;
            float avg_new = (temp_history[3] + temp_history[4]) / 2.0f;
            if (avg_new < avg_old - 0.5f) {
                alarm_msg = "🔴 ALARM: Możliwa AWARIA GRZAŁKI! Temperatura spada mimo grzania.";
                alarm_active = true;
            }
        }
    }

    if (alarm_active) {
        Serial.println("[ALARM] " + alarm_msg);
    }
}
