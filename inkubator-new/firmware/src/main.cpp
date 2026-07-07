// main.cpp
#include <Arduino.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <FS.h>
#include <LittleFS.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <HTTPUpdateServer.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include <WebSocketsServer.h>

#include "config_manager.h"
#include "tray_controller.h"
#include "controller.h"
#include "calibration.h"
#include "alarm_manager.h"
#include "commands.h"
#include "incubation_profile.h"

// ============================================================================
//  KONFIGURACJA
// ============================================================================
#define WIFI_SSID "v2"
#define WIFI_PASSWORD "5654test"
#define WS_PORT 81
#define CSV_LOG_INTERVAL_MS 60000  // 1 minuta

static const IPAddress WIFI_AP_IP(192, 168, 5, 1);
static const IPAddress WIFI_AP_NETMASK(255, 255, 255, 0);
#define OTA_USER "admin"
#define OTA_PASSWORD WIFI_PASSWORD

// ============================================================================
//  GLOBALNE
// ============================================================================
WebServer webServer(80);
HTTPUpdateServer httpUpdater;
WebSocketsServer webSocket(WS_PORT);

// Czujniki
OneWire* ds_wire[MAX_SENSORS] = {};
DallasTemperature* ds_sensor[MAX_SENSORS] = {};
DHT* dht_sensor = nullptr;

// RTC
RTC_DS3231 rtc;
bool rtc_ok = false;

// Stan
SensorData last_sensor = {};
unsigned long long system_start_ms = 0;
unsigned long long incubation_elapsed_base_ms = 0;
bool wifi_connected = false;
bool api_online = false;

// Profile
IncubationProfiles g_incubation_profiles;

// CSV log
unsigned long lastCsvWrite = 0;

// Kalibracja
CalibrationState calState;

// ============================================================================
//  DEKLARACJE FUNKCJI
// ============================================================================
void sensors_init();
void sensors_read(SensorData& data);
void webserver_setup();
void websocket_setup();
void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
void broadcastStatus();
void csvWriteLog(const SensorData& data);
void handleCalibrationCommand(uint8_t num, const String& cmd);
void updateAll();

static bool parseFloatValue(const String& text, float& out) {
    String temp = text;
    temp.trim();
    if (temp.length() == 0) return false;
    char* endPtr;
    out = strtof(temp.c_str(), &endPtr);
    return endPtr != temp.c_str() && *endPtr == '\0';
}

static bool parseIntValue(const String& text, int& out) {
    String temp = text;
    temp.trim();
    if (temp.length() == 0) return false;
    char* endPtr;
    long val = strtol(temp.c_str(), &endPtr, 10);
    if (endPtr == temp.c_str() || *endPtr != '\0') return false;
    out = (int)val;
    return true;
}

// ============================================================================
//  RTC DS3231 – PEŁNA OBSŁUGA
// ============================================================================

void rtc_sync_ntp() {
    if (!rtc_ok) {
        Serial.println("[RTC] RTC niedostępny – pomijam NTP");
        return;
    }

    Serial.println("[RTC] Synchronizing RTC with NTP...");
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 5000)) {
        Serial.println("[RTC] NTP sync failed");
        return;
    }
    DateTime now(timeinfo.tm_year + 1900,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_mday,
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec);
    rtc.adjust(now);
    Serial.printf("[RTC] RTC updated from NTP: %04d-%02d-%02d %02d:%02d:%02d\n",
                  now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
}

bool rtc_set_time(int year, int month, int day, int hour, int minute, int second) {
    if (!rtc_ok) return false;

    DateTime dt(year, month, day, hour, minute, second);
    rtc.adjust(dt);

    Serial.printf("[RTC] Czas ustawiony: %04d-%02d-%02d %02d:%02d:%02d\n",
        year, month, day, hour, minute, second);
    return true;
}

String rtc_get_time_string() {
    if (!rtc_ok) return "Brak RTC";

    DateTime now = rtc.now();
    char buf[30];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second());
    return String(buf);
}

unsigned long rtc_get_unix_time() {
    if (!rtc_ok) return 0;
    return rtc.now().unixtime();
}

bool rtc_has_lost_power() {
    if (!rtc_ok) return true;
    return rtc.lostPower();
}

// ============================================================================
//  SETUP
// ============================================================================
void setup() {
    Serial.begin(115200);

    if (!LittleFS.begin(true)) {
        Serial.println("[MAIN] CRITICAL: LittleFS mount failed! Restart za 2s...");
        delay(2000);
        ESP.restart();
    }

    // Wczytaj konfigurację
    Serial.println("[MAIN] Before configMgr.load()");
    bool configLoaded = configMgr.load();
    if (!configLoaded) {
        Serial.println("[MAIN] configMgr.load() failed, using default configuration");
    }
    Serial.println("[MAIN] After configMgr.load()");
    configMgr.load_learning_data();
    Serial.println("[MAIN] After load_learning_data()");

    // Wczytaj profile inkubacji
    Serial.println("[MAIN] Checking incubation profiles file...");
    if (LittleFS.exists("/incubation_profiles.json")) {
        if (loadAllProfiles("/incubation_profiles.json", g_incubation_profiles)) {
            Serial.printf("[MAIN] Wczytano %d profili gatunków\n", 
                g_incubation_profiles.species_map.size());
        } else {
            Serial.println("[MAIN] Błąd wczytywania profili");
        }
    } else {
        Serial.println("[MAIN] Brak pliku incubation_profiles.json");
    }

    // Inicjalizacja I2C i RTC
    Serial.println("[MAIN] Initializing I2C and RTC...");
    Wire.begin(configMgr.cfg.sensors.i2c_sda_pin, configMgr.cfg.sensors.i2c_scl_pin);
    if (rtc.begin()) {
        rtc_ok = true;
        if (rtc.lostPower()) {
            Serial.println("[MAIN] RTC has lost power, time may need resetting");
        }
    } else {
        rtc_ok = false;
        Serial.println("[MAIN] RTC initialization failed");
    }

    // Inicjalizacja sensorów
    Serial.println("[MAIN] Initializing sensors...");
    sensors_init();
    Serial.println("[MAIN] Sensors initialized");

    Serial.println("[MAIN] Initializing trays...");
    tray_init(0);
    Serial.println("[MAIN] Trays initialized");

    Serial.println("[MAIN] Updating tray days from RTC...");
    tray_update_days_from_rtc();

    // Wi-Fi AP
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(WIFI_AP_IP, WIFI_AP_IP, WIFI_AP_NETMASK);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    wifi_connected = true;
    Serial.printf("[WiFi] AP: %s, IP: %s\n", WIFI_SSID, WiFi.softAPIP().toString().c_str());

    // WebServer + WebSocket
    webserver_setup();
    websocket_setup();

    // LEDC dla grzałek
    ledcSetup(0, 25000, 10);
    ledcAttachPin(configMgr.cfg.system.heater_pin_1, 0);
    ledcSetup(1, 25000, 10);
    ledcAttachPin(configMgr.cfg.system.heater_pin_2, 1);

    // Dyfuzor
    if (configMgr.cfg.diffusers.count > 0 && configMgr.cfg.diffusers.pins[0] >= 0) {
        pinMode(configMgr.cfg.diffusers.pins[0], OUTPUT);
        digitalWrite(configMgr.cfg.diffusers.pins[0], HIGH);
    }

    Serial.println("[MAIN] Inkubator gotowy!");
}

// ============================================================================
//  LOOP
// ============================================================================
void loop() {
    webServer.handleClient();
    webSocket.loop();

    static unsigned long lastSensorRead = 0;
    static unsigned long lastControl = 0;
    static unsigned long lastWebSocketBroadcast = 0;

    unsigned long now = millis();

    static unsigned long lastRtcUpdate = 0;
    if (now - lastRtcUpdate > 3600000UL) {
        lastRtcUpdate = now;
        tray_update_days_from_rtc();
    }

    // Odczyt sensorów co 5s
    if (now - lastSensorRead > 5000) {
        lastSensorRead = now;
        sensors_read(last_sensor);
        check_alarms(last_sensor);
        
        // CSV log co minutę
        if (now - lastCsvWrite > CSV_LOG_INTERVAL_MS) {
            lastCsvWrite = now;
            csvWriteLog(last_sensor);
        }
    }

    // Sterowanie co 2s
    if (now - lastControl > 2000) {
        lastControl = now;
        heaters_control(last_sensor);
        diffusers_control(last_sensor);
            tray_update_all(now);
        lastWebSocketBroadcast = now;
        broadcastStatus();
    }

    delay(10);
}

// ============================================================================
//  SENSORY – RZECZYWISTY ODCZYT
// ============================================================================
void sensors_init() {
    SensorConfig& sc = configMgr.cfg.sensors;

    // DS18B20
    for (int i = 0; i < sc.ds18b20_count && i < MAX_SENSORS; i++) {
        int pin = sc.ds18b20_pins[i];
        if (pin < 0) continue;
        bool shared = false;
        for (int j = 0; j < i; j++) {
            if (sc.ds18b20_pins[j] == pin) shared = true;
        }
        if (!shared) {
            pinMode(pin, INPUT_PULLUP);
            ds_wire[i] = new OneWire(pin);
            ds_sensor[i] = new DallasTemperature(ds_wire[i]);
            ds_sensor[i]->begin();
            ds_sensor[i]->setResolution(12);
            ds_sensor[i]->setWaitForConversion(false);
            Serial.printf("[SENSOR] DS18B20 na GPIO%d\n", pin);
        }
    }

    // DHT11
    if (sc.dht11_enabled && sc.dht11_pin >= 0) {
        dht_sensor = new DHT(sc.dht11_pin, DHT11);
        dht_sensor->begin();
        Serial.printf("[SENSOR] DHT11 na GPIO%d\n", sc.dht11_pin);
    }
}

void sensors_read(SensorData& data) {
    SensorConfig& sc = configMgr.cfg.sensors;
    data.is_dummy = (sc.ds18b20_count == 0 && !sc.dht11_enabled);

    // DS18B20
    for (int i = 0; i < sc.ds18b20_count && i < MAX_SENSORS; i++) {
        int bus = i;
        int dev_idx = 0;
        for (int j = 0; j < i; j++) {
            if (sc.ds18b20_pins[j] == sc.ds18b20_pins[i]) {
                bus = j;
                dev_idx++;
                break;
            }
        }
        float t = -99.0f;
        if (bus >= 0 && bus < MAX_SENSORS && ds_sensor[bus]) {
            ds_sensor[bus]->requestTemperatures();
            t = ds_sensor[bus]->getTempCByIndex(dev_idx);
        }
        if (t == DEVICE_DISCONNECTED_C || t < -50 || t > 125) {
            data.temp[i] = -99.0f;
        } else {
            data.temp[i] = t + configMgr.cfg.system.temp_calibration;
        }
    }
    for (int i = sc.ds18b20_count; i < MAX_SENSORS; i++) {
        data.temp[i] = -99.0f;
    }

    // DHT11
    if (sc.dht11_enabled && dht_sensor) {
        float h = dht_sensor->readHumidity();
        if (!isnan(h) && h >= 0 && h <= 100) {
            float calibrated = h + configMgr.cfg.system.humidity_calibration;
            data.humidity = constrain(calibrated, 0.0f, 100.0f);
        } else {
            data.humidity = -1.0f;
        }
    } else {
        data.humidity = -1.0f;
    }

    data.elapsed_ms = millis();
    data.water_level = -1;
    data.door_open = false;
    data.heater_pwm_pct = (int)heater_pwm_pct;
    data.heater_pwm_raw = (int)round(constrain(heater_pwm_pct, 0, 100) * 10.23f);
    data.diffuser_on = diffuser_on;
    data.diffuser_active_idx = diffuser_active_idx;
}

// ============================================================================
//  CSV LOG DO LittleFS
// ============================================================================
void csvWriteLog(const SensorData& data) {
    if (!LittleFS.begin(true)) return;

    bool exists = LittleFS.exists("/inkubator_log.csv");
    File f = LittleFS.open("/inkubator_log.csv", "a");
    if (!f) return;

    if (!exists) {
        f.println("timestamp_ms,temp_chamber,temp_aquarium,humidity,heater_pct,heater_raw,diffuser_on");
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "%llu,%.2f,%.2f,%.2f,%d,%d,%d\n",
        millis(),
        data.temp[0],
        data.temp[1],
        data.humidity,
        data.heater_pwm_pct,
        data.heater_pwm_raw,
        data.diffuser_on ? 1 : 0);
    f.print(buf);
    f.close();

    // Rotacja jeśli > 1MB
    if (LittleFS.exists("/inkubator_log.csv")) {
        File check = LittleFS.open("/inkubator_log.csv", "r");
        if (check && check.size() > 1024 * 1024) {
            check.close();
            if (LittleFS.exists("/inkubator_log_old.csv")) {
                LittleFS.remove("/inkubator_log_old.csv");
            }
            LittleFS.rename("/inkubator_log.csv", "/inkubator_log_old.csv");
        } else if (check) {
            check.close();
        }
    }
}

// ============================================================================
//  WEBSOCKET – PEŁNA OBSŁUGA
// ============================================================================
void websocket_setup() {
    webSocket.onEvent(handleWebSocketMessage);
    webSocket.begin();
    Serial.printf("[WS] WebSocket na porcie %d\n", WS_PORT);
}

void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type != WStype_TEXT) return;

    String msg = String((char*)payload);
    msg.trim();
    Serial.printf("[WS] Odebrano: %s\n", msg.c_str());

    // ===== TEMPERATURA =====
    if (msg == "carregarConfiguracao" || msg == CMD_GET_TEMP_CONFIG || msg == "getTempConfig") {
        String cfg = "setTemp:" + String(configMgr.cfg.system.target_temp, 1) +
                     ",setpoint:" + String(configMgr.cfg.system.target_temp, 1) +
                     ",setHysteresis:" + String(configMgr.cfg.system.temp_hysteresis, 1) +
                     ",histerese:" + String(configMgr.cfg.system.temp_hysteresis, 1) +
                     ",setAlarmMaxTemp:" + String(configMgr.cfg.system.alarm_max_temp, 1) +
                     ",alarmMax:" + String(configMgr.cfg.system.alarm_max_temp, 1) +
                     ",setAlarmMinTemp:" + String(configMgr.cfg.system.alarm_min_temp, 1) +
                     ",alarmMin:" + String(configMgr.cfg.system.alarm_min_temp, 1) +
                     ",setTempCalibration:" + String(configMgr.cfg.system.temp_calibration, 1) +
                     ",calibracao:" + String(configMgr.cfg.system.temp_calibration, 1) +
                     ",setControlMode:" + String(configMgr.cfg.system.control_mode) +
                     ",mode:" + String(configMgr.cfg.system.control_mode) +
                     ",setFanDelay:" + String(configMgr.cfg.system.fan_delay_sec) +
                     ",ventiladorAtual:" + String(configMgr.cfg.system.fan_delay_sec);
        webSocket.sendTXT(num, cfg);
    }
    else if (msg.startsWith("setpoint:") || msg.startsWith(CMD_SET_TEMP)) {
        String valueText = msg.startsWith(CMD_SET_TEMP) ? msg.substring(strlen(CMD_SET_TEMP)) : msg.substring(9);
        float val;
        if (!parseFloatValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setTemp");
        } else {
            configMgr.cfg.system.target_temp = constrain(val, 36.0, 39.0);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setTemp");
        }
    }
    else if (msg.startsWith("histerese:") || msg.startsWith(CMD_SET_TEMP_HYSTERESIS)) {
        String valueText = msg.startsWith(CMD_SET_TEMP_HYSTERESIS) ? msg.substring(strlen(CMD_SET_TEMP_HYSTERESIS)) : msg.substring(10);
        float val;
        if (!parseFloatValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setHysteresis");
        } else {
            configMgr.cfg.system.temp_hysteresis = constrain(val, 0.2f, 1.0f);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setHysteresis");
        }
    }
    else if (msg.startsWith("alarmMax:") || msg.startsWith(CMD_SET_ALARM_MAX)) {
        String valueText = msg.startsWith(CMD_SET_ALARM_MAX) ? msg.substring(strlen(CMD_SET_ALARM_MAX)) : msg.substring(9);
        float val;
        if (!parseFloatValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setAlarmMaxTemp");
        } else {
            configMgr.cfg.system.alarm_max_temp = constrain(val, 38.0f, 40.0f);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setAlarmMaxTemp");
        }
    }
    else if (msg.startsWith("alarmMin:") || msg.startsWith(CMD_SET_ALARM_MIN)) {
        String valueText = msg.startsWith(CMD_SET_ALARM_MIN) ? msg.substring(strlen(CMD_SET_ALARM_MIN)) : msg.substring(9);
        float val;
        if (!parseFloatValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setAlarmMinTemp");
        } else {
            configMgr.cfg.system.alarm_min_temp = constrain(val, 34.0f, 36.0f);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setAlarmMinTemp");
        }
    }
    else if (msg.startsWith("calibracao:") || msg.startsWith(CMD_SET_TEMPERATURE_CAL)) {
        String valueText = msg.startsWith(CMD_SET_TEMPERATURE_CAL) ? msg.substring(strlen(CMD_SET_TEMPERATURE_CAL)) : msg.substring(11);
        float val;
        if (!parseFloatValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setTempCalibration");
        } else {
            configMgr.cfg.system.temp_calibration = constrain(val, -5.0f, 5.0f);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setTempCalibration");
        }
    }
    else if (msg.startsWith("mode:") || msg.startsWith(CMD_SET_CONTROL_MODE)) {
        String valueText = msg.startsWith(CMD_SET_CONTROL_MODE) ? msg.substring(strlen(CMD_SET_CONTROL_MODE)) : msg.substring(5);
        int val;
        if (!parseIntValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setControlMode");
        } else {
            configMgr.cfg.system.control_mode = constrain(val, 0, 1);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setControlMode");
        }
    }
    else if (msg.startsWith("salvarVentilador:") || msg.startsWith(CMD_SET_FAN_DELAY)) {
        String valueText = msg.startsWith(CMD_SET_FAN_DELAY) ? msg.substring(strlen(CMD_SET_FAN_DELAY)) : msg.substring(17);
        int val;
        if (!parseIntValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setFanDelay");
        } else {
            configMgr.cfg.system.fan_delay_sec = constrain(val, 0, 15);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setFanDelay");
        }
    }
    else if (msg == "limparMinMax") {
        webSocket.sendTXT(num, "ok:resetMinMax");
    }

    // ===== WILGOTNOŚĆ =====
    else if (msg == "carregarConfiguracaoUmidade" || msg == "getHumidityConfig") {
        String cfg = "humidityConfig:" + String(configMgr.cfg.system.target_humidity, 1) +
                     ",setpointUmidade:" + String(configMgr.cfg.system.target_humidity, 1) +
                     ",humidityHysteresis:" + String(configMgr.cfg.system.humidity_hysteresis, 1) +
                     ",histereseUmidade:" + String(configMgr.cfg.system.humidity_hysteresis, 1) +
                     ",humidityAlarmMax:" + String(configMgr.cfg.system.alarm_max_humidity, 1) +
                     ",alarmMaxUmidade:" + String(configMgr.cfg.system.alarm_max_humidity, 1) +
                     ",humidityAlarmMin:" + String(configMgr.cfg.system.alarm_min_humidity, 1) +
                     ",alarmMinUmidade:" + String(configMgr.cfg.system.alarm_min_humidity, 1) +
                     ",humidityCalibration:" + String(configMgr.cfg.system.humidity_calibration, 1) +
                     ",calibracaoUmidade:" + String(configMgr.cfg.system.humidity_calibration, 1) +
                     ",pumpTime:" + String(configMgr.cfg.diffusers.on_time_sec) +
                     ",tempoBomba:" + String(configMgr.cfg.diffusers.on_time_sec) +
                     ",waterCheckInterval:" + String(configMgr.cfg.diffusers.off_time_sec / 60) +
                     ",intervaloVerificacaoAgua:" + String(configMgr.cfg.diffusers.off_time_sec / 60);
        webSocket.sendTXT(num, cfg);
    }
    else if (msg.startsWith("setHumidity:") || msg.startsWith("setpointUmidade:")) {
        String valueText = msg.startsWith("setHumidity:") ? msg.substring(strlen("setHumidity:")) : msg.substring(16);
        float val;
        if (!parseFloatValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setHumidity");
        } else {
            configMgr.cfg.system.target_humidity = constrain(val, 30.0f, 90.0f);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setHumidity");
        }
    }
    else if (msg.startsWith("setHumidityHysteresis:") || msg.startsWith("histereseUmidade:")) {
        String valueText = msg.startsWith("setHumidityHysteresis:") ? msg.substring(strlen("setHumidityHysteresis:")) : msg.substring(18);
        float val;
        if (!parseFloatValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setHumidityHysteresis");
        } else {
            configMgr.cfg.system.humidity_hysteresis = constrain(val, 0.5f, 20.0f);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setHumidityHysteresis");
        }
    }
    else if (msg.startsWith("setHumidityAlarmMax:") || msg.startsWith("alarmMaxUmidade:")) {
        String valueText = msg.startsWith("setHumidityAlarmMax:") ? msg.substring(strlen("setHumidityAlarmMax:")) : msg.substring(16);
        float val;
        if (!parseFloatValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setHumidityAlarmMax");
        } else {
            configMgr.cfg.system.alarm_max_humidity = constrain(val, 40.0f, 100.0f);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setHumidityAlarmMax");
        }
    }
    else if (msg.startsWith("setHumidityAlarmMin:") || msg.startsWith("alarmMinUmidade:")) {
        String valueText = msg.startsWith("setHumidityAlarmMin:") ? msg.substring(strlen("setHumidityAlarmMin:")) : msg.substring(16);
        float val;
        if (!parseFloatValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setHumidityAlarmMin");
        } else {
            configMgr.cfg.system.alarm_min_humidity = constrain(val, 10.0f, 60.0f);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setHumidityAlarmMin");
        }
    }
    else if (msg.startsWith("setHumidityCalibration:") || msg.startsWith("calibracaoUmidade:")) {
        String valueText = msg.startsWith("setHumidityCalibration:") ? msg.substring(strlen("setHumidityCalibration:")) : msg.substring(17);
        float val;
        if (!parseFloatValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setHumidityCalibration");
        } else {
            configMgr.cfg.system.humidity_calibration = constrain(val, -10.0f, 10.0f);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setHumidityCalibration");
        }
    }
    else if (msg.startsWith("setPumpTime:") || msg.startsWith("tempoBomba:")) {
        String valueText = msg.startsWith("setPumpTime:") ? msg.substring(strlen("setPumpTime:")) : msg.substring(11);
        int val;
        if (!parseIntValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setPumpTime");
        } else {
            configMgr.cfg.diffusers.on_time_sec = constrain(val, 0, 60);
            configMgr.save();
            webSocket.sendTXT(num, "ok:setPumpTime");
        }
    }
    else if (msg.startsWith("setWaterCheckInterval:") || msg.startsWith("intervaloVerificacaoAgua:")) {
        String valueText = msg.startsWith("setWaterCheckInterval:") ? msg.substring(strlen("setWaterCheckInterval:")) : msg.substring(24);
        int val;
        if (!parseIntValue(valueText, val)) {
            webSocket.sendTXT(num, "err:setWaterCheckInterval");
        } else {
            configMgr.cfg.diffusers.off_time_sec = constrain(val, 0, 60) * 60;
            configMgr.save();
            webSocket.sendTXT(num, "ok:setWaterCheckInterval");
        }
    }

    // ===== OBRÓT (TURNER) =====
    else if (msg == "carregarConfiguracaoVirador" || msg == "getTurnerConfig") {
        String cfg = "turnerMode0:" + String(0) +
                     ",modoVirador0:" + String(0) +
                     ",turnerWorkTime0:" + String(configMgr.cfg.trays[0].rotation_duration_ms / 1000) +
                     ",tempoTrabalho0:" + String(configMgr.cfg.trays[0].rotation_duration_ms / 1000) +
                     ",turnerRestTime0:" + String(configMgr.cfg.trays[0].rotation_interval_ms / 3600000) +
                     ",tempoRepouso0:" + String(configMgr.cfg.trays[0].rotation_interval_ms / 3600000) +
                     ",turnerPwm0:" + String(configMgr.cfg.trays[0].rotation_pwm_speed) +
                     ",pwm0:" + String(configMgr.cfg.trays[0].rotation_pwm_speed) +
                     ",turnerMode1:" + String(0) +
                     ",modoVirador1:" + String(0) +
                     ",turnerWorkTime1:" + String(configMgr.cfg.trays[1].rotation_duration_ms / 1000) +
                     ",tempoTrabalho1:" + String(configMgr.cfg.trays[1].rotation_duration_ms / 1000) +
                     ",turnerRestTime1:" + String(configMgr.cfg.trays[1].rotation_interval_ms / 3600000) +
                     ",tempoRepouso1:" + String(configMgr.cfg.trays[1].rotation_interval_ms / 3600000) +
                     ",turnerPwm1:" + String(configMgr.cfg.trays[1].rotation_pwm_speed) +
                     ",pwm1:" + String(configMgr.cfg.trays[1].rotation_pwm_speed);
        webSocket.sendTXT(num, cfg);
    }
    else if (msg.startsWith("modoVirador0:") || msg.startsWith("setTurnerMode0:")) {
        webSocket.sendTXT(num, "ok:setTurnerMode0");
    }
    else if (msg.startsWith("tempoTrabalho0:") || msg.startsWith("setTurnerWorkTime0:")) {
        int val = msg.startsWith("setTurnerWorkTime0:") ? msg.substring(strlen("setTurnerWorkTime0:")).toInt() : msg.substring(15).toInt();
        configMgr.cfg.trays[0].rotation_duration_ms = constrain(val, 10, 60) * 1000UL;
        configMgr.save();
        webSocket.sendTXT(num, "ok:setTurnerWorkTime0");
    }
    else if (msg.startsWith("tempoRepouso0:") || msg.startsWith("setTurnerRestTime0:")) {
        int val = msg.startsWith("setTurnerRestTime0:") ? msg.substring(strlen("setTurnerRestTime0:")).toInt() : msg.substring(14).toInt();
        configMgr.cfg.trays[0].rotation_interval_ms = constrain(val, 1, 4) * 3600000UL;
        configMgr.save();
        webSocket.sendTXT(num, "ok:setTurnerRestTime0");
    }
    else if (msg.startsWith("pwm0:") || msg.startsWith("setTurnerPwm0:")) {
        int val = msg.startsWith("setTurnerPwm0:") ? msg.substring(strlen("setTurnerPwm0:")).toInt() : msg.substring(4).toInt();
        configMgr.cfg.trays[0].rotation_pwm_speed = constrain(val, 0, 255);
        configMgr.save();
        webSocket.sendTXT(num, "ok:setTurnerPwm0");
    }
    else if (msg.startsWith("modoVirador1:") || msg.startsWith("setTurnerMode1:")) {
        webSocket.sendTXT(num, "ok:setTurnerMode1");
    }
    else if (msg.startsWith("tempoTrabalho1:") || msg.startsWith("setTurnerWorkTime1:")) {
        int val = msg.startsWith("setTurnerWorkTime1:") ? msg.substring(strlen("setTurnerWorkTime1:")).toInt() : msg.substring(15).toInt();
        configMgr.cfg.trays[1].rotation_duration_ms = constrain(val, 10, 60) * 1000UL;
        configMgr.save();
        webSocket.sendTXT(num, "ok:setTurnerWorkTime1");
    }
    else if (msg.startsWith("tempoRepouso1:") || msg.startsWith("setTurnerRestTime1:")) {
        int val = msg.startsWith("setTurnerRestTime1:") ? msg.substring(strlen("setTurnerRestTime1:")).toInt() : msg.substring(14).toInt();
        configMgr.cfg.trays[1].rotation_interval_ms = constrain(val, 1, 4) * 3600000UL;
        configMgr.save();
        webSocket.sendTXT(num, "ok:setTurnerRestTime1");
    }
    else if (msg.startsWith("pwm1:") || msg.startsWith("setTurnerPwm1:")) {
        int val = msg.startsWith("setTurnerPwm1:") ? msg.substring(strlen("setTurnerPwm1:")).toInt() : msg.substring(4).toInt();
        configMgr.cfg.trays[1].rotation_pwm_speed = constrain(val, 0, 255);
        configMgr.save();
        webSocket.sendTXT(num, "ok:setTurnerPwm1");
    }
    else if (msg.startsWith("virador:start") || msg.startsWith("turnerStart:")) {
        int tray = 0;
        if (msg.startsWith("turnerStart:")) {
            tray = msg.substring(strlen("turnerStart:")).toInt();
        }
        tray_manual_move(tray, 15000, millis());
        String response = msg.startsWith("turnerStart:") ? "ok:turnerStart:" + String(tray) : "ok:virador:start";
        webSocket.sendTXT(num, response);
    }
    else if (msg.startsWith("virador:stop") || msg.startsWith("turnerStop:")) {
        int tray = 0;
        if (msg.startsWith("turnerStop:")) {
            tray = msg.substring(strlen("turnerStop:")).toInt();
        }
        tray_stop(tray, millis());
        String response = msg.startsWith("turnerStop:") ? "ok:turnerStop:" + String(tray) : "ok:virador:stop";
        webSocket.sendTXT(num, response);
    }

    // ===== INKUBACJA =====
    else if (msg == "carregarConfiguracaoIncubacao" || msg == "getIncubationConfig") {
        String cfg = "incubationDays:21,firstDaysEnabled:0,firstDays:3,turnerFirstDays:0,tempAdjustmentFirst:0,humidityAdjustmentFirst:0,lastDaysEnabled:0,lastDays:3,turnerLastDays:0,tempAdjustmentLast:0,humidityAdjustmentLast:0";
        webSocket.sendTXT(num, cfg);
    }
    else if (msg.startsWith("setStartUnix:") || msg.startsWith("setStartDate:")) {
        int firstColon = msg.indexOf(':');
        int secondColon = msg.indexOf(':', firstColon + 1);
        if (firstColon > 0 && secondColon > firstColon) {
            String trayText = msg.substring(firstColon + 1, secondColon);
            String unixText = msg.substring(secondColon + 1);
            int tray;
            int unixVal;
            if (parseIntValue(trayText, tray) && parseIntValue(unixText, unixVal) && tray >= 0) {
                tray_set_start_unix(tray, (unsigned long)unixVal);
                webSocket.sendTXT(num, msg.startsWith("setStartDate:") ? "ok:setStartDate:" + String(tray) : "ok:setStartUnix:" + String(tray));
            } else {
                webSocket.sendTXT(num, "err:setStartDate");
            }
        } else {
            webSocket.sendTXT(num, "err:setStartDate");
        }
    }
    else if (msg.startsWith("resetDay:")) {
        int tray = msg.substring(9).toInt();
        tray_reset_day(tray);
        webSocket.sendTXT(num, "ok:resetDay:" + String(tray));
    }
    else if (msg.startsWith("getStartUnix:") || msg.startsWith("getStartDate:")) {
        int tray = msg.substring(msg.indexOf(':') + 1).toInt();
        unsigned long long unix = tray_get_start_unix(tray);
        webSocket.sendTXT(num, msg.startsWith("getStartDate:") ? "startDate:" + String(tray) + ":" + String((unsigned long)unix) : "startUnix:" + String(tray) + ":" + String((unsigned long)unix));
    }

    // ===== ALARMY =====
    else if (msg == "carregarAlarmes" || msg == "getAlarms") {
        webSocket.sendTXT(num, "alarmsStatus:1,1,1,1,1,1,1,1,1,1");
    }
    else if (msg.startsWith("salvarAlarmes:")) {
        webSocket.sendTXT(num, "ok:salvarAlarmes");
    }

    // ===== WI-FI =====
    else if (msg == "carregarRede" || msg == "getWifiConfig") {
        webSocket.sendTXT(num, "wifiConfig:ssid=MojaSiec,senha=********");
    }
    else if (msg == "status" || msg == "getWifiStatus") {
        webSocket.sendTXT(num, "wifiStatus:connected");
    }
    else if (msg.startsWith("salvarRede:") || msg.startsWith("setWifi:")) {
        webSocket.sendTXT(num, "ok:setWifi");
    }
    else if (msg == "removerRede" || msg == "removeWifi") {
        webSocket.sendTXT(num, "ok:removeWifi");
    }

    // ===== RTC =====
    else if (msg == "carregarRTC" || msg == "getRtcTime") {
        String timeStr = rtc_get_time_string();
        webSocket.sendTXT(num, "rtcTime:" + timeStr);
    }
    else if (msg == "sincronizarNTP" || msg == "syncNtp") {
        rtc_sync_ntp();
        webSocket.sendTXT(num, "ok:syncNtp");
    }
    else if (msg.startsWith("rtcSet:") || msg.startsWith("setRtcTime:")) {
        String data = msg.substring(msg.indexOf(':') + 1);
        if (data.length() >= 19) {
            int year = data.substring(0, 4).toInt();
            int month = data.substring(5, 7).toInt();
            int day = data.substring(8, 10).toInt();
            int hour = data.substring(11, 13).toInt();
            int minute = data.substring(14, 16).toInt();
            int second = data.substring(17, 19).toInt();
            if (rtc_set_time(year, month, day, hour, minute, second)) {
                webSocket.sendTXT(num, "ok:setRtcTime");
            } else {
                webSocket.sendTXT(num, "err:setRtcTime");
            }
        } else {
            webSocket.sendTXT(num, "err:setRtcTime");
        }
    }
    else if (msg == "rtcStatus" || msg == "getRtcStatus") {
        String status = "rtcStatus:" + String(rtc_ok ? "OK" : "ERR") +
                        ",lostPower:" + String(rtc_has_lost_power() ? 1 : 0) +
                        ",time:" + rtc_get_time_string();
        webSocket.sendTXT(num, status);
    }
    
    // ===== RÓŻNE =====
    else if (msg == "versaoFirmware") {
        webSocket.sendTXT(num, "versaoFirmware:v2.1");
    }
    else if (msg.startsWith("idioma:")) {
        webSocket.sendTXT(num, "ok:idioma");
    }
    else if (msg == "carregarIdioma") {
        webSocket.sendTXT(num, "idiomaAtual:pl");
    }
    else if (msg == "redefinir") {
        configMgr.factory_reset();
        webSocket.sendTXT(num, "ok:redefinir");
    }
    else if (msg == "reiniciar") {
        webSocket.sendTXT(num, "ok:reiniciar");
        delay(500);
        ESP.restart();
    }

    // ===== KALIBRACJA TACY =====
    else {
        handleCalibrationCommand(num, msg);
    }
}

// ============================================================================
//  KALIBRACJA TACY – OBSŁUGA KOMEND
// ============================================================================
void handleCalibrationCommand(uint8_t num, const String& cmd) {
    // PWM
    if (cmd.startsWith("kalPwm:")) {
        int val = cmd.substring(7).toInt();
        calState.pwmPercent = constrain(val, 40, 100);
        webSocket.sendTXT(num, "ok:kalPwm");
    }
    // Faza
    else if (cmd.startsWith("kalPhase:")) {
        calState.phase = cmd.substring(9);
        webSocket.sendTXT(num, "ok:kalPhase");
    }
    // Start pomiaru
    else if (cmd == "kalStart") {
        calState.running = true;
        calState.startMs = millis();
        calState.elapsedMs = 0;
        webSocket.sendTXT(num, "ok:kalStart");
    }
    // Stop pomiaru
    else if (cmd == "kalStop") {
        calState.running = false;
        calState.elapsedMs = millis() - calState.startMs;
        webSocket.sendTXT(num, "ok:kalStop:" + String(calState.elapsedMs));
    }
    // Reset
    else if (cmd == "kalReset") {
        calState.running = false;
        calState.elapsedMs = 0;
        calState.seqRunning = false;
        calState.seqCount = 0;
        webSocket.sendTXT(num, "ok:kalReset");
    }
    // Sekwencja
    else if (cmd.startsWith("kalSeq:")) {
        int count = cmd.substring(6).toInt();
        calState.seqMax = constrain(count, 1, 50);
        calState.seqRunning = true;
        calState.seqStartMs = millis();
        calState.seqCount = 0;
        webSocket.sendTXT(num, "ok:kalSeq:" + String(calState.seqMax));
    }
    // Stop sekwencji
    else if (cmd == "kalSeqStop") {
        calState.seqRunning = false;
        webSocket.sendTXT(num, "ok:kalSeqStop");
    }
    // Status kalibracji
    else if (cmd == "kalStatus") {
        String status = "running:" + String(calState.running ? 1 : 0) +
                        ",elapsed:" + String(calState.elapsedMs) +
                        ",pwm:" + String(calState.pwmPercent) +
                        ",phase:" + calState.phase +
                        ",seqRunning:" + String(calState.seqRunning ? 1 : 0) +
                        ",seqCount:" + String(calState.seqCount) +
                        ",seqMax:" + String(calState.seqMax);
        webSocket.sendTXT(num, status);
    }
    // Ruch tacy (API)
    else if (cmd.startsWith("kalMove:")) {
        int tray = cmd.substring(8, 9).toInt();
        String dir = cmd.substring(10);
        int pwm = (calState.pwmPercent * 255) / 100;
        unsigned long duration = 2000;
        tray_manual_move(tray, duration, millis());
        webSocket.sendTXT(num, "ok:kalMove:" + dir);
    }
    // Stop ruchu
    else if (cmd == "kalStop") {
        tray_stop(0, millis());
        tray_stop(1, millis());
        webSocket.sendTXT(num, "ok:kalStop");
    }
    // OFF ALL (decay)
    else if (cmd == "kalOffAll") {
        heater_pwm_pct = 0;
        diffuser_on = false;
        for (int ch = 0; ch < 2; ch++) {
            ledcWrite(ch, 0);
        }
        if (configMgr.cfg.diffusers.pins[0] >= 0) {
            digitalWrite(configMgr.cfg.diffusers.pins[0], HIGH);
        }
        webSocket.sendTXT(num, "ok:kalOffAll");
    }
    else {
        // Unknown command fallback
        webSocket.sendTXT(num, "err:unknownCommand");
    }
}

// ============================================================================
//  BROADCAST STATUS
// ============================================================================
void broadcastStatus() {
    DynamicJsonDocument doc(1536);
    
    doc["temp_chamber"] = last_sensor.temp[0];
    doc["temp_aquarium"] = last_sensor.temp[1];
    doc["humidity"] = last_sensor.humidity;
    doc["target_temp"] = configMgr.cfg.system.target_temp;
    doc["heater_pwm_ch0"] = (int)heater_pwm_pct * 10.23f;
    doc["heater_pwm_ch1"] = (int)heater_pwm_pct * 10.23f;
    doc["diffuser_0_on"] = diffuser_on;
    doc["esp_internal_temp"] = temperatureRead();

    bool alarmTempHigh = false;
    bool alarmTempLow = false;
    bool alarmHumidityHigh = false;
    bool alarmHumidityLow = false;
    if (last_sensor.temp[0] > -50 && last_sensor.temp[0] < 80) {
        alarmTempHigh = last_sensor.temp[0] > configMgr.cfg.system.alarm_max_temp;
        alarmTempLow = last_sensor.temp[0] < configMgr.cfg.system.alarm_min_temp;
    }
    if (last_sensor.humidity >= 0 && last_sensor.humidity <= 100) {
        alarmHumidityHigh = last_sensor.humidity > configMgr.cfg.system.alarm_max_humidity;
        alarmHumidityLow = last_sensor.humidity < configMgr.cfg.system.alarm_min_humidity;
    }
    doc["alarm_temp_high"] = alarmTempHigh;
    doc["alarm_temp_low"] = alarmTempLow;
    doc["alarm_humidity_high"] = alarmHumidityHigh;
    doc["alarm_humidity_low"] = alarmHumidityLow;
    doc["tray0_start_unix"] = configMgr.cfg.trays[0].start_unix;
    doc["tray1_start_unix"] = configMgr.cfg.trays[1].start_unix;
    doc["uptime_ms"] = millis();

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

// ============================================================================
//  WEB SERVER
// ============================================================================
void webserver_setup() {
    webServer.on("/api/status", HTTP_GET, []() {
        StaticJsonDocument<512> doc;
        doc["temp_chamber"] = last_sensor.temp[0];
        doc["temp_aquarium"] = last_sensor.temp[1];
        doc["humidity"] = last_sensor.humidity;
        doc["target_temp"] = configMgr.cfg.system.target_temp;
        doc["heater_pwm_ch0"] = (int)heater_pwm_pct * 10.23f;
        doc["heater_pwm_ch1"] = (int)heater_pwm_pct * 10.23f;
        doc["diffuser_0_on"] = diffuser_on;
        doc["esp_internal_temp"] = temperatureRead();
        doc["uptime_ms"] = millis();
        String json;
        serializeJson(doc, json);
        webServer.send(200, "application/json", json);
    });

    webServer.on("/api/trays", HTTP_GET, []() {
        webServer.send(200, "application/json", tray_status_json(millis()));
    });

    webServer.on("/api/profile", HTTP_GET, []() {
        if (!webServer.hasArg("species")) {
            webServer.send(400, "application/json", "{\"error\":\"Brak parametru species\"}");
            return;
        }
        String species = webServer.arg("species");
        species.toLowerCase();
        const SpeciesProfile* sp = getSpeciesProfile(g_incubation_profiles, species);
        if (!sp) {
            webServer.send(404, "application/json", "{\"error\":\"Nieznany gatunek\"}");
            return;
        }
        JsonDocument doc;
        doc["species_id"] = sp->id;
        doc["common_name"] = sp->common_name;
        doc["incubation_days"] = sp->incubation_days;
        JsonArray daysArr = doc["schedule"].to<JsonArray>();
        for (int i = 0; i < sp->incubation_days && i < 30; i++) {
            JsonObject dayObj = daysArr.add<JsonObject>();
            const IncubationDay& d = sp->days[i];
            dayObj["day"] = d.day;
            dayObj["shell_temp"] = d.shell_temp;
            dayObj["air_temp_target"] = d.air_temp_target;
            dayObj["humidity_set"] = d.humidity_set;
            dayObj["humidity_min"] = d.humidity_min;
            dayObj["humidity_max"] = d.humidity_max;
            dayObj["turning"] = d.turning;
            dayObj["misting"] = d.misting;
            dayObj["sensitivity"] = d.sensitivity;
            dayObj["status_color"] = d.status_color;
            dayObj["notes"] = d.notes;
        }
        String output;
        serializeJson(doc, output);
        webServer.send(200, "application/json", output);
    });

    webServer.on("/api/log", HTTP_GET, []() {
        if (LittleFS.exists("/inkubator_log.csv")) {
            File f = LittleFS.open("/inkubator_log.csv", "r");
            webServer.sendHeader("Content-Disposition", "attachment; filename=inkubator_log.csv");
            webServer.streamFile(f, "text/csv");
            f.close();
        } else {
            webServer.send(404, "text/plain", "Brak pliku logu");
        }
    });

    webServer.on("/api/profiles", HTTP_GET, []() {
        if (LittleFS.exists("/incubation_profiles.json")) {
            File f = LittleFS.open("/incubation_profiles.json", "r");
            webServer.streamFile(f, "application/json");
            f.close();
        } else {
            webServer.send(404, "application/json", "{\"error\":\"Brak profili\"}");
        }
    });

    webServer.onNotFound([]() {
        String uri = webServer.uri();
        if (uri == "/" || uri == "/index.html") {
            if (LittleFS.exists("/index.html")) {
                File f = LittleFS.open("/index.html", "r");
                webServer.streamFile(f, "text/html");
                f.close();
                return;
            }
        }
        if (LittleFS.exists(uri)) {
            File f = LittleFS.open(uri, "r");
            webServer.streamFile(f, "text/html");
            f.close();
            return;
        }
        webServer.send(404, "text/plain", "404 Not Found");
    });

    httpUpdater.setup(&webServer, "/update", OTA_USER, OTA_PASSWORD);
    webServer.begin();
    Serial.println("[WEB] Serwer HTTP na porcie 80");
}