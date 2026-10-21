#pragma once
#include <Arduino.h>
#include "config_manager.h"

struct SensorData {
  float temp[MAX_SENSORS];
  float humidity;
  int water_level;
  bool door_open;
  int heater_pwm_pct;
  int heater_pwm_raw;
  bool diffuser_on;
  int diffuser_active_idx;
  unsigned long long elapsed_ms;
  bool is_dummy;
  float pressure_hpa;
  float bmp180_temp;
};

#define TEMP_DEADBAND 0.3f            // martwa strefa – stabilizacja (grzałka utrzymuje poprzednią wartość)
#define TEMP_HYST 0.3f                // histereza załącz/wyłącz
#define HUMIDITY_TARGET_HYST 1.0f     // dyfuzor: włącz przy target - 1%
#define HUMIDITY_MAX_ON_TIME_MS 300000UL // max 5 minut ciągłej pracy dyfuzora (zabezpieczenie)
#define HEATER_RAMP_MS 30000ULL       // ramp-up przez pierwsze 30 sekund

constexpr int MAX_HEATERS_HW = 2;

extern bool diffuser_on;
extern unsigned long diffuser_change_ms;
extern int diffuser_active_idx;
extern bool diffuser_both_active;

extern bool heater_manual_mode[2];
extern float heater_manual_pct[2];
extern bool diffuser_manual_mode;
extern bool diffuser_manual_on[2];
extern float heater_pwm_pct;

extern String heater_profile_name;
extern float heater_fuzzy_small;
extern float heater_fuzzy_medium;
extern float heater_fuzzy_large;
extern float heater_fuzzy_min_out;
extern float heater_fuzzy_med_out;
extern float heater_fuzzy_high_out;

extern bool learning_mode;
extern String learning_profile;

bool set_heater_profile(const String& mode);
bool set_diffuser_profile(const String& mode);
void set_heater_custom_profile(float small, float medium, float large,
                               float min_out = 20.0f,
                               float med_out = 80.0f,
                               float high_out = 92.0f);

void start_learning_record(unsigned long duration_sec);
void stop_learning_record();
bool is_learning_record_active();
unsigned long learning_record_elapsed_sec();
int learning_record_samples_count();
void handle_learning_record(const SensorData& data);
void clear_learning_state();

int heater_pin_for_index(int idx);
void heaters_control(const SensorData& data);
void diffusers_control(const SensorData& data);
