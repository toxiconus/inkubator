// config_manager.h
#pragma once

#include <Arduino.h>
#include <Preferences.h>

#define MAX_TRAYS 2
#define MAX_SENSORS 4
#define MAX_DIFFUSERS 2
#define CONFIG_VERSION 2
#define PREF_NAMESPACE "inkubator"
#define PREF_KEY_CFG "config"
#define PREF_KEY_CRC "crc"
#define PREF_KEY_LEARNING_CFG "learn_cfg"
#define PREF_KEY_LEARNING_CRC "learn_crc"

struct TrayConfig {
    bool active = true;
    char species[12] = "CHICKEN";
    int start_day_offset = 0;
    unsigned long long start_unix = 0;
    int pin = 16;
    char label[24] = "Taca";
    bool signal_active_low = false;
    bool has_angle_sensor = false;
    unsigned long rotation_interval_ms = 17280000; // 8h
    unsigned long rotation_duration_ms = 15000;
    int rotation_pwm_speed = 102;
    int rotations_per_day = 5;
};

struct SensorConfig {
    int ds18b20_count = 1;
    int ds18b20_pins[MAX_SENSORS] = {6, -1, -1, -1};
    char ds18b20_labels[MAX_SENSORS][20] = {"Chamber", "", "", ""};
    bool dht11_enabled = true;
    int dht11_pin = 8;
    bool water_level_enabled = false;
    int water_level_pin = -1;
    bool door_sensor_enabled = false;
    int door_sensor_pin = -1;
    int aquarium_sensor_index = 0;
    float aquarium_target_temp = 25.0f;
    bool bmp180_enabled = false;
    int bmp180_sda_pin = 4;
    int bmp180_scl_pin = 5;
    int i2c_sda_pin = 4;
    int i2c_scl_pin = 5;
};

struct DiffuserConfig {
    int count = 1;
    int on_time_sec = 30;
    int off_time_sec = 120;
    char profile[16] = "fast";
    int pins[MAX_DIFFUSERS] = {11, -1};
    char labels[MAX_DIFFUSERS][20] = {"Diffuser 1", ""};
};

struct SystemConfig {
    bool test_mode = false;
    int fan_pin = -1;
    int heater_pin_1 = 1;
    int heater_pin_2 = 2;
    int heater_pin_3 = -1;
    int heater_pin_4 = -1;
    float target_temp = 37.7;
    float temp_hysteresis = 0.5;
    float alarm_max_temp = 40.0;
    float alarm_min_temp = 36.0;
    float temp_calibration = 0.0;
    int control_mode = 0;
    int fan_delay_sec = 5;
    float target_humidity = 60.0;
    float humidity_hysteresis = 5.0;
    float alarm_max_humidity = 75.0;
    float alarm_min_humidity = 30.0;
    float humidity_calibration = 0.0;
    int log_interval_sec = 60;
};

struct IncubatorConfig {
    int config_version = CONFIG_VERSION;
    int tray_count = 2;
    TrayConfig trays[MAX_TRAYS];
    SensorConfig sensors;
    DiffuserConfig diffusers;
    SystemConfig system;
};

struct LearningState {
    bool has_value = false;
    float learned_pwm_pct = 0.0f;
    float learned_target_temp = NAN;
    float learned_target_humidity = NAN;
    int sample_count = 0;
    int confirmed_cycles = 0;
    unsigned long long last_updated_ms = 0;
    int diffuser_on_ms = 0;
    int diffuser_off_ms = 0;
};

class ConfigManager {
public:
    IncubatorConfig cfg;
    LearningState learning_state;
    Preferences pref;

    ConfigManager();
    bool load();
    bool save();
    void factory_reset();

    bool load_spiffs();
    bool save_spiffs();
    void save_preferences();
    bool load_preferences();

    void save_learning_data();
    bool load_learning_data();
    void clear_learning_data();

    String to_json();
    bool from_json(const String& json);
};

extern ConfigManager configMgr;
IncubatorConfig default_config();