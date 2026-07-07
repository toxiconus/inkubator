// config_manager.cpp
#include "config_manager.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

ConfigManager configMgr;

ConfigManager::ConfigManager() {
    cfg = default_config();
}

IncubatorConfig default_config() {
    IncubatorConfig cfg;
    cfg.config_version = CONFIG_VERSION;
    cfg.tray_count = 2;
    // Tace
    cfg.trays[0].pin = 16;
    cfg.trays[1].pin = 17;
    strcpy(cfg.trays[0].label, "Taca 1");
    strcpy(cfg.trays[1].label, "Taca 2");
    // Sensory
    cfg.sensors.ds18b20_count = 1;
    cfg.sensors.ds18b20_pins[0] = 6;
    cfg.sensors.dht11_enabled = true;
    cfg.sensors.dht11_pin = 8;
    cfg.sensors.i2c_sda_pin = 4;
    cfg.sensors.i2c_scl_pin = 5;
    // System
    cfg.system.heater_pin_1 = 1;
    cfg.system.heater_pin_2 = 2;
    cfg.system.target_temp = 37.7;
    cfg.system.temp_hysteresis = 0.5;
    cfg.system.alarm_max_temp = 40.0;
    cfg.system.alarm_min_temp = 36.0;
    cfg.system.temp_calibration = 0.0;
    cfg.system.control_mode = 0;
    cfg.system.fan_delay_sec = 5;
    cfg.system.target_humidity = 60.0;
    cfg.system.humidity_hysteresis = 5.0;
    cfg.system.alarm_max_humidity = 75.0;
    cfg.system.alarm_min_humidity = 30.0;
    cfg.system.humidity_calibration = 0.0;
    return cfg;
}

bool ConfigManager::load() {
    if (load_spiffs()) return true;
    if (load_preferences()) return true;
    cfg = default_config();
    return false;
}

bool ConfigManager::save() {
    save_spiffs();
    save_preferences();
    return true;
}

void ConfigManager::factory_reset() {
    cfg = default_config();
    save();
}

bool ConfigManager::load_spiffs() {
    Serial.println("[CFG] load_spiffs: begin");
    if (!LittleFS.begin(true)) {
        Serial.println("[CFG] load_spiffs: LittleFS.begin failed");
        return false;
    }
    bool exists = LittleFS.exists("/config.json");
    Serial.printf("[CFG] load_spiffs: /config.json exists=%d\n", exists ? 1 : 0);
    if (!exists) return false;
    File f = LittleFS.open("/config.json", "r");
    if (!f) {
        Serial.println("[CFG] load_spiffs: open failed");
        return false;
    }
    String json = f.readString();
    f.close();
    Serial.printf("[CFG] load_spiffs: read %u bytes\n", json.length());
    return from_json(json);
}

bool ConfigManager::save_spiffs() {
    if (!LittleFS.begin(true)) return false;
    String json = to_json();
    File f = LittleFS.open("/config.json.tmp", "w");
    if (!f) return false;
    f.print(json);
    f.close();
    if (LittleFS.exists("/config.json")) LittleFS.remove("/config.json");
    LittleFS.rename("/config.json.tmp", "/config.json");
    return true;
}

void ConfigManager::save_preferences() {
    pref.begin(PREF_NAMESPACE, false);
    String json = to_json();
    pref.putString(PREF_KEY_CFG, json);
    pref.end();
}

bool ConfigManager::load_preferences() {
    Serial.println("[CFG] load_preferences: begin");
    pref.begin(PREF_NAMESPACE, true);
    bool hasKey = pref.isKey(PREF_KEY_CFG);
    Serial.printf("[CFG] load_preferences: hasKey=%d\n", hasKey ? 1 : 0);
    if (!hasKey) { pref.end(); return false; }
    String json = pref.getString(PREF_KEY_CFG, "");
    pref.end();
    Serial.printf("[CFG] load_preferences: read %u bytes\n", json.length());
    return from_json(json);
}

void ConfigManager::save_learning_data() {
    pref.begin(PREF_NAMESPACE, false);
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
    pref.end();
}

bool ConfigManager::load_learning_data() {
    pref.begin(PREF_NAMESPACE, true);
    if (!pref.isKey(PREF_KEY_LEARNING_CFG)) { pref.end(); return false; }
    String json = pref.getString(PREF_KEY_LEARNING_CFG, "");
    pref.end();
    JsonDocument doc;
    if (deserializeJson(doc, json)) return false;
    learning_state.has_value = doc["has_value"] | false;
    learning_state.learned_pwm_pct = doc["learned_pwm_pct"] | 0.0f;
    learning_state.learned_target_temp = doc["learned_target_temp"] | NAN;
    learning_state.learned_target_humidity = doc["learned_target_humidity"] | NAN;
    learning_state.sample_count = doc["sample_count"] | 0;
    learning_state.confirmed_cycles = doc["confirmed_cycles"] | 0;
    learning_state.last_updated_ms = doc["last_updated_ms"] | 0ULL;
    learning_state.diffuser_on_ms = doc["diffuser_on_ms"] | 0;
    learning_state.diffuser_off_ms = doc["diffuser_off_ms"] | 0;
    return true;
}

void ConfigManager::clear_learning_data() {
    learning_state = LearningState();
    pref.begin(PREF_NAMESPACE, false);
    pref.remove(PREF_KEY_LEARNING_CFG);
    pref.remove(PREF_KEY_LEARNING_CRC);
    pref.end();
}

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
    JsonObject sensors = doc["sensors"].to<JsonObject>();
    sensors["ds18b20_count"] = cfg.sensors.ds18b20_count;
    JsonArray sensorPins = sensors["ds18b20_pins"].to<JsonArray>();
    for (int i = 0; i < MAX_SENSORS; i++) {
        sensorPins.add(cfg.sensors.ds18b20_pins[i]);
    }
    JsonArray sensorLabels = sensors["ds18b20_labels"].to<JsonArray>();
    for (int i = 0; i < MAX_SENSORS; i++) {
        sensorLabels.add(cfg.sensors.ds18b20_labels[i]);
    }
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
    JsonObject sys = doc["system"].to<JsonObject>();
    sys["target_temp"] = cfg.system.target_temp;
    sys["temp_hysteresis"] = cfg.system.temp_hysteresis;
    sys["alarm_max_temp"] = cfg.system.alarm_max_temp;
    sys["alarm_min_temp"] = cfg.system.alarm_min_temp;
    sys["temp_calibration"] = cfg.system.temp_calibration;
    sys["control_mode"] = cfg.system.control_mode;
    sys["fan_delay_sec"] = cfg.system.fan_delay_sec;
    sys["target_humidity"] = cfg.system.target_humidity;
    sys["humidity_hysteresis"] = cfg.system.humidity_hysteresis;
    sys["alarm_max_humidity"] = cfg.system.alarm_max_humidity;
    sys["alarm_min_humidity"] = cfg.system.alarm_min_humidity;
    sys["humidity_calibration"] = cfg.system.humidity_calibration;
    sys["log_interval_sec"] = cfg.system.log_interval_sec;
    JsonObject dif = doc["diffusers"].to<JsonObject>();
    dif["count"] = cfg.diffusers.count;
    dif["on_time_sec"] = cfg.diffusers.on_time_sec;
    dif["off_time_sec"] = cfg.diffusers.off_time_sec;
    dif["profile"] = cfg.diffusers.profile;
    JsonArray difPins = dif["pins"].to<JsonArray>();
    for (int i = 0; i < MAX_DIFFUSERS; i++) {
        difPins.add(cfg.diffusers.pins[i]);
    }
    JsonArray difLabels = dif["labels"].to<JsonArray>();
    for (int i = 0; i < MAX_DIFFUSERS; i++) {
        difLabels.add(cfg.diffusers.labels[i]);
    }
    String output;
    serializeJson(doc, output);
    return output;
}

bool ConfigManager::from_json(const String& json) {
    Serial.printf("[CFG] from_json: begin json=%u bytes\n", json.length());
    cfg = default_config();
    DynamicJsonDocument doc(8192);
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        Serial.printf("[CFG] from_json: deserialize failed %s\n", err.c_str());
        return false;
    }
    Serial.println("[CFG] from_json: deserialize OK");
    int fileVersion = doc["config_version"] | -1;
    if (fileVersion != CONFIG_VERSION) {
        Serial.printf("[CFG] config version mismatch %d != %d, resetting defaults\n", fileVersion, CONFIG_VERSION);
        cfg = default_config();
        return false;
    }
    cfg.config_version = fileVersion;
    cfg.tray_count = doc["tray_count"] | 2;
    JsonArray traysArray = doc["trays"].as<JsonArray>();
    if (!traysArray.isNull()) {
        for (int i = 0; i < MAX_TRAYS && i < (int)traysArray.size(); i++) {
            JsonObject t = traysArray[i].as<JsonObject>();
            if (t.isNull()) continue;
            cfg.trays[i].active = t["active"] | cfg.trays[i].active;
            const char* species = t["species"] | cfg.trays[i].species;
            strncpy(cfg.trays[i].species, species, sizeof(cfg.trays[i].species) - 1);
            cfg.trays[i].species[sizeof(cfg.trays[i].species) - 1] = '\0';
            cfg.trays[i].start_day_offset = t["start_day_offset"] | cfg.trays[i].start_day_offset;
            cfg.trays[i].start_unix = t["start_unix"] | cfg.trays[i].start_unix;
            cfg.trays[i].pin = t["pin"] | cfg.trays[i].pin;
            const char* label = t["label"] | cfg.trays[i].label;
            strncpy(cfg.trays[i].label, label, sizeof(cfg.trays[i].label) - 1);
            cfg.trays[i].label[sizeof(cfg.trays[i].label) - 1] = '\0';
            cfg.trays[i].signal_active_low = t["signal_active_low"] | cfg.trays[i].signal_active_low;
            cfg.trays[i].has_angle_sensor = t["has_angle_sensor"] | cfg.trays[i].has_angle_sensor;
            cfg.trays[i].rotation_interval_ms = t["rotation_interval_ms"] | cfg.trays[i].rotation_interval_ms;
            cfg.trays[i].rotation_duration_ms = t["rotation_duration_ms"] | cfg.trays[i].rotation_duration_ms;
            cfg.trays[i].rotation_pwm_speed = t["rotation_pwm_speed"] | cfg.trays[i].rotation_pwm_speed;
            cfg.trays[i].rotations_per_day = t["rotations_per_day"] | cfg.trays[i].rotations_per_day;
        }
    }
    JsonObject sensors = doc["sensors"].as<JsonObject>();
    if (!sensors.isNull()) {
        cfg.sensors.ds18b20_count = sensors["ds18b20_count"] | cfg.sensors.ds18b20_count;
        JsonArray sensorPins = sensors["ds18b20_pins"].as<JsonArray>();
        for (int i = 0; i < MAX_SENSORS && i < (int)sensorPins.size(); i++) {
            cfg.sensors.ds18b20_pins[i] = sensorPins[i] | cfg.sensors.ds18b20_pins[i];
        }
        JsonArray sensorLabels = sensors["ds18b20_labels"].as<JsonArray>();
        for (int i = 0; i < MAX_SENSORS && i < (int)sensorLabels.size(); i++) {
            const char* label = sensorLabels[i] | cfg.sensors.ds18b20_labels[i];
            strncpy(cfg.sensors.ds18b20_labels[i], label, sizeof(cfg.sensors.ds18b20_labels[i]) - 1);
            cfg.sensors.ds18b20_labels[i][sizeof(cfg.sensors.ds18b20_labels[i]) - 1] = '\0';
        }
        cfg.sensors.dht11_enabled = sensors["dht11_enabled"] | cfg.sensors.dht11_enabled;
        cfg.sensors.dht11_pin = sensors["dht11_pin"] | cfg.sensors.dht11_pin;
        cfg.sensors.water_level_enabled = sensors["water_level_enabled"] | cfg.sensors.water_level_enabled;
        cfg.sensors.water_level_pin = sensors["water_level_pin"] | cfg.sensors.water_level_pin;
        cfg.sensors.door_sensor_enabled = sensors["door_sensor_enabled"] | cfg.sensors.door_sensor_enabled;
        cfg.sensors.door_sensor_pin = sensors["door_sensor_pin"] | cfg.sensors.door_sensor_pin;
        cfg.sensors.aquarium_sensor_index = sensors["aquarium_sensor_index"] | cfg.sensors.aquarium_sensor_index;
        cfg.sensors.aquarium_target_temp = sensors["aquarium_target_temp"] | cfg.sensors.aquarium_target_temp;
        cfg.sensors.bmp180_enabled = sensors["bmp180_enabled"] | cfg.sensors.bmp180_enabled;
        cfg.sensors.bmp180_sda_pin = sensors["bmp180_sda_pin"] | cfg.sensors.bmp180_sda_pin;
        cfg.sensors.bmp180_scl_pin = sensors["bmp180_scl_pin"] | cfg.sensors.bmp180_scl_pin;
        cfg.sensors.i2c_sda_pin = sensors["i2c_sda_pin"] | cfg.sensors.i2c_sda_pin;
        cfg.sensors.i2c_scl_pin = sensors["i2c_scl_pin"] | cfg.sensors.i2c_scl_pin;
    }
    JsonObject sys = doc["system"].as<JsonObject>();
    if (!sys.isNull()) {
        cfg.system.target_temp = sys["target_temp"] | cfg.system.target_temp;
        cfg.system.temp_hysteresis = sys["temp_hysteresis"] | cfg.system.temp_hysteresis;
        cfg.system.alarm_max_temp = sys["alarm_max_temp"] | cfg.system.alarm_max_temp;
        cfg.system.alarm_min_temp = sys["alarm_min_temp"] | cfg.system.alarm_min_temp;
        cfg.system.temp_calibration = sys["temp_calibration"] | cfg.system.temp_calibration;
        cfg.system.control_mode = sys["control_mode"] | cfg.system.control_mode;
        cfg.system.fan_delay_sec = sys["fan_delay_sec"] | cfg.system.fan_delay_sec;
        cfg.system.target_humidity = sys["target_humidity"] | cfg.system.target_humidity;
        cfg.system.humidity_hysteresis = sys["humidity_hysteresis"] | cfg.system.humidity_hysteresis;
        cfg.system.alarm_max_humidity = sys["alarm_max_humidity"] | cfg.system.alarm_max_humidity;
        cfg.system.alarm_min_humidity = sys["alarm_min_humidity"] | cfg.system.alarm_min_humidity;
        cfg.system.humidity_calibration = sys["humidity_calibration"] | cfg.system.humidity_calibration;
        cfg.system.log_interval_sec = sys["log_interval_sec"] | cfg.system.log_interval_sec;
    }
    JsonObject dif = doc["diffusers"].as<JsonObject>();
    if (!dif.isNull()) {
        cfg.diffusers.count = dif["count"] | cfg.diffusers.count;
        cfg.diffusers.on_time_sec = dif["on_time_sec"] | cfg.diffusers.on_time_sec;
        cfg.diffusers.off_time_sec = dif["off_time_sec"] | cfg.diffusers.off_time_sec;
        const char* profile = dif["profile"] | cfg.diffusers.profile;
        strncpy(cfg.diffusers.profile, profile, sizeof(cfg.diffusers.profile) - 1);
        cfg.diffusers.profile[sizeof(cfg.diffusers.profile) - 1] = '\0';
        JsonArray difPins = dif["pins"].as<JsonArray>();
        for (int i = 0; i < MAX_DIFFUSERS && i < difPins.size(); i++) {
            cfg.diffusers.pins[i] = difPins[i] | cfg.diffusers.pins[i];
        }
        JsonArray difLabels = dif["labels"].as<JsonArray>();
        for (int i = 0; i < MAX_DIFFUSERS && i < difLabels.size(); i++) {
            const char* label = difLabels[i] | cfg.diffusers.labels[i];
            strncpy(cfg.diffusers.labels[i], label, sizeof(cfg.diffusers.labels[i]) - 1);
            cfg.diffusers.labels[i][sizeof(cfg.diffusers.labels[i]) - 1] = '\0';
        }
    }
    return true;
}
