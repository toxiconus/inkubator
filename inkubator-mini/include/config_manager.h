#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <ArduinoJson.h>

// ============================================================================
// CONFIG MANAGER – wersja ulepszona
// ============================================================================

#define MAX_TRAYS       3
#define MAX_SENSORS     4
#define MAX_DIFFUSERS   3
#define CONFIG_PATH     "/config.json"
#define PREF_NAMESPACE        "inkubator"
#define PREF_KEY_CFG          "config_json"
#define PREF_KEY_CRC          "config_crc"
#define PREF_KEY_LEARNING_CFG "learning_json"
#define PREF_KEY_LEARNING_CRC "learning_crc"

#define CONFIG_VERSION  107   // Zwiększ przy zmianie struktury! (dodano has_angle_sensor)

// ── Struktury konfiguracji ─────────────────────────────────────────────────
struct TrayConfig {
  bool active;
  char species[12];
  int start_day_offset;
  int pin;
  char label[24];
  bool signal_active_low;             // true = LOW aktywuje napęd, HIGH = OFF
  bool has_angle_sensor;              // MPU6050 tilt sensor installed for this tray
  unsigned long long start_unix;       // UNIX start inkubacji (jeśli używany)

  // PWM rotation parameters (obrót jaj)
  unsigned long rotation_interval_ms;    // Interwał między obrotami (np. 4.8h = 17280000 ms)
  unsigned long rotation_duration_ms;    // Jak długo trwa PWM ON (np. 15000 ms = 15s)
  int rotation_pwm_speed;                // PWM 0-255 (raw duty cycle, np. 102 = ~40%)
  int rotations_per_day;                 // Info: ile obrotów dziennie
};

struct SensorConfig {
  int ds18b20_count;
  int ds18b20_pins[MAX_SENSORS];
  char ds18b20_labels[MAX_SENSORS][20];
  bool dht11_enabled;  // Current hardware is DHT11; legacy JSON keys dht22_* are also accepted.
  int dht11_pin;       // GPIO for the DHT11 sensor.
  bool water_level_enabled;
  int water_level_pin;
  bool door_sensor_enabled;
  int door_sensor_pin;
  int aquarium_sensor_index;
  float aquarium_target_temp;
  bool bmp180_enabled;
  int bmp180_sda_pin;
  int bmp180_scl_pin;
  int i2c_sda_pin;    // GPIO for shared I2C bus (MPU6050/BMP180)
  int i2c_scl_pin;
};

struct DiffuserConfig {
  int count;
  int pins[MAX_DIFFUSERS];
  char labels[MAX_DIFFUSERS][20];
  int on_time_sec;
  int off_time_sec;
  char profile[16];
};

struct SystemConfig {
  bool test_mode;
  int fan_pin;
  int heater_pin_1;
  int heater_pin_2;
  int heater_pin_3;
  int heater_pin_4;
  float target_temp;
  int log_interval_sec;
};

struct IncubatorConfig {
  int config_version = CONFIG_VERSION;
  int tray_count;
  TrayConfig trays[MAX_TRAYS];
  SensorConfig sensors;
  DiffuserConfig diffusers;
  SystemConfig system;
};

// ── Domyślna konfiguracja (naprawione konflikty pinów) ─────────────────────
inline IncubatorConfig default_config() {
  IncubatorConfig cfg = {};

  cfg.config_version = CONFIG_VERSION;
  cfg.tray_count = 2;

  // Taca 0 - Kaczki
  cfg.trays[0].active = true;
  strlcpy(cfg.trays[0].species, "DUCK", sizeof(cfg.trays[0].species));
  cfg.trays[0].start_day_offset = 0;
  cfg.trays[0].pin = 16;
  strlcpy(cfg.trays[0].label, "Taca 1 – Kaczki", sizeof(cfg.trays[0].label));
  cfg.trays[0].signal_active_low = false;
  cfg.trays[0].has_angle_sensor = true;
  cfg.trays[0].rotation_interval_ms = 17280000;    // 4.8 godziny (24h / 5 sesji)
  cfg.trays[0].rotation_duration_ms = 15000;       // 15 sekund PWM
  cfg.trays[0].rotation_pwm_speed = 102;           // 40% PWM (0.4 * 255)
  cfg.trays[0].rotations_per_day = 5;

  // Taca 1 - Kury
  cfg.trays[1].active = true;
  strlcpy(cfg.trays[1].species, "CHICKEN", sizeof(cfg.trays[1].species));
  cfg.trays[1].start_day_offset = 0;
  cfg.trays[1].pin = 17;
  strlcpy(cfg.trays[1].label, "Taca 2 – Kury", sizeof(cfg.trays[1].label));
  cfg.trays[1].signal_active_low = false;
  cfg.trays[1].has_angle_sensor = false;
  cfg.trays[1].rotation_interval_ms = 17280000;
  cfg.trays[1].rotation_duration_ms = 15000;
  cfg.trays[1].rotation_pwm_speed = 102;
  cfg.trays[1].rotations_per_day = 5;
  cfg.trays[1].start_unix = 0ULL;

  // Taca 2 - rezerwa
  cfg.trays[2].active = false;
  strlcpy(cfg.trays[2].species, "CHICKEN", sizeof(cfg.trays[2].species));
  cfg.trays[2].start_day_offset = 0;
  cfg.trays[2].pin = 18;
  strlcpy(cfg.trays[2].label, "Taca 3 – (wolna)", sizeof(cfg.trays[2].label));
  cfg.trays[2].signal_active_low = false;
  cfg.trays[2].has_angle_sensor = false;
  cfg.trays[2].rotation_interval_ms = 17280000;
  cfg.trays[2].rotation_duration_ms = 15000;
  cfg.trays[2].rotation_pwm_speed = 102;
  cfg.trays[2].rotations_per_day = 5;
  cfg.trays[2].start_unix = 0ULL;

  // Sensory
  cfg.sensors.ds18b20_count = 2;
  int ds_pins_default[] = {6, 6, -1, -1};  // GPIO6 = wspólna magistrala OneWire dla obu DS18B20
  const char* ds_labels_default[] = {"Komora jaj", "Akwarium", "Zewnetrzny", "Dodatkowy"};
  for (int i = 0; i < MAX_SENSORS; i++) {
    cfg.sensors.ds18b20_pins[i] = ds_pins_default[i];
    strlcpy(cfg.sensors.ds18b20_labels[i], ds_labels_default[i], 20);
  }

  cfg.sensors.dht11_enabled = true;
  cfg.sensors.dht11_pin = 8;
  cfg.sensors.water_level_enabled = false;
  cfg.sensors.water_level_pin = 9;
  cfg.sensors.door_sensor_enabled = false;
  cfg.sensors.door_sensor_pin = 10;
  cfg.sensors.aquarium_sensor_index = 1;           // DS #1 = Akwarium
  cfg.sensors.aquarium_target_temp = 36.0f;
  cfg.sensors.bmp180_enabled = false;  // Domyślnie OFF (czujnik opcjonalny)
  cfg.sensors.bmp180_sda_pin = 4;      // GPIO4 (SDA) - jeśli BMP180 ma być użyty
  cfg.sensors.bmp180_scl_pin = 5;      // GPIO5 (SCL)
  cfg.sensors.i2c_sda_pin = 4;         // Wspólna magistrala I2C dla MPU6050/BMP180/RTC
  cfg.sensors.i2c_scl_pin = 5;

  // Dyfuzory
  cfg.diffusers.count = 2;
  int diff_pins_default[] = {11, 12, -1};  // D1=GPIO11, D2=GPIO12 (nawilżacze ultradźwiękowe)
  const char* diff_labels_default[] = {"Nawilzacz 1", "Nawilzacz 2", "Nawilzacz 3"};
  for (int i = 0; i < MAX_DIFFUSERS; i++) {
    cfg.diffusers.pins[i] = diff_pins_default[i];
    strlcpy(cfg.diffusers.labels[i], diff_labels_default[i], 20);
  }
  cfg.diffusers.on_time_sec = 60;
  cfg.diffusers.off_time_sec = 300;
  strlcpy(cfg.diffusers.profile, "fast", sizeof(cfg.diffusers.profile));

  // System
  cfg.system.test_mode = false;
  cfg.system.fan_pin = 7;      // GPIO7 – wolny (oba DS18B20 na GPIO6, wspólna magistrala)
  cfg.system.heater_pin_1 = 38;
  cfg.system.heater_pin_2 = 39;
  cfg.system.heater_pin_3 = 40;  // CH2: 3. grzałka (GPIO40 + MOSFET)
  cfg.system.heater_pin_4 = -1;
  cfg.system.target_temp = 37.5f;
  cfg.system.log_interval_sec = 600;

  return cfg;
}

// ── Główna klasa ───────────────────────────────────────────────────────────
struct LearningState {
  bool has_value = false;
  float learned_pwm_pct = 0.0f;
  float learned_target_temp = NAN;
  float learned_target_humidity = NAN;
  int diffuser_on_ms = 0;
  int diffuser_off_ms = 0;
  int sample_count = 0;
  int confirmed_cycles = 0;
  unsigned long long last_updated_ms = 0;
};

class ConfigManager {
public:
  IncubatorConfig cfg;
  LearningState learning_state;
  bool loaded_ok = false;

  ConfigManager() = default;
  ~ConfigManager() { pref.end(); }

  bool load() {
    if (load_spiffs()) {
      Serial.println("[CFG] Wczytano z LittleFS – synchronizuję do Preferences");
      save_preferences();
      loaded_ok = true;
      return true;
    }

    Serial.println("[CFG] Brak poprawnego pliku LittleFS, próbuję Preferences...");
    if (load_preferences()) {
      Serial.println("[CFG] Wczytano z Preferences – synchronizuję do LittleFS");
      save_spiffs();
      loaded_ok = true;
      return true;
    }

    Serial.println("[CFG] Używam konfiguracji domyslnej");
    cfg = default_config();
    save();
    loaded_ok = true;
    return false;
  }

  bool save() {
    bool ok = save_spiffs();
    save_preferences();
    return ok;
  }

  void factory_reset() {
    cfg = default_config();
    save();
    save_preferences();
    Serial.println("[CFG] Przywrócono ustawienia fabryczne");
  }

  bool load_learning_data();
  void save_learning_data();
  void clear_learning_data();

  String to_json();
  bool from_json(const String& json_str);

private:
  Preferences pref;

  bool load_spiffs();
  bool save_spiffs();
  void save_preferences();
  bool load_preferences();
};

// ── Globalna instancja ─────────────────────────────────────────────────────
extern ConfigManager configMgr;
