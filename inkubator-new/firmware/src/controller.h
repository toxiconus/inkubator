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