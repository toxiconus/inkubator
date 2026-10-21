// config_manager.cpp
#include "config_manager.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <ArduinoJson.h>

static bool is_valid_pin(int pin);
static void sanitize_config(IncubatorConfig& cfg);
static bool ensure_littlefs();
static uint16_t calculate_crc(const uint8_t* data, size_t len);

static bool ensure_littlefs() {
  if (LittleFS.begin()) {
    return true;
  }

  Serial.println("[CFG] Nie można zamontować LittleFS");
  return false;
}

static uint16_t calculate_crc(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  while (len--) {
    crc ^= *data++;
    for (int i = 0; i < 8; i++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

// ============================================================================
// Implementacja metod ConfigManager
// ============================================================================

bool ConfigManager::load_spiffs() {
  if (!ensure_littlefs()) {
    return false;
  }

  if (!LittleFS.exists(CONFIG_PATH)) {
    Serial.println("[CFG] Brak pliku config.json w LittleFS");
    return false;
  }

  File f = LittleFS.open(CONFIG_PATH, "r");
  if (!f) {
    Serial.println("[CFG] Nie można otworzyć config.json w LittleFS");
    return false;
  }

  String json = f.readString();
  f.close();

  if (json.length() < 50) {
    Serial.println("[CFG] Plik config.json jest za krótki lub uszkodzony");
    return false;
  }

  // Sprawdź wersję konfiguracji PRZED załadowaniem
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    Serial.printf("[CFG] Błąd parsowania JSON w LittleFS: %s\n", err.c_str());
    return false;
  }
  
  if (!doc["config_version"].is<int>()) {
    Serial.println("[CFG] Brak config_version – plik uszkodzony!");
    return false;
  }

  int saved_version = doc["config_version"];
  if (saved_version != CONFIG_VERSION) {
    Serial.printf("[CFG] Config version mismatch: %d != %d – załaduję domyślną\n", saved_version, CONFIG_VERSION);
    return false;  // Wymuś załadowanie domyślnej konfiguracji
  }

  return from_json(json);
}

bool ConfigManager::save_spiffs() {
  if (!ensure_littlefs()) {
    return false;
  }

  String json = to_json();
  const char* tmp_path = "/config.json.tmp";
  File f = LittleFS.open(tmp_path, "w");
  if (!f) {
    Serial.println("[CFG] Błąd zapisu pliku tymczasowego LittleFS!");
    return false;
  }

  if (f.print(json) < 0) {
    f.close();
    LittleFS.remove(tmp_path);
    Serial.println("[CFG] Błąd zapisu zawartości do LittleFS!");
    return false;
  }
  f.close();

  if (LittleFS.exists(CONFIG_PATH) && !LittleFS.remove(CONFIG_PATH)) {
    LittleFS.remove(tmp_path);
    Serial.println("[CFG] Nie udało się usunąć starego pliku config.json");
    return false;
  }

  if (!LittleFS.rename(tmp_path, CONFIG_PATH)) {
    LittleFS.remove(tmp_path);
    Serial.println("[CFG] Nie udało się zastąpić pliku config.json");
    return false;
  }

  Serial.printf("[CFG] Zapisano do LittleFS (%d bajtów)\n", json.length());
  return true;
}

void ConfigManager::save_preferences() {
  if (!pref.begin(PREF_NAMESPACE, false)) {
    Serial.println("[CFG] Błąd otwarcia Preferences do zapisu!");
    return;
  }

  String json = to_json();
  pref.putString(PREF_KEY_CFG, json);
  uint16_t crc = calculate_crc((const uint8_t*)json.c_str(), json.length());
  pref.putUShort(PREF_KEY_CRC, crc);

  pref.end();
  Serial.println("[CFG] Backup zapisany w Preferences");
}

bool ConfigManager::load_preferences() {
  if (!pref.begin(PREF_NAMESPACE, true)) {
    Serial.println("[CFG] Błąd otwarcia Preferences do odczytu!");
    return false;
  }

  if (!pref.isKey(PREF_KEY_CFG)) {
    pref.end();
    return false;
  }

  String json = pref.getString(PREF_KEY_CFG, "");
  uint16_t saved_crc = pref.getUShort(PREF_KEY_CRC, 0);
  pref.end();

  if (json.length() == 0) return false;

  // Weryfikacja CRC
  uint16_t crc = calculate_crc((const uint8_t*)json.c_str(), json.length());

  if (crc != saved_crc) {
    Serial.println("[CFG] CRC mismatch w Preferences – odrzucam backup");
    return false;
  }

  // Sprawdź wersję konfiguracji PRZED załadowaniem
  JsonDocument doc;
  if (deserializeJson(doc, json)) {
    Serial.println("[CFG] Błąd parsowania JSON w Preferences");
    return false;
  }
  
  if (!doc["config_version"].is<int>()) {
    Serial.println("[CFG] Brak config_version w Preferences – plik uszkodzony!");
    return false;
  }

  int saved_version = doc["config_version"];
  if (saved_version != CONFIG_VERSION) {
    Serial.printf("[CFG] Preferences version mismatch: %d != %d – załaduję domyślną\n", saved_version, CONFIG_VERSION);
    return false;  // Wymuś załadowanie domyślnej konfiguracji
  }

  return from_json(json);
}

void ConfigManager::save_learning_data() {
  if (!pref.begin(PREF_NAMESPACE, false)) {
    Serial.println("[CFG] Błąd otwarcia Preferences do zapisu uczenia!");
    return;
  }

  JsonDocument doc;
  doc["has_value"] = learning_state.has_value;
  doc["learned_pwm_pct"] = learning_state.learned_pwm_pct;
  doc["learned_target_temp"] = learning_state.learned_target_temp;
  doc["learned_target_humidity"] = learning_state.learned_target_humidity;
  doc["sample_count"] = learning_state.sample_count;
  doc["confirmed_cycles"] = learning_state.confirmed_cycles;
  doc["last_updated_ms"] = learning_state.last_updated_ms;
  doc["diffuser_on_ms"] = learning_state.diffuser_on_ms;
  doc["diffuser_off_ms"] = learning_state.diffuser_off_ms;

  String json;
  serializeJson(doc, json);
  pref.putString(PREF_KEY_LEARNING_CFG, json);
  uint16_t crc = calculate_crc((const uint8_t*)json.c_str(), json.length());
  pref.putUShort(PREF_KEY_LEARNING_CRC, crc);
  pref.end();
  Serial.printf("[CFG] Learning backup zapisany w Preferences (%d bajtów)\n", json.length());
}

bool ConfigManager::load_learning_data() {
  if (!pref.begin(PREF_NAMESPACE, true)) {
    Serial.println("[CFG] Błąd otwarcia Preferences do odczytu uczenia!");
    return false;
  }

  if (!pref.isKey(PREF_KEY_LEARNING_CFG)) {
    pref.end();
    return false;
  }

  String json = pref.getString(PREF_KEY_LEARNING_CFG, "");
  uint16_t saved_crc = pref.getUShort(PREF_KEY_LEARNING_CRC, 0);
  pref.end();

  if (json.length() == 0) return false;

  uint16_t crc = calculate_crc((const uint8_t*)json.c_str(), json.length());
  if (crc != saved_crc) {
    Serial.println("[CFG] Learning CRC mismatch w Preferences – odrzucam backup");
    return false;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    Serial.printf("[CFG] Błąd parsowania JSON uczenia: %s\n", err.c_str());
    return false;
  }

  learning_state.has_value = doc["has_value"] | false;
  learning_state.learned_pwm_pct = doc["learned_pwm_pct"] | 0.0f;
  learning_state.learned_target_temp = doc["learned_target_temp"] | NAN;
  learning_state.learned_target_humidity = doc["learned_target_humidity"] | NAN;
  learning_state.sample_count = doc["sample_count"] | 0;
  learning_state.confirmed_cycles = doc["confirmed_cycles"] | 0;
  learning_state.last_updated_ms = doc["last_updated_ms"] | 0ULL;
  learning_state.diffuser_on_ms = doc["diffuser_on_ms"] | 0;
  learning_state.diffuser_off_ms = doc["diffuser_off_ms"] | 0;

  Serial.println("[CFG] Wczytano Learning data z Preferences");
  return true;
}

void ConfigManager::clear_learning_data() {
  learning_state = LearningState();

  if (!pref.begin(PREF_NAMESPACE, false)) {
    Serial.println("[CFG] Błąd otwarcia Preferences do czyszczenia uczenia!");
    return;
  }

  pref.remove(PREF_KEY_LEARNING_CFG);
  pref.remove(PREF_KEY_LEARNING_CRC);
  pref.end();
  Serial.println("[CFG] Learning data usunięte z Preferences");
}

// ============================================================================
// JSON Serialization / Deserialization
// ============================================================================

String ConfigManager::to_json() {
  JsonDocument doc;

  doc["config_version"] = cfg.config_version;
  doc["tray_count"] = cfg.tray_count;

  JsonArray trays = doc["trays"].to<JsonArray>();
  for (int i = 0; i < MAX_TRAYS; i++) {
    JsonObject t = trays.add<JsonObject>();
    t["active"] = cfg.trays[i].active;
    t["species"] = cfg.trays[i].species;
    t["start_day_offset"] = cfg.trays[i].start_day_offset;
    t["start_unix"] = cfg.trays[i].start_unix;
    t["pin"] = cfg.trays[i].pin;
    t["label"] = cfg.trays[i].label;
    t["signal_active_low"] = cfg.trays[i].signal_active_low;
    t["has_angle_sensor"] = cfg.trays[i].has_angle_sensor;
    t["rotation_interval_ms"] = cfg.trays[i].rotation_interval_ms;
    t["rotation_duration_ms"] = cfg.trays[i].rotation_duration_ms;
    t["rotation_pwm_speed"] = cfg.trays[i].rotation_pwm_speed;
    t["rotations_per_day"] = cfg.trays[i].rotations_per_day;
  }

  // Sensors
  JsonObject sensors = doc["sensors"].to<JsonObject>();
  sensors["ds18b20_count"] = cfg.sensors.ds18b20_count;
  JsonArray ds_pins = sensors["ds18b20_pins"].to<JsonArray>();
  JsonArray ds_labels = sensors["ds18b20_labels"].to<JsonArray>();
  for (int i = 0; i < MAX_SENSORS; i++) {
    ds_pins.add(cfg.sensors.ds18b20_pins[i]);
    ds_labels.add(cfg.sensors.ds18b20_labels[i]);
  }
  // Legacy keys (dht22_*) kept for backward compatibility.
  sensors["dht22_enabled"] = cfg.sensors.dht11_enabled;
  sensors["dht22_pin"] = cfg.sensors.dht11_pin;
  // Preferred keys for current hardware naming.
  sensors["dht11_enabled"] = cfg.sensors.dht11_enabled;
  sensors["dht11_pin"] = cfg.sensors.dht11_pin;
  sensors["water_level_enabled"] = cfg.sensors.water_level_enabled;
  sensors["water_level_pin"] = cfg.sensors.water_level_pin;
  sensors["door_sensor_enabled"] = cfg.sensors.door_sensor_enabled;
  sensors["door_sensor_pin"] = cfg.sensors.door_sensor_pin;
  sensors["aquarium_sensor_index"] = cfg.sensors.aquarium_sensor_index;
  sensors["aquarium_target_temp"] = cfg.sensors.aquarium_target_temp;
  sensors["bmp180_enabled"] = cfg.sensors.bmp180_enabled;
  sensors["bmp180_sda_pin"] = cfg.sensors.bmp180_sda_pin;
  sensors["bmp180_scl_pin"] = cfg.sensors.bmp180_scl_pin;
  sensors["i2c_sda_pin"] = cfg.sensors.i2c_sda_pin;
  sensors["i2c_scl_pin"] = cfg.sensors.i2c_scl_pin;

  // Diffusers
  JsonObject diffusers = doc["diffusers"].to<JsonObject>();
  diffusers["count"] = cfg.diffusers.count;
  diffusers["on_time_sec"] = cfg.diffusers.on_time_sec;
  diffusers["off_time_sec"] = cfg.diffusers.off_time_sec;
  diffusers["profile"] = cfg.diffusers.profile;
  JsonArray d_pins = diffusers["pins"].to<JsonArray>();
  JsonArray d_labels = diffusers["labels"].to<JsonArray>();
  for (int i = 0; i < MAX_DIFFUSERS; i++) {
    d_pins.add(cfg.diffusers.pins[i]);
    d_labels.add(cfg.diffusers.labels[i]);
  }

  // System
  JsonObject system = doc["system"].to<JsonObject>();
  system["test_mode"] = cfg.system.test_mode;
  system["fan_pin"] = cfg.system.fan_pin;
  system["heater_pin_1"] = cfg.system.heater_pin_1;
  system["heater_pin_2"] = cfg.system.heater_pin_2;
  system["heater_pin_3"] = cfg.system.heater_pin_3;
  system["heater_pin_4"] = cfg.system.heater_pin_4;
  system["target_temp"] = cfg.system.target_temp;
  system["log_interval_sec"] = cfg.system.log_interval_sec;

  String output;
  serializeJson(doc, output);
  return output;
}

bool ConfigManager::from_json(const String& json_str) {
  // Zawsze zaczynaj od bezpiecznych domyślnych wartości,
  // aby brakujące klucze w starszym JSON nie zostawiały pól niezainicjalizowanych.
  cfg = default_config();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json_str);

  if (err) {
    Serial.printf("[CFG] JSON parse error: %s\n", err.c_str());
    return false;
  }

  // Wersja konfiguracji
  if (doc["config_version"].is<int>()) {
    cfg.config_version = doc["config_version"];
  }

  if (doc["tray_count"].is<int>())
    cfg.tray_count = constrain(doc["tray_count"].as<int>(), 1, MAX_TRAYS);

  // Trays
  if (doc["trays"].is<JsonArray>()) {
    JsonArray arr = doc["trays"].as<JsonArray>();
    int i = 0;
    for (JsonObject t : arr) {
      if (i >= MAX_TRAYS) break;
      cfg.trays[i].active = t["active"] | cfg.trays[i].active;
      strlcpy(cfg.trays[i].species, t["species"] | "CHICKEN", 12);
      cfg.trays[i].start_day_offset = t["start_day_offset"] | 0;
      cfg.trays[i].start_unix = t["start_unix"] | 0ULL;
      cfg.trays[i].pin = t["pin"] | (18 + i);
      strlcpy(cfg.trays[i].label, t["label"] | "Taca", 24);
      cfg.trays[i].signal_active_low = t["signal_active_low"] | cfg.trays[i].signal_active_low;
      cfg.trays[i].has_angle_sensor = t["has_angle_sensor"] | cfg.trays[i].has_angle_sensor;
      cfg.trays[i].rotation_interval_ms = t["rotation_interval_ms"] | 17280000;
      cfg.trays[i].rotation_duration_ms = t["rotation_duration_ms"] | 15000;
      int pwm_in = t["rotation_pwm_speed"] | 102;
      cfg.trays[i].rotation_pwm_speed = constrain(pwm_in, 0, 255);
      cfg.trays[i].rotations_per_day = t["rotations_per_day"] | 5;
      i++;
    }
  }

  // Sensors
  if (doc["sensors"].is<JsonObject>()) {
    JsonObject s = doc["sensors"].as<JsonObject>();
    cfg.sensors.ds18b20_count = constrain(s["ds18b20_count"] | cfg.sensors.ds18b20_count, 0, MAX_SENSORS);

    if (s["ds18b20_pins"].is<JsonArray>()) {
      JsonArray p = s["ds18b20_pins"].as<JsonArray>();
      for (int i = 0; i < MAX_SENSORS && i < (int)p.size(); i++)
        cfg.sensors.ds18b20_pins[i] = p[i];
    }
    if (s["ds18b20_labels"].is<JsonArray>()) {
      JsonArray l = s["ds18b20_labels"].as<JsonArray>();
      for (int i = 0; i < MAX_SENSORS && i < (int)l.size(); i++)
        strlcpy(cfg.sensors.ds18b20_labels[i], l[i].as<const char*>(), 20);
    }

    // Accept both dht11_* and legacy dht22_* config keys.
    cfg.sensors.dht11_enabled = s["dht11_enabled"] | (s["dht22_enabled"] | cfg.sensors.dht11_enabled);
    cfg.sensors.dht11_pin = s["dht11_pin"] | (s["dht22_pin"] | cfg.sensors.dht11_pin);
    cfg.sensors.water_level_enabled = s["water_level_enabled"] | cfg.sensors.water_level_enabled;
    cfg.sensors.water_level_pin = s["water_level_pin"] | cfg.sensors.water_level_pin;
    cfg.sensors.door_sensor_enabled = s["door_sensor_enabled"] | cfg.sensors.door_sensor_enabled;
    cfg.sensors.door_sensor_pin = s["door_sensor_pin"] | cfg.sensors.door_sensor_pin;
    cfg.sensors.aquarium_sensor_index = s["aquarium_sensor_index"] | cfg.sensors.aquarium_sensor_index;
    cfg.sensors.aquarium_target_temp = s["aquarium_target_temp"] | cfg.sensors.aquarium_target_temp;
    cfg.sensors.bmp180_enabled = s["bmp180_enabled"] | cfg.sensors.bmp180_enabled;
    cfg.sensors.bmp180_sda_pin = s["bmp180_sda_pin"] | cfg.sensors.bmp180_sda_pin;
    cfg.sensors.bmp180_scl_pin = s["bmp180_scl_pin"] | cfg.sensors.bmp180_scl_pin;
  }

  // Diffusers
  if (doc["diffusers"].is<JsonObject>()) {
    JsonObject d = doc["diffusers"].as<JsonObject>();
    cfg.diffusers.count = constrain(d["count"] | cfg.diffusers.count, 0, MAX_DIFFUSERS);
    cfg.diffusers.on_time_sec = d["on_time_sec"] | cfg.diffusers.on_time_sec;
    cfg.diffusers.off_time_sec = d["off_time_sec"] | cfg.diffusers.off_time_sec;

    if (d["pins"].is<JsonArray>()) {
      JsonArray p = d["pins"].as<JsonArray>();
      for (int i = 0; i < MAX_DIFFUSERS && i < (int)p.size(); i++)
        cfg.diffusers.pins[i] = p[i];
    }
    if (d["labels"].is<JsonArray>()) {
      JsonArray l = d["labels"].as<JsonArray>();
      for (int i = 0; i < MAX_DIFFUSERS && i < (int)l.size(); i++)
        strlcpy(cfg.diffusers.labels[i], l[i].as<const char*>(), 20);
    }
    if (d["profile"].is<const char*>()) {
      strlcpy(cfg.diffusers.profile, d["profile"].as<const char*>(), sizeof(cfg.diffusers.profile));
    }
  }

  // System
  if (doc["system"].is<JsonObject>()) {
    JsonObject sys = doc["system"].as<JsonObject>();
    cfg.system.test_mode = sys["test_mode"] | cfg.system.test_mode;
    cfg.system.fan_pin = sys["fan_pin"] | cfg.system.fan_pin;
    cfg.system.heater_pin_1 = sys["heater_pin_1"] | cfg.system.heater_pin_1;
    cfg.system.heater_pin_2 = sys["heater_pin_2"] | cfg.system.heater_pin_2;
    cfg.system.heater_pin_3 = sys["heater_pin_3"] | cfg.system.heater_pin_3;
    cfg.system.heater_pin_4 = sys["heater_pin_4"] | cfg.system.heater_pin_4;
    cfg.system.target_temp = sys["target_temp"] | cfg.system.target_temp;
    cfg.system.log_interval_sec = sys["log_interval_sec"] | cfg.system.log_interval_sec;
  }

  sanitize_config(cfg);
  return true;
}

static bool is_valid_pin(int pin) {
  return pin >= 0 && pin <= 48;
}

static void sanitize_config(IncubatorConfig& cfg) {
  const IncubatorConfig defaults = default_config();

  cfg.tray_count = constrain(cfg.tray_count, 1, MAX_TRAYS);
  for (int i = 0; i < cfg.tray_count; i++) {
    if (cfg.trays[i].rotation_interval_ms < 1000UL) cfg.trays[i].rotation_interval_ms = defaults.trays[i].rotation_interval_ms;
    if (cfg.trays[i].rotation_duration_ms < 1000UL) cfg.trays[i].rotation_duration_ms = defaults.trays[i].rotation_duration_ms;
    cfg.trays[i].rotation_pwm_speed = constrain(cfg.trays[i].rotation_pwm_speed, 0, 255);
    cfg.trays[i].rotations_per_day = constrain(cfg.trays[i].rotations_per_day, 0, 48);

    // --- NOWA WALIDACJA PINÓW TAC ---
    int pin = cfg.trays[i].pin;
    if (!is_valid_pin(pin) || pin == 19 || pin == 20) {
      if (i == 0) cfg.trays[i].pin = 16;
      else if (i == 1) cfg.trays[i].pin = 17;
      else cfg.trays[i].pin = 18;
      Serial.printf("[CFG] Taca %d: nieprawidłowy pin -> ustawiono GPIO%d\n", i, cfg.trays[i].pin);
    }
  }

  cfg.sensors.ds18b20_count = constrain(cfg.sensors.ds18b20_count, 0, MAX_SENSORS);
  for (int i = cfg.sensors.ds18b20_count; i < MAX_SENSORS; i++) {
    cfg.sensors.ds18b20_pins[i] = -1;
  }
  for (int i = 0; i < MAX_SENSORS; i++) {
    if (!is_valid_pin(cfg.sensors.ds18b20_pins[i])) {
      cfg.sensors.ds18b20_pins[i] = -1;
    }
  }

  if (!is_valid_pin(cfg.sensors.dht11_pin)) cfg.sensors.dht11_pin = defaults.sensors.dht11_pin;
  if (!is_valid_pin(cfg.sensors.water_level_pin)) cfg.sensors.water_level_pin = -1;
  if (!is_valid_pin(cfg.sensors.door_sensor_pin)) cfg.sensors.door_sensor_pin = -1;
  cfg.sensors.aquarium_sensor_index = constrain(cfg.sensors.aquarium_sensor_index, 0, MAX_SENSORS - 1);
  if (cfg.sensors.aquarium_target_temp < 0.0f || cfg.sensors.aquarium_target_temp > 100.0f)
    cfg.sensors.aquarium_target_temp = defaults.sensors.aquarium_target_temp;
  if (!is_valid_pin(cfg.sensors.i2c_sda_pin)) cfg.sensors.i2c_sda_pin = cfg.sensors.bmp180_sda_pin;
  if (!is_valid_pin(cfg.sensors.i2c_scl_pin)) cfg.sensors.i2c_scl_pin = cfg.sensors.bmp180_scl_pin;
  if (!is_valid_pin(cfg.sensors.i2c_sda_pin)) cfg.sensors.i2c_sda_pin = defaults.sensors.i2c_sda_pin;
  if (!is_valid_pin(cfg.sensors.i2c_scl_pin)) cfg.sensors.i2c_scl_pin = defaults.sensors.i2c_scl_pin;

  if (cfg.sensors.bmp180_enabled) {
    cfg.sensors.bmp180_sda_pin = cfg.sensors.i2c_sda_pin;
    cfg.sensors.bmp180_scl_pin = cfg.sensors.i2c_scl_pin;
  } else {
    if (!is_valid_pin(cfg.sensors.bmp180_sda_pin)) cfg.sensors.bmp180_sda_pin = defaults.sensors.bmp180_sda_pin;
    if (!is_valid_pin(cfg.sensors.bmp180_scl_pin)) cfg.sensors.bmp180_scl_pin = defaults.sensors.bmp180_scl_pin;
  }

  cfg.diffusers.count = constrain(cfg.diffusers.count, 0, MAX_DIFFUSERS);
  cfg.diffusers.on_time_sec = constrain(cfg.diffusers.on_time_sec, 1, 86400);
  cfg.diffusers.off_time_sec = constrain(cfg.diffusers.off_time_sec, 1, 86400);
  if (strcmp(cfg.diffusers.profile, "fast") != 0 && strcmp(cfg.diffusers.profile, "balanced") != 0 && strcmp(cfg.diffusers.profile, "eco") != 0) {
    strlcpy(cfg.diffusers.profile, defaults.diffusers.profile, sizeof(cfg.diffusers.profile));
  }
  for (int i = cfg.diffusers.count; i < MAX_DIFFUSERS; i++) {
    cfg.diffusers.pins[i] = -1;
  }
  for (int i = 0; i < MAX_DIFFUSERS; i++) {
    if (!is_valid_pin(cfg.diffusers.pins[i])) cfg.diffusers.pins[i] = -1;
  }

  cfg.system.log_interval_sec = constrain(cfg.system.log_interval_sec, 1, 86400);
  if (!is_valid_pin(cfg.system.fan_pin)) cfg.system.fan_pin = defaults.system.fan_pin;
  if (!is_valid_pin(cfg.system.heater_pin_1)) cfg.system.heater_pin_1 = defaults.system.heater_pin_1;
  if (!is_valid_pin(cfg.system.heater_pin_2)) cfg.system.heater_pin_2 = defaults.system.heater_pin_2;
  if (cfg.system.heater_pin_3 != -1 && !is_valid_pin(cfg.system.heater_pin_3)) cfg.system.heater_pin_3 = -1;
  if (cfg.system.heater_pin_4 != -1 && !is_valid_pin(cfg.system.heater_pin_4)) cfg.system.heater_pin_4 = -1;
  if (cfg.system.target_temp < 0.0f || cfg.system.target_temp > 100.0f)
    cfg.system.target_temp = defaults.system.target_temp;
}

// ============================================================================
// Globalna instancja
// ============================================================================
ConfigManager configMgr;
