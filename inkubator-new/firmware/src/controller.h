// controller.h
#pragma once

#include "config_manager.h"

#define MAX_HEATERS_HW 4
#define TEMP_DEADBAND 0.1f
#define HEATER_RAMP_MS 60000

extern float heater_pwm_pct;
extern bool heater_manual_mode[2];
extern float heater_manual_pct[2];
extern bool diffuser_manual_mode;
extern bool diffuser_manual_on[2];
extern bool diffuser_on;
extern int diffuser_active_idx;
extern String heater_profile_name;
// Skala sprzętowa PWM grzałek (LEDC 10 bitów). W reszcie kodu moc jest w procentach 0..100.
#define HEATER_PWM_MAX 1023
void heater_write_pct(int ch, float pct);
extern unsigned long diffuser_switch_count;   // liczba przełączeń dyfuzorów od startu (zużycie styków)
extern bool learning_mode;
extern String learning_profile;
int heater_pin_for_index(int idx);

struct SensorData {
    float temp[4];
    float humidity;
    int water_level;
    bool door_open;
    int heater_pwm_pct;
    int heater_pwm_raw;
    bool diffuser_on;
    int diffuser_active_idx;
    float pressure_hpa;
    float bmp180_temp;
    unsigned long long elapsed_ms;
    bool is_dummy;
};

void heaters_control(const SensorData& data);
void diffusers_control(const SensorData& data);
float control_temperature_value(const SensorData& data);   // odczyt czujnika sterującego grzałką
float control_setpoint_temp();                              // nastawa temperatury dla tego czujnika
float control_humidity_ideal();                             // idealna wilgotność (0 = program bez regulacji wilgotności)
float effective_alarm_max_temp();                           // progi alarmów podążają za programem tac
float effective_alarm_min_temp();
float effective_alarm_max_humidity();
float effective_alarm_min_humidity();
bool set_heater_profile(const String& mode);
bool set_diffuser_profile(const String& mode);
void reset_learning_state();
void clear_learning_state();
void start_learning_record(unsigned long duration_sec);
void stop_learning_record();
bool is_learning_record_active();
unsigned long learning_record_elapsed_sec();
int learning_record_samples_count();
void handle_learning_record(const SensorData& data);