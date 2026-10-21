#include <Arduino.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <FS.h>
#include <LittleFS.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <HTTPUpdateServer.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include <MPU6050_light.h>

#include "incubation_schedule.h"
#include "config_manager.h"
#include "tray_controller.h"
#include "species_profiles.h"
#include "controller.h"

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 4096
#endif

#ifndef HISTORY_SIZE
#define HISTORY_SIZE 300
#endif

// ============================================================================
// KONFIGURACJA
// ============================================================================

#define WIFI_SSID "INKUBATOR"
#define WIFI_PASSWORD "5465INKUBATOR5465"
#define WIFI_STA_SSID ""
#define WIFI_STA_PASSWORD ""

static const IPAddress WIFI_AP_IP(192, 168, 5, 1);
static const IPAddress WIFI_AP_NETMASK(255, 255, 255, 0);

#define OTA_USER "admin"
#define OTA_PASSWORD WIFI_PASSWORD

// ============================================================================
// GLOBALNE
// ============================================================================
WebServer webServer(80);
HTTPUpdateServer httpUpdater;

File uploadFile;
String uploadPath;
String uploadResultPath;

unsigned long long system_start_ms = 0;
unsigned long long incubation_elapsed_base_ms = 0;
unsigned long long incubation_start_unix = 0;
SensorData last_sensor = {};
bool wifi_connected = false;
bool bmp180_ok = false;
bool rtc_ok = false;
float last_valid_humidity = -1.0f;
unsigned long last_valid_humidity_ms = 0;

// MPU6050 sensor
static MPU6050 mpu(Wire);
static bool mpu_sensor_ok = false;
static bool mpu_i2c_found = false;
static unsigned long last_mpu_read_ms = 0;
static float last_mpu_angle_deg = NAN;
static unsigned long dht_retry_start_ms = 0;
static int dht_retry_attempts = 0;
static bool ds18b20_conversion_pending = false;
static unsigned long ds18b20_request_ms = 0;
static const unsigned long DS18B20_CONVERSION_MS = 750UL;
static unsigned long long last_saved_elapsed_ms = 0;
static const char* JOURNAL_PATH = "/journal.json";
static const char* JOURNAL_TMP_PATH = "/journal.json.tmp";

struct HistoryEntry {
  unsigned long long t_min;
  float temp0;
  float temp1;
  float humidity;
  float heater_pct;
  int heater_pwm_raw;
  bool diffuser_on;
  int diffuser_active_idx;
};

static HistoryEntry history[HISTORY_SIZE] = {};
static int history_idx = 0;

RTC_DS3231 rtc;

// ============================================================================
// PINY GRZAŁEK
// ============================================================================

// ============================================================================
// EEPROM / START TIME
// ============================================================================
#define EEPROM_START_ADDR 200
#define EEPROM_ROTATION_ADDR 320
#define EEPROM_ROTATION_MAGIC 0x524F5431UL
#define EEPROM_START_MAGIC 0x53544152UL

enum StartTimeMode : uint8_t {
  START_MODE_MILLIS = 1,
  START_MODE_UNIX = 2
};

struct StartPersistBlock {
  uint32_t magic;
  uint8_t mode;
  uint8_t reserved[3];
  unsigned long long value;
};

struct RotationPersistEntry {
  bool has_last_rotation;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  unsigned long long elapsed_ms;
};

struct RotationPersistBlock {
  uint32_t magic;
  RotationPersistEntry tray[MAX_TRAYS];
};

static const unsigned long long EEPROM_SAVE_INTERVAL_MS = 1800000ULL;

void save_start_time(unsigned long long t, uint8_t mode) {
  StartPersistBlock blk = {};
  blk.magic = EEPROM_START_MAGIC;
  blk.mode = mode;
  blk.value = t;

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(EEPROM_START_ADDR, blk);
  EEPROM.commit();
  EEPROM.end();
}

bool should_save_fallback_elapsed(unsigned long long elapsed_ms) {
  return !rtc_ok && elapsed_ms > last_saved_elapsed_ms &&
         (elapsed_ms - last_saved_elapsed_ms) >= EEPROM_SAVE_INTERVAL_MS;
}

bool load_start_time(unsigned long long& t, uint8_t& mode) {
  StartPersistBlock blk = {};
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(EEPROM_START_ADDR, blk);

  if (blk.magic == EEPROM_START_MAGIC && (blk.mode == START_MODE_MILLIS || blk.mode == START_MODE_UNIX)) {
    t = blk.value;
    mode = blk.mode;
    EEPROM.end();
    return true;
  }

  unsigned long long legacy = 0;
  EEPROM.get(EEPROM_START_ADDR, legacy);
  EEPROM.end();

  if (legacy == 0ULL || legacy == 0xFFFFFFFFFFFFFFFFULL) return false;

  t = legacy;
  mode = START_MODE_MILLIS;
  return true;
}

unsigned long long get_incubation_elapsed_ms() {
  if (rtc_ok && incubation_start_unix > 0) {
    DateTime now = rtc.now();
    unsigned long long now_unix = now.unixtime();
    if (now_unix >= incubation_start_unix) {
      return (now_unix - incubation_start_unix) * 1000ULL;
    }
    return 0;
  }

  unsigned long long now_ms = millis();
  if (now_ms >= system_start_ms) {
    return incubation_elapsed_base_ms + (now_ms - system_start_ms);
  }
  return incubation_elapsed_base_ms + (0xFFFFFFFFULL - system_start_ms) + now_ms + 1ULL;
}

void save_rotation_state() {
  EEPROM.begin(EEPROM_SIZE);
  RotationPersistBlock blk = {};
  blk.magic = EEPROM_ROTATION_MAGIC;
  for (int i = 0; i < MAX_TRAYS; i++) {
    blk.tray[i].has_last_rotation = tray_states[i].has_last_rotation;
    blk.tray[i].day = (uint8_t)max(0, tray_states[i].last_rotation_day);
    blk.tray[i].hour = (uint8_t)max(0, tray_states[i].last_rotation_hour);
    blk.tray[i].minute = (uint8_t)max(0, tray_states[i].last_rotation_minute);
    blk.tray[i].elapsed_ms = tray_states[i].last_rotation_elapsed_ms;
  }
  EEPROM.put(EEPROM_ROTATION_ADDR, blk);
  EEPROM.commit();
  EEPROM.end();
}

void load_rotation_state() {
  EEPROM.begin(EEPROM_SIZE);
  RotationPersistBlock blk = {};
  EEPROM.get(EEPROM_ROTATION_ADDR, blk);
  EEPROM.end();

  if (blk.magic != EEPROM_ROTATION_MAGIC) return;

  for (int i = 0; i < MAX_TRAYS; i++) {
    tray_states[i].has_last_rotation = blk.tray[i].has_last_rotation;
    tray_states[i].last_rotation_day = (int)blk.tray[i].day;
    tray_states[i].last_rotation_hour = (int)blk.tray[i].hour;
    tray_states[i].last_rotation_minute = (int)blk.tray[i].minute;
    tray_states[i].last_rotation_elapsed_ms = blk.tray[i].elapsed_ms;
    tray_states[i].rotation_event_pending = false;
  }
}

void clear_rotation_state() {
  for (int i = 0; i < MAX_TRAYS; i++) {
    tray_states[i].has_last_rotation = false;
    tray_states[i].last_rotation_day = 0;
    tray_states[i].last_rotation_hour = 0;
    tray_states[i].last_rotation_minute = 0;
    tray_states[i].last_rotation_elapsed_ms = 0;
    tray_states[i].rotation_event_pending = false;
  }
  save_rotation_state();
}

void persist_rotation_events_if_needed() {
  bool dirty = false;
  for (int i = 0; i < MAX_TRAYS; i++) {
    if (tray_states[i].rotation_event_pending) {
      tray_states[i].rotation_event_pending = false;
      dirty = true;
    }
  }
  if (dirty) save_rotation_state();
}

// ============================================================================
// CZUJNIKI
// ============================================================================
OneWire* ds_wire[MAX_SENSORS] = {};
DallasTemperature* ds_sensor[MAX_SENSORS] = {};
DHT* dht_sensor = nullptr;
Adafruit_BMP085 bmp180;

void sensors_deinit() {
  for (int i = 0; i < MAX_SENSORS; i++) {
    if (ds_wire[i]) { delete ds_wire[i]; ds_wire[i] = nullptr; }
    if (ds_sensor[i]) { delete ds_sensor[i]; ds_sensor[i] = nullptr; }
  }
  if (dht_sensor) { delete dht_sensor; dht_sensor = nullptr; }
}

void sensors_init() {
  sensors_deinit();
  SensorConfig& sc = configMgr.cfg.sensors;

  for (int i = 0; i < sc.ds18b20_count && i < MAX_SENSORS; i++) {
    int pin = sc.ds18b20_pins[i];
    bool shared = false;
    for (int j = 0; j < i; j++) {
      if (sc.ds18b20_pins[j] == pin) { shared = true; break; }
    }
    if (!shared) {
      pinMode(pin, INPUT_PULLUP);
      ds_wire[i] = new OneWire(pin);
      if (ds_sensor[i]) delete ds_sensor[i];
      ds_sensor[i] = new DallasTemperature(ds_wire[i]);
      ds_sensor[i]->begin();
      ds_sensor[i]->setResolution(12);
      ds_sensor[i]->setWaitForConversion(false);
      int dev_count = ds_sensor[i]->getDeviceCount();
      for (int dev = 0; dev < dev_count; dev++) {
        DeviceAddress addr;
        if (ds_sensor[i]->getAddress(addr, dev)) {
          Serial.printf("[SENSOR]   GPIO%d dev[%d] ROM=", pin, dev);
          for (int b = 0; b < 8; b++) {
            Serial.printf("%02X", addr[b]);
            if (b < 7) Serial.print(":");
          }
          Serial.println();
        } else {
          Serial.printf("[SENSOR]   GPIO%d dev[%d] ROM=UNREADABLE\n", pin, dev);
        }
      }
      if (dev_count == 0) {
        Serial.printf("[SENSOR]   GPIO%d: brak odpowiedzi DS18B20\n", pin);
      }
    }
    Serial.printf("[SENSOR] DS18B20 #%d na GPIO%d (%s)%s\n",
      i, pin, sc.ds18b20_labels[i], shared ? " [wspólna magistrala]" : "");
  }

  if (sc.dht11_enabled) {
    dht_sensor = new DHT(sc.dht11_pin, DHT11);
    dht_sensor->begin();
    Serial.printf("[SENSOR] DHT11 na GPIO%d\n", sc.dht11_pin);
  }

  if (sc.water_level_enabled) {
    pinMode(sc.water_level_pin, INPUT_PULLUP);
    Serial.printf("[SENSOR] Przełącznik pływakowy na GPIO%d\n", sc.water_level_pin);
  }
  if (sc.door_sensor_enabled) pinMode(sc.door_sensor_pin, INPUT_PULLUP);

  if (sc.bmp180_enabled) {
    Wire.beginTransmission(0x77);
    bool bmp_present = (Wire.endTransmission() == 0);
    if (bmp_present) {
      Serial.println("[SENSOR] BMP180 - inicjalizacja...");
      bmp180_ok = bmp180.begin();
      if (bmp180_ok) {
        Serial.printf("[SENSOR] ✓ BMP180 OK na GPIO%d (SDA), GPIO%d (SCL)\n", sc.bmp180_sda_pin, sc.bmp180_scl_pin);
      } else {
        Serial.println("[SENSOR] BMP180 wykryty, ale inicjalizacja nieudana.");
      }
    } else {
      bmp180_ok = false;
      Serial.println("[SENSOR] BMP180 pominięty (brak na I2C 0x77).");
    }
  } else {
    bmp180_ok = false;
  }
}

// ============================================================================
// ODCZYT CZUJNIKÓW
// ============================================================================
void sensors_read(SensorData& data) {
  SensorConfig& sc = configMgr.cfg.sensors;
  data.elapsed_ms = get_incubation_elapsed_ms();
  data.is_dummy = (sc.ds18b20_count == 0 && !sc.dht11_enabled);

  if (data.is_dummy) {
    float progress = min(1.0f, (float)data.elapsed_ms / 3600000.0f);
    data.temp[0] = 25.0f + progress * 12.5f;
    data.temp[1] = 25.0f + progress * 11.0f;
    data.humidity = 40.0f + progress * 20.0f;
    data.water_level = 2048;
    data.door_open = false;
    return;
  }

  bool any_bus = false;
  for (int i = 0; i < sc.ds18b20_count && i < MAX_SENSORS; i++) {
    if (ds_wire[i] && ds_sensor[i]) {
      any_bus = true;
      if (!ds18b20_conversion_pending) {
        ds_sensor[i]->requestTemperatures();
      }
    }
  }
  if (any_bus && !ds18b20_conversion_pending) {
    ds18b20_conversion_pending = true;
    ds18b20_request_ms = millis();
  }

  bool ds18b20_ready = true;
  bool ds18b20_used_last_data = false;
  if (ds18b20_conversion_pending) {
    unsigned long elapsed = millis() - ds18b20_request_ms;
    for (int i = 0; i < sc.ds18b20_count && i < MAX_SENSORS; i++) {
      if (ds_wire[i] && ds_sensor[i]) {
        if (!ds_sensor[i]->isConversionComplete()) {
          ds18b20_ready = false;
          break;
        }
      }
    }
    if (!ds18b20_ready && elapsed < DS18B20_CONVERSION_MS) {
      ds18b20_used_last_data = true;
      for (int i = 0; i < sc.ds18b20_count && i < MAX_SENSORS; i++) {
        data.temp[i] = (last_sensor.temp[i] > -90.0f) ? last_sensor.temp[i] : -99.0f;
      }
      for (int i = sc.ds18b20_count; i < MAX_SENSORS; i++) data.temp[i] = -99.0f;
    }
    if (ds18b20_ready) ds18b20_conversion_pending = false;
  }

  if (!ds18b20_used_last_data) {
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
        t = ds_sensor[bus]->getTempCByIndex(dev_idx);
      }
      if (t == DEVICE_DISCONNECTED_C || t < -50.0f || t > 125.0f) {
        data.temp[i] = -99.0f;
      } else {
        data.temp[i] = t;
      }
    }
    for (int i = sc.ds18b20_count; i < MAX_SENSORS; i++) data.temp[i] = -99.0f;
  }

  if (sc.dht11_enabled && dht_sensor) {
    float h = dht_sensor->readHumidity();
    if (!isnan(h) && h >= 0.0f && h <= 100.0f) {
      data.humidity = h;
      last_valid_humidity = h;
      last_valid_humidity_ms = millis();
      dht_retry_attempts = 0;
    } else {
      unsigned long now_ms = millis();
      if (dht_retry_attempts == 0) {
        dht_retry_start_ms = now_ms;
        dht_retry_attempts = 1;
      }
      if (dht_retry_attempts > 0 && dht_retry_attempts < 3 && now_ms - dht_retry_start_ms >= 120UL) {
        h = dht_sensor->readHumidity();
        dht_retry_start_ms = now_ms;
        if (!isnan(h) && h >= 0.0f && h <= 100.0f) {
          data.humidity = h;
          last_valid_humidity = h;
          last_valid_humidity_ms = now_ms;
          dht_retry_attempts = 0;
        } else {
          dht_retry_attempts++;
        }
      }
      if (isnan(h) || h < 0.0f || h > 100.0f) {
        unsigned long age_ms = now_ms - last_valid_humidity_ms;
        if (last_valid_humidity >= 0.0f && age_ms <= 120000UL) {
          data.humidity = last_valid_humidity;
        } else {
          data.humidity = -1.0f;
        }
      }
    }
  } else {
    data.humidity = -1.0f;
  }

  if (sc.water_level_enabled) {
    int switch_state = digitalRead(sc.water_level_pin);
    data.water_level = switch_state ? 0 : 4095;
  } else {
    data.water_level = -1;
  }
  data.door_open = sc.door_sensor_enabled ? (digitalRead(sc.door_sensor_pin) == LOW) : false;
  data.heater_pwm_pct = (int)heater_pwm_pct;
  data.heater_pwm_raw = (int)round(constrain(heater_pwm_pct, 0.0f, 100.0f) * 10.23f);
  data.diffuser_on = diffuser_on;
  data.diffuser_active_idx = diffuser_active_idx;

  if (sc.bmp180_enabled && bmp180_ok) {
    float pressure_hpa = bmp180.readPressure() / 100.0f;
    float bmp_temp = bmp180.readTemperature();
    if (isnan(bmp_temp) || pressure_hpa <= 0.0f) {
      data.pressure_hpa = -1.0f;
      data.bmp180_temp = -99.0f;
      bmp180_ok = false;
    } else {
      data.pressure_hpa = pressure_hpa;
      data.bmp180_temp = bmp_temp;
    }
  } else {
    data.pressure_hpa = -1.0f;
    data.bmp180_temp = -99.0f;
  }
}

// ============================================================================
// CSV LOG
// ============================================================================
#define LOG_ROTATE_SIZE (1024 * 1024)

void csv_create_header() {
  File f = LittleFS.open("/inkubator_log.csv", "w");
  if (f) {
    f.println("time_sec,temp_chamber,temp_secondary,humidity,water_level,door,heater_pct,heater_pwm_raw,diffuser_on,diffuser_idx");
    f.close();
  }
}

void csv_write(const SensorData& data) {
  if (LittleFS.exists("/inkubator_log.csv")) {
    File existing = LittleFS.open("/inkubator_log.csv", "r");
    if (existing && existing.size() >= LOG_ROTATE_SIZE) {
      existing.close();
      if (LittleFS.exists("/inkubator_log_old.csv")) {
        LittleFS.remove("/inkubator_log_old.csv");
      }
      LittleFS.rename("/inkubator_log.csv", "/inkubator_log_old.csv");
      csv_create_header();
    } else if (existing) {
      existing.close();
    }
  }

  File f = LittleFS.open("/inkubator_log.csv", "a");
  if (!f) return;
  char buf[256];
  snprintf(buf, sizeof(buf), "%llu,%.2f,%.2f,%.2f,%d,%d,%d,%d,%d,%d\n",
           data.elapsed_ms / 1000,
           data.temp[0], data.temp[1],
           data.humidity,
           data.water_level,
           data.door_open ? 1 : 0,
           data.heater_pwm_pct,
           data.heater_pwm_raw,
           data.diffuser_on ? 1 : 0,
           data.diffuser_active_idx);
  f.print(buf);
  f.close();
}

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
void webserver_setup();
void api_status();
void api_trays();
void api_tray_angle();
static void mpu_refresh_angle();
static void mpu_update_angle_if_rotating(unsigned long now);
void handle_serial_commands();

bool sanitize_legacy_usb_pin_conflicts() {
  bool changed = false;
  for (int i = 0; i < MAX_TRAYS; i++) {
    int pin = configMgr.cfg.trays[i].pin;
    if (pin == 19) {
      configMgr.cfg.trays[i].pin = 16;
      changed = true;
      Serial.printf("[CFG] Taca %d: GPIO19 (USB D-) -> GPIO16\n", i);
    } else if (pin == 20) {
      configMgr.cfg.trays[i].pin = 17;
      changed = true;
      Serial.printf("[CFG] Taca %d: GPIO20 (USB D+) -> GPIO17\n", i);
    }
  }
  return changed;
}

void handle_serial_commands() {
  static String serial_buffer;
  while (Serial.available()) {
    int c = Serial.read();
    if (c < 0) break;
    if (c == '\r' || c == '\n') {
      if (serial_buffer.length() == 0) continue;
      String cmd = serial_buffer;
      serial_buffer = "";
      cmd.trim();
      if (cmd.equalsIgnoreCase("MPU") || cmd.equalsIgnoreCase("MPU?") || cmd.equalsIgnoreCase("MPUSTATUS")) {
        Serial.printf("[MPU] i2c_found=%s sensor_ok=%s\n",
                      mpu_i2c_found ? "true" : "false",
                      mpu_sensor_ok ? "true" : "false");
        if (mpu_i2c_found && mpu_sensor_ok) {
          mpu_refresh_angle();
          unsigned long age = millis() - last_mpu_read_ms;
          Serial.printf("[MPU] angle=%.2f deg age=%lums\n", last_mpu_angle_deg, age);
        }
      } else if (cmd.equalsIgnoreCase("I2C")) {
        Serial.printf("[I2C] SDA=%d SCL=%d\n",
                      configMgr.cfg.sensors.i2c_sda_pin,
                      configMgr.cfg.sensors.i2c_scl_pin);
      } else {
        Serial.printf("[SERIAL] Nieznana komenda: %s\n", cmd.c_str());
      }
    } else {
      serial_buffer += (char)c;
      if (serial_buffer.length() > 128) {
        serial_buffer = serial_buffer.substring(serial_buffer.length() - 128);
      }
    }
  }
}

void wifi_keepalive() {
  // In AP-only mode, avoid forcing WiFi restart from loop.
  // Calling WiFi.softAPConfig()/WiFi.softAP() repeatedly here may crash the WiFi stack.
}

static String content_type_for_path(const String& path) {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".css")) return "text/css";
  if (path.endsWith(".js")) return "application/javascript";
  if (path.endsWith(".svg")) return "image/svg+xml";
  if (path.endsWith(".png")) return "image/png";
  if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  if (path.endsWith(".bmp")) return "image/bmp";
  return "text/plain";
}

static bool serve_littlefs_uri(const String& uri) {
  if (!LittleFS.exists(uri)) return false;
  File f = LittleFS.open(uri, "r");
  if (!f) return false;
  webServer.streamFile(f, content_type_for_path(uri));
  f.close();
  return true;
}

void api_root() {
  if (serve_littlefs_uri("/index.html")) {
    return;
  }
  webServer.send(404, "text/plain", "Brak pliku index.html w LittleFS");
}

void api_favicon() {
  webServer.send(204, "text/plain", "");
}

void api_docs_v11() {
  if (LittleFS.exists("/documentation_v1_1.html")) {
    File f = LittleFS.open("/documentation_v1_1.html", "r");
    webServer.streamFile(f, "text/html");
    f.close();
  } else {
    webServer.send(404, "text/plain", "Brak pliku documentation_v1_1.html w LittleFS");
  }
}

void api_status() {
  tray_update_all(get_incubation_elapsed_ms());

  float hum_min, hum_max;
  tray_get_chamber_humidity(hum_min, hum_max);

  float temp_chamber = last_sensor.temp[0];
  int aq_idx = configMgr.cfg.sensors.aquarium_sensor_index;
  float temp_aq = (aq_idx >= 0 && aq_idx < MAX_SENSORS) ? last_sensor.temp[aq_idx] : last_sensor.temp[1];

  bool aq_ok = (aq_idx >= 0) ? 
    (temp_aq >= 0 && fabs(temp_aq - configMgr.cfg.sensors.aquarium_target_temp) < 1.0f) : true;

  bool temp_ok = !last_sensor.is_dummy && fabs(temp_chamber - configMgr.cfg.system.target_temp) <= 0.6f;
  bool hum_ok = (last_sensor.humidity >= 0) && 
                (last_sensor.humidity >= hum_min - 5) && 
                (last_sensor.humidity <= hum_max + 5);

  String alert = "";
  if (last_sensor.is_dummy) alert += "TRYB TESTOWY – brak czujników. ";
  if (!temp_ok && !last_sensor.is_dummy) alert += "Temperatura poza normą! ";
  if (!hum_ok && last_sensor.humidity >= 0) alert += "Wilgotność poza normą! ";
  if (!aq_ok && aq_idx >= 0) alert += "Akwarium poza normą! ";
  if (last_sensor.door_open) alert += "DRZWI OTWARTE! ";

  JsonDocument doc;
  doc["status"] = (temp_ok && hum_ok && aq_ok) ? "OK" : "WARNING";
  doc["alert"] = alert;
  doc["temp_chamber"] = temp_chamber;
  doc["temp_aquarium"] = temp_aq;
  doc["aquarium_ok"] = aq_ok;
  doc["humidity"] = last_sensor.humidity;
  doc["humidity_target_min"] = hum_min;
  doc["humidity_target_max"] = hum_max;
  float hum_set;
  tray_get_chamber_humidity_target(hum_set);
  if (isfinite(hum_set)) doc["target_humidity"] = hum_set;
  doc["target_temp"] = configMgr.cfg.system.target_temp;
  doc["heater_profile"] = heater_profile_name;
  doc["learning_mode"] = learning_mode;
  doc["learning_profile"] = learning_profile;
  doc["mpu_i2c_found"] = mpu_i2c_found;
  doc["mpu_sensor_ok"] = mpu_sensor_ok;
  doc["heater_pwm"] = (int)heater_pwm_pct;
  doc["heater_pwm_raw"] = (int)round(constrain(heater_pwm_pct, 0.0f, 100.0f) * 10.23f);
  int heater_pwm_ch0 = heater_manual_mode[0]
    ? (int)round(constrain(heater_manual_pct[0], 0.0f, 100.0f) * 10.23f)
    : (int)round(constrain(heater_pwm_pct, 0.0f, 100.0f) * 10.23f);
  int heater_pwm_ch1 = heater_manual_mode[1]
    ? (int)round(constrain(heater_manual_pct[1], 0.0f, 100.0f) * 10.23f)
    : (int)round(constrain(heater_pwm_pct, 0.0f, 100.0f) * 10.23f);
  int heater_pwm_ch23 = (int)round(constrain(heater_pwm_pct, 0.0f, 100.0f) * 10.23f);
  doc["heater_pwm_ch0"] = heater_pwm_ch0;
  doc["heater_pwm_ch1"] = heater_pwm_ch1;
  doc["heater_pwm_ch2"] = heater_pwm_ch23;
  doc["heater_pwm_ch3"] = heater_pwm_ch23;
  doc["heater_pin_3"] = configMgr.cfg.system.heater_pin_3;
  doc["heater_pin_4"] = configMgr.cfg.system.heater_pin_4;
  doc["heater_manual_mode"] = heater_manual_mode[0] || heater_manual_mode[1];
  doc["heater_manual_ch0"] = heater_manual_mode[0];
  doc["heater_manual_ch1"] = heater_manual_mode[1];
  doc["heater_manual_pct"] = heater_manual_pct[0];
  doc["heater_manual_pct_ch0"] = heater_manual_pct[0];
  doc["heater_manual_pct_ch1"] = heater_manual_pct[1];
  doc["water_level"] = last_sensor.water_level;
  doc["door_open"] = last_sensor.door_open;
  doc["diffuser_on"] = diffuser_on;
  doc["test_mode"] = configMgr.cfg.system.test_mode;
  doc["config_version"] = configMgr.cfg.config_version;
  doc["wifi_connected"] = wifi_connected;
  doc["rtc_connected"] = rtc_ok;
  doc["uptime_ms"] = (unsigned long)(last_sensor.elapsed_ms);
  doc["pressure_hpa"] = last_sensor.pressure_hpa;
  doc["bmp180_temp"] = last_sensor.bmp180_temp;
  doc["esp_free_heap"] = ESP.getFreeHeap();
  doc["esp_total_heap"] = ESP.getHeapSize();
  doc["esp_free_psram"] = ESP.getFreePsram();
  doc["esp_cpu_freq_mhz"] = ESP.getCpuFreqMHz();
  doc["esp_flash_size"] = ESP.getFlashChipSize();
  doc["esp_chip_cores"] = ESP.getChipCores();
  doc["esp_internal_temp"] = temperatureRead();
  doc["diffuser_manual_mode"] = diffuser_manual_mode;
  doc["diffuser_manual_0"]    = diffuser_manual_on[0];
  doc["diffuser_manual_1"]    = diffuser_manual_on[1];
  doc["diffuser_active_idx"]  = diffuser_active_idx;
  doc["diffuser_0_on"] = diffuser_on && (diffuser_active_idx == 0);
  doc["diffuser_1_on"] = diffuser_on && (diffuser_active_idx == 1);
  doc["diffuser_profile"] = configMgr.cfg.diffusers.profile;
  doc["learned_pwm_pct"] = configMgr.learning_state.learned_pwm_pct;
  doc["learned_target_temp"] = configMgr.learning_state.learned_target_temp;
  doc["learned_target_humidity"] = configMgr.learning_state.learned_target_humidity;
  doc["learned_diffuser_on_ms"] = configMgr.learning_state.diffuser_on_ms;
  doc["learned_diffuser_off_ms"] = configMgr.learning_state.diffuser_off_ms;
  doc["learning_record_active"] = is_learning_record_active();
  doc["learning_record_elapsed_sec"] = learning_record_elapsed_sec();
  doc["learning_record_samples"] = learning_record_samples_count();

  TrayState& main_tray = tray_states[0];
  doc["current_day"] = main_tray.current_day;
  doc["days_remaining"] = main_tray.days_remaining;

  String output;
  serializeJson(doc, output);
  webServer.send(200, "application/json", output);
}

void api_trays() {
  String json = tray_status_json(get_incubation_elapsed_ms());
  webServer.send(200, "application/json", json);
}

void api_tray_set_day() {
  if (!webServer.hasArg("tray") || !webServer.hasArg("day")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametrów tray/day\"}");
    return;
  }
  int tray = webServer.arg("tray").toInt();
  int day = webServer.arg("day").toInt();

  if (tray < 0 || tray >= MAX_TRAYS || day < 1 || day > 30) {
    webServer.send(400, "application/json", "{\"error\":\"Nieprawidłowe wartości\"}");
    return;
  }

  if (tray_set_day(tray, day, get_incubation_elapsed_ms())) {
    webServer.send(200, "application/json", "{\"ok\":true}");
  } else {
    webServer.send(400, "application/json", "{\"error\":\"Nie można ustawić dnia\"}");
  }
}

void api_tray_set_start() {
  if (!webServer.hasArg("tray") || !webServer.hasArg("date") || !webServer.hasArg("time")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametrów tray/date/time\"}");
    return;
  }
  int tray = webServer.arg("tray").toInt();
  String date = webServer.arg("date");
  String time = webServer.arg("time");

  if (tray < 0 || tray >= MAX_TRAYS) {
    webServer.send(400, "application/json", "{\"error\":\"Nieprawidłowy numer tacy\"}");
    return;
  }
  if (date.length() != 10 || time.length() != 5) {
    webServer.send(400, "application/json", "{\"error\":\"Nieprawidłowy format daty lub godziny\"}");
    return;
  }

  int year = date.substring(0,4).toInt();
  int month = date.substring(5,7).toInt();
  int day = date.substring(8,10).toInt();
  int hour = time.substring(0,2).toInt();
  int minute = time.substring(3,5).toInt();

  if (year < 2020 || month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 || minute > 59) {
    webServer.send(400, "application/json", "{\"error\":\"Nieprawidłowa data lub godzina\"}");
    return;
  }

  // Convert to Unix timestamp. This assumes RTC is available and set.
  if (!rtc_ok) {
    webServer.send(400, "application/json", "{\"error\":\"Brak RTC - nie można ustawić daty startu\"}");
    return;
  }

  DateTime dt(year, month, day, hour, minute, 0);
  unsigned long long unix_ts = dt.unixtime();

  configMgr.cfg.trays[tray].start_unix = unix_ts;
  if (tray >= 0 && tray < MAX_TRAYS) {
    tray_states[tray].start_millis = 0ULL;
    tray_update_state(tray_states[tray], get_incubation_elapsed_ms());
  }
  configMgr.save();
  webServer.send(200, "application/json", "{\"ok\":true,\"start_unix\":" + String(unix_ts) + "}");
}

void api_tray_set_species() {
  if (!webServer.hasArg("tray") || !webServer.hasArg("species")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametrów\"}");
    return;
  }
  int tray = webServer.arg("tray").toInt();
  String species = webServer.arg("species");
  tray_set_species(tray, species.c_str());
  webServer.send(200, "application/json", "{\"ok\":true}");
}

void api_tray_set_active() {
  if (!webServer.hasArg("tray") || !webServer.hasArg("active")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametrów\"}");
    return;
  }
  int tray = webServer.arg("tray").toInt();
  bool active = (webServer.arg("active") == "true");
  tray_set_active(tray, active);
  webServer.send(200, "application/json", "{\"ok\":true}");
}

void api_tray_move() {
  int tray = webServer.hasArg("tray") ? webServer.arg("tray").toInt() : 0;
  unsigned long duration_ms = webServer.hasArg("duration_ms") 
                              ? (unsigned long)webServer.arg("duration_ms").toInt() 
                              : 15000UL;
  int pwm_speed = webServer.hasArg("pwm_speed") 
                  ? webServer.arg("pwm_speed").toInt() 
                  : configMgr.cfg.trays[tray].rotation_pwm_speed;

  if (tray < 0 || tray >= MAX_TRAYS) {
    webServer.send(400, "application/json", "{\"error\":\"Nieprawidłowy numer tacy\"}");
    return;
  }
  if (duration_ms > 600000) {
    webServer.send(400, "application/json", "{\"error\":\"Czas poza zakresem 0-600000 ms\"}");
    return;
  }
  if (pwm_speed < 0 || pwm_speed > 255) {
    webServer.send(400, "application/json", "{\"error\":\"PWM poza zakresem 0-255\"}");
    return;
  }

  if (duration_ms == 0) {
    tray_stop(tray, get_incubation_elapsed_ms());
    webServer.send(200, "application/json", "{\"ok\":true,\"tray\":" + String(tray) + ",\"stopped\":true}");
    return;
  }

  if (!tray_states[tray].active) {
    webServer.send(400, "application/json", "{\"error\":\"Taca nieaktywna\"}");
    return;
  }

  int old_pwm = configMgr.cfg.trays[tray].rotation_pwm_speed;
  configMgr.cfg.trays[tray].rotation_pwm_speed = pwm_speed;
  tray_manual_move(tray, duration_ms, get_incubation_elapsed_ms());
  configMgr.cfg.trays[tray].rotation_pwm_speed = old_pwm;
  
  webServer.send(200, "application/json", 
    "{\"ok\":true,\"tray\":" + String(tray) + ",\"duration_ms\":" + String(duration_ms) + ",\"pwm_speed\":" + String(pwm_speed) + "}");
}

void api_tray_stop() {
  if (!webServer.hasArg("tray")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametru tray\"}");
    return;
  }
  int tray = webServer.arg("tray").toInt();
  if (tray < 0 || tray >= MAX_TRAYS) {
    webServer.send(400, "application/json", "{\"error\":\"Nieprawidłowy numer tacy\"}");
    return;
  }
  tray_stop(tray, get_incubation_elapsed_ms());
  webServer.send(200, "application/json", "{\"ok\":true,\"tray\":" + String(tray) + ",\"stopped\":true}");
}

void api_tray_angle() {
  if (!webServer.hasArg("tray")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametru tray\"}");
    return;
  }

  int tray = webServer.arg("tray").toInt();
  if (tray < 0 || tray >= MAX_TRAYS) {
    webServer.send(400, "application/json", "{\"error\":\"Nieprawidłowy numer tacy\"}");
    return;
  }

  DynamicJsonDocument doc(256);
  doc["tray"] = tray;
  bool has_sensor = configMgr.cfg.trays[tray].has_angle_sensor;
  doc["has_sensor"] = has_sensor;
  doc["i2c_found"] = mpu_i2c_found;
  if (!has_sensor) {
    String output;
    serializeJson(doc, output);
    webServer.send(200, "application/json", output);
    return;
  }

  if (!mpu_sensor_ok) {
    doc["sensor_ok"] = false;
    doc["error"] = "MPU6050 niedostępny";
    String output;
    serializeJson(doc, output);
    webServer.send(503, "application/json", output);
    return;
  }

  mpu_refresh_angle();
  unsigned long now = millis();
  unsigned long age_ms = (last_mpu_read_ms == 0) ? 0 : (now - last_mpu_read_ms);

  doc["sensor_ok"] = true;
  if (isnan(last_mpu_angle_deg)) {
    doc["angle_deg"] = nullptr;
  } else {
    doc["angle_deg"] = last_mpu_angle_deg;
  }
  doc["age_ms"] = age_ms;

  String output;
  serializeJson(doc, output);
  webServer.send(200, "application/json", output);
}

static void mpu_refresh_angle() {
  if (!mpu_sensor_ok) return;
  mpu.update();
  float angle_x = mpu.getAngleX();
  float angle_y = mpu.getAngleY();
  last_mpu_angle_deg = sqrt(angle_x * angle_x + angle_y * angle_y);
  last_mpu_read_ms = millis();
}

static void mpu_update_angle_if_rotating(unsigned long now) {
  if (!mpu_sensor_ok) return;
  if (now - last_mpu_read_ms < 250) return;

  bool motion_active = false;
  for (int i = 0; i < MAX_TRAYS; i++) {
    if (!configMgr.cfg.trays[i].has_angle_sensor) continue;
    if (tray_states[i].currently_rotating || tray_states[i].manual_move_until_ms > now) {
      motion_active = true;
      break;
    }
  }
  if (!motion_active) return;

  mpu_refresh_angle();
}

void api_config_get() {
  String json = configMgr.to_json();
  webServer.send(200, "application/json", json);
}

void api_config_set() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak body\"}");
    return;
  }
  String body = webServer.arg("plain");
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    webServer.send(400, "application/json", "{\"error\":\"Nieprawidłowy JSON\"}");
    return;
  }
  if (doc["factory_reset"].is<bool>() && doc["factory_reset"].as<bool>()) {
    configMgr.factory_reset();
    sensors_init();
    tray_init(system_start_ms);
    webServer.send(200, "application/json", "{\"ok\":true,\"message\":\"Factory reset wykonany\"}");
    return;
  }
  String json;
  serializeJson(doc, json);
  if (configMgr.from_json(json)) {
    configMgr.save();
    sensors_init();
    tray_init(system_start_ms);
    webServer.send(200, "application/json", "{\"ok\":true,\"message\":\"Konfiguracja zapisana\"}");
  } else {
    webServer.send(400, "application/json", "{\"error\":\"Nieprawidłowy JSON\"}");
  }
}

void api_species() {
  String output = serialize_species_profiles();
  if (output != "[]") {
    webServer.send(200, "application/json", output);
    return;
  }

  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < SPECIES_COUNT; i++) {
    JsonObject obj = arr.add<JsonObject>();
    obj["id"] = SPECIES_LIST[i].id;
    obj["name"] = SPECIES_LIST[i].name;
    obj["days"] = SPECIES_LIST[i].total_days;
  }
  serializeJson(arr, output);
  webServer.send(200, "application/json", output);
}

void api_species_active() {
  String output = api_species_profiles_active();
  webServer.send(200, "application/json", output);
}

static bool trySendScheduleFromProcessProfiles(const String& sp_id) {
  if (!LittleFS.exists("/process_profiles.json")) return false;
  File f = LittleFS.open("/process_profiles.json", "r");
  if (!f) return false;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;
  JsonArray speciesArr = doc["species"].as<JsonArray>();
  if (!speciesArr) return false;
  String target = sp_id;
  target.trim();
  target.toLowerCase();
  JsonVariant profile;
  for (JsonVariant item : speciesArr) {
    const char* id = item["id"];
    if (!id) continue;
    String candidate = id;
    candidate.toLowerCase();
    if (candidate == target) {
      profile = item;
      break;
    }
  }
  if (profile.isNull()) return false;
  const char* category = profile["category"];
  if (!category || strcmp(category, "egg") != 0) return false;
  double totalHours = profile["total_hours"] | 0.0;
  int totalDays = totalHours > 0.0 ? (int)ceil(totalHours / 24.0) : 0;
  JsonDocument outDoc;
  outDoc["species"] = profile["id"].as<const char*>();
  outDoc["name"] = profile["name"].as<const char*>();
  outDoc["total_days"] = totalDays;
  JsonArray sched = outDoc["schedule"].to<JsonArray>();
  JsonArray phases = profile["phases"].as<JsonArray>();
  for (JsonVariant phase : phases) {
    double hourStart = phase["hour_start"] | 0.0;
    double hourEnd = phase["hour_end"] | 0.0;
    int startDay = (int)floor(hourStart / 24.0) + 1;
    int endDay = max(startDay, (int)ceil(hourEnd / 24.0));
    for (int day = startDay; day <= endDay; day++) {
      JsonObject dayObj = sched.add<JsonObject>();
      dayObj["day"] = day;
      dayObj["temp_min"] = phase["temp_min"] | 0.0;
      dayObj["temp_max"] = phase["temp_max"] | 0.0;
      dayObj["target_temp"] = phase["temp_ideal"] | 0.0;
      dayObj["humidity_min"] = phase["humidity_min"] | 0.0;
      dayObj["humidity_max"] = phase["humidity_max"] | 0.0;
      dayObj["turn"] = phase["turning"]["enabled"] | false;
      dayObj["lockdown"] = !phase["turning"]["enabled"];
      dayObj["notes"] = phase["notes"].as<const char*>() ?: "";
    }
  }
  String output;
  serializeJson(outDoc, output);
  webServer.send(200, "application/json", output);
  return true;
}

static bool trySendScheduleFromSpeciesProfiles(const String& sp_id) {
  JsonVariant profile = find_species_profile(sp_id);
  if (profile.isNull()) return false;
  const char* category = profile["category"];
  if (!category || strcmp(category, "egg") != 0) return false;

  double totalHours = profile["total_hours"] | 0.0;
  int totalDays = totalHours > 0.0 ? (int)ceil(totalHours / 24.0) : 0;

  JsonDocument outDoc;
  outDoc["species"] = profile["id"].as<const char*>();
  outDoc["name"] = profile["name"].as<const char*>();
  outDoc["total_days"] = totalDays;
  JsonArray sched = outDoc["schedule"].to<JsonArray>();
  JsonArray phases = profile["phases"].as<JsonArray>();
  for (JsonVariant phase : phases) {
    double hourStart = phase["hour_start"] | 0.0;
    double hourEnd = phase["hour_end"] | 0.0;
    int startDay = (int)floor(hourStart / 24.0) + 1;
    int endDay = max(startDay, (int)ceil(hourEnd / 24.0));
    for (int day = startDay; day <= endDay; day++) {
      JsonObject dayObj = sched.add<JsonObject>();
      dayObj["day"] = day;
      dayObj["temp_min"] = phase["temp_min"] | 0.0;
      dayObj["temp_max"] = phase["temp_max"] | 0.0;
      dayObj["target_temp"] = phase["temp_ideal"] | 0.0;
      dayObj["humidity_min"] = phase["humidity_min"] | 0.0;
      dayObj["humidity_max"] = phase["humidity_max"] | 0.0;
      dayObj["turn"] = phase["turning"]["enabled"] | false;
      dayObj["lockdown"] = !phase["turning"]["enabled"];
      dayObj["notes"] = phase["notes"].as<const char*>() ?: "";
    }
  }
  String output;
  serializeJson(outDoc, output);
  webServer.send(200, "application/json", output);
  return true;
}

void api_schedule() {
  String sp_id = webServer.hasArg("species") ? webServer.arg("species") : "CHICKEN";
  if (trySendScheduleFromSpeciesProfiles(sp_id)) return;
  if (trySendScheduleFromProcessProfiles(sp_id)) return;
  const SpeciesInfo* sp = get_species(sp_id.c_str());
  if (!sp) {
    webServer.send(404, "application/json", "{\"error\":\"Nieznany gatunek\"}");
    return;
  }
  JsonDocument doc;
  doc["species"] = sp->id;
  doc["name"] = sp->name;
  doc["total_days"] = sp->total_days;
  JsonArray sched = doc["schedule"].to<JsonArray>();
  for (int i = 0; i < sp->total_days; i++) {
    const IncubationDay& d = sp->schedule[i];
    JsonObject dayObj = sched.add<JsonObject>();
    dayObj["day"] = d.day;
    dayObj["temp_min"] = d.temp_min;
    dayObj["temp_max"] = d.temp_max;
    dayObj["target_temp"] = d.temp_set;
    dayObj["humidity_min"] = d.humidity_min;
    dayObj["humidity_max"] = d.humidity_max;
    dayObj["turn"] = d.turn;
    dayObj["lockdown"] = d.lockdown;
    dayObj["target_humidity"] = d.humidity_set;
    dayObj["notes"] = d.notes;
  }
  String output;
  serializeJson(doc, output);
  webServer.send(200, "application/json", output);
}

void api_history() {
  JsonDocument doc;
  JsonArray dataArr = doc["history"].to<JsonArray>();
  for (int i = 0; i < HISTORY_SIZE; i++) {
    int idx = (history_idx + i) % HISTORY_SIZE;
    if (history[idx].t_min == 0) continue;
    JsonObject p = dataArr.add<JsonObject>();
    p["timestamp_min"] = (unsigned long)history[idx].t_min;
    p["temp_chamber"] = history[idx].temp0;
    p["temp_aquarium"] = history[idx].temp1;
    p["humidity"] = history[idx].humidity;
    p["heater_pct"] = history[idx].heater_pct;
    p["heater_pwm_raw"] = history[idx].heater_pwm_raw;
    p["diffuser_on"] = history[idx].diffuser_on;
    p["diffuser_active_idx"] = history[idx].diffuser_active_idx;
  }
  String output;
  serializeJson(doc, output);
  webServer.send(200, "application/json", output);
}

void api_history_csv() {
  String csv = "timestamp_min,temp_chamber,temp_aquarium,humidity,heater_pct,heater_pwm_raw,diffuser_on,diffuser_active_idx\n";
  for (int i = 0; i < HISTORY_SIZE; i++) {
    int idx = (history_idx + i) % HISTORY_SIZE;
    if (history[idx].t_min == 0) continue;
    csv += String((unsigned long)history[idx].t_min) + "," +
           String(history[idx].temp0, 2) + "," +
           String(history[idx].temp1, 2) + "," +
           String(history[idx].humidity, 2) + "," +
           String((int)history[idx].heater_pct) + "," +
           String(history[idx].heater_pwm_raw) + "," +
           String(history[idx].diffuser_on ? 1 : 0) + "," +
           String(history[idx].diffuser_active_idx) + "\n";
  }
  webServer.sendHeader("Content-Disposition", "attachment; filename=history.csv");
  webServer.send(200, "text/csv", csv);
}

void api_journal_download() {
  if (!LittleFS.exists(JOURNAL_PATH)) {
    webServer.send(404, "text/plain", "Brak pliku dziennika");
    return;
  }
  File f = LittleFS.open(JOURNAL_PATH, "r");
  if (!f) {
    webServer.send(500, "text/plain", "Nie można otworzyć dziennika");
    return;
  }
  webServer.sendHeader("Content-Disposition", "attachment; filename=journal.json");
  webServer.streamFile(f, "application/json");
  f.close();
}

void api_process_profiles() {
  if (!LittleFS.exists("/process_profiles.json")) {
    webServer.send(404, "application/json", "{\"error\":\"Brak pliku process_profiles.json\"}");
    return;
  }
  File f = LittleFS.open("/process_profiles.json", "r");
  if (!f) {
    webServer.send(500, "application/json", "{\"error\":\"Nie można otworzyć process_profiles.json\"}");
    return;
  }
  String content;
  while (f.available()) {
    content += (char)f.read();
  }
  f.close();
  webServer.send(200, "application/json", content);
}

bool journal_save_to_littlefs(const String& json) {
  File f = LittleFS.open(JOURNAL_TMP_PATH, "w");
  if (!f) {
    return false;
  }
  if (f.print(json) < 0) {
    f.close();
    LittleFS.remove(JOURNAL_TMP_PATH);
    return false;
  }
  f.close();

  if (LittleFS.exists(JOURNAL_PATH) && !LittleFS.remove(JOURNAL_PATH)) {
    LittleFS.remove(JOURNAL_TMP_PATH);
    return false;
  }
  if (!LittleFS.rename(JOURNAL_TMP_PATH, JOURNAL_PATH)) {
    LittleFS.remove(JOURNAL_TMP_PATH);
    return false;
  }
  return true;
}

void api_log() {
  if (LittleFS.exists("/inkubator_log.csv")) {
    File f = LittleFS.open("/inkubator_log.csv", "r");
    webServer.sendHeader("Content-Disposition", "attachment; filename=inkubator_log.csv");
    webServer.streamFile(f, "text/csv");
    f.close();
  } else {
    webServer.send(404, "text/plain", "Brak pliku logu");
  }
}

void api_journal_get() {
  if (!LittleFS.exists(JOURNAL_PATH)) {
    webServer.send(200, "application/json", "{\"version\":1,\"batches\":[]}");
    return;
  }

  File f = LittleFS.open(JOURNAL_PATH, "r");
  if (!f) {
    webServer.send(500, "application/json", "{\"error\":\"Nie można otworzyć pliku dziennika\"}");
    return;
  }

  String json = f.readString();
  f.close();
  webServer.send(200, "application/json", json);
}

void api_journal_set() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak body\"}");
    return;
  }
  String body = webServer.arg("plain");
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    webServer.send(400, "application/json", "{\"error\":\"Nieprawidłowy JSON\"}");
    return;
  }

  if (!doc["batches"].is<JsonArray>()) {
    webServer.send(400, "application/json", "{\"error\":\"Brak pola batches\"}");
    return;
  }

  String output;
  serializeJson(doc, output);
  if (!journal_save_to_littlefs(output)) {
    webServer.send(500, "application/json", "{\"error\":\"Nie udało się zapisać dziennika\"}");
    return;
  }

  webServer.send(200, "application/json", "{\"ok\":true}");
}

void api_journal_clear() {
  if (LittleFS.exists(JOURNAL_PATH) && !LittleFS.remove(JOURNAL_PATH)) {
    webServer.send(500, "application/json", "{\"error\":\"Nie udało się usunąć dziennika\"}");
    return;
  }
  webServer.send(200, "application/json", "{\"ok\":true}");
}

void api_reset() {
  if (rtc_ok) {
    incubation_start_unix = rtc.now().unixtime();
    save_start_time(incubation_start_unix, START_MODE_UNIX);
    Serial.printf("[RTC] Reset inkubacji: start_unix=%llu\n", incubation_start_unix);
  } else {
    system_start_ms = millis();
    incubation_elapsed_base_ms = 0;
    save_start_time(incubation_elapsed_base_ms, START_MODE_MILLIS);
    Serial.println("[RTC] Reset inkubacji fallback millis: elapsed=0");
  }
  tray_init(0);
  clear_rotation_state();
  memset(history, 0, sizeof(history));
  history_idx = 0;
  csv_create_header();
  webServer.send(200, "application/json", "{\"ok\":true}");
}

void api_reboot() {
  webServer.send(200, "application/json", "{\"ok\":true}");
  delay(800);
  ESP.restart();
}

void api_heater_set() {
  int ch = webServer.hasArg("ch") ? webServer.arg("ch").toInt() : -1;
  if (webServer.hasArg("auto")) {
    if (ch >= 0 && ch <= 1) heater_manual_mode[ch] = false;
    else { heater_manual_mode[0] = false; heater_manual_mode[1] = false; }
    webServer.send(200, "application/json", "{\"ok\":true,\"mode\":\"auto\"}");
    return;
  }
  if (!webServer.hasArg("pct")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametru pct\"}");
    return;
  }
  float pct = webServer.arg("pct").toFloat();
  if (pct < 0.0f || pct > 100.0f) {
    webServer.send(400, "application/json", "{\"error\":\"PWM poza zakresem 0-100\"}");
    return;
  }
  if (ch >= 0 && ch <= 1) {
    heater_manual_mode[ch] = true;
    heater_manual_pct[ch] = pct;
  } else {
    heater_manual_mode[0] = true;
    heater_manual_mode[1] = true;
    heater_manual_pct[0] = pct;
    heater_manual_pct[1] = pct;
  }
  webServer.send(200, "application/json", "{\"ok\":true,\"mode\":\"manual\",\"pct\":" + String(pct, 1) + "}");
}

void api_heater_profile_set() {
  if (!webServer.hasArg("mode")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametru mode\"}");
    return;
  }

  String mode = webServer.arg("mode");
  if (!set_heater_profile(mode)) {
    webServer.send(400, "application/json", "{\"error\":\"Niepoprawny tryb. Użyj fast, balanced lub eco\"}");
    return;
  }

  if (learning_mode) {
    learning_profile = heater_profile_name;
  }

  String response = "{\"ok\":true,\"profile\":\"" + heater_profile_name + "\"}";
  webServer.send(200, "application/json", response);
}

void api_heater_learning_set() {
  if (!webServer.hasArg("mode")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametru mode\"}");
    return;
  }

  String mode = webServer.arg("mode");
  if (mode == "on") {
    learning_mode = true;
    if (webServer.hasArg("profile")) {
      String profile = webServer.arg("profile");
      if (profile.length() > 0 && set_heater_profile(profile)) {
        learning_profile = heater_profile_name;
      }
    } else {
      learning_profile = heater_profile_name;
    }
    configMgr.save_learning_data();
    String response = "{\"ok\":true,\"learning_mode\":true,\"profile\":\"" + learning_profile + "\"}";
    webServer.send(200, "application/json", response);
    return;
  }

  if (mode == "off") {
    learning_mode = false;
    learning_profile = "";
    configMgr.save_learning_data();
    webServer.send(200, "application/json", "{\"ok\":true,\"learning_mode\":false}");
    return;
  }

  if (mode == "start_record") {
    unsigned long duration_sec = webServer.hasArg("duration_sec") ? (unsigned long)webServer.arg("duration_sec").toInt() : 3600UL;
    if (duration_sec == 0) duration_sec = 3600UL;
    start_learning_record(duration_sec);
    String response = "{\"ok\":true,\"mode\":\"record_started\",\"duration_sec\":" + String(duration_sec) + ",\"learning_record_active\":true}";
    webServer.send(200, "application/json", response);
    return;
  }

  if (mode == "stop_record") {
    stop_learning_record();
    String response = "{\"ok\":true,\"mode\":\"record_stopped\",\"learning_record_active\":false}";
    webServer.send(200, "application/json", response);
    return;
  }

  if (mode == "status") {
    String response = "{\"ok\":true,\"learning_mode\":" + String(learning_mode ? "true" : "false") + ",\"learning_profile\":\"" + learning_profile + "\",\"learning_record_active\":" + String(is_learning_record_active() ? "true" : "false") + ",\"learning_record_elapsed_sec\":" + String(learning_record_elapsed_sec()) + ",\"learning_record_samples\":" + String(learning_record_samples_count()) + "}";
    webServer.send(200, "application/json", response);
    return;
  }

  webServer.send(400, "application/json", "{\"error\":\"Niepoprawny tryb. Użyj on, off, start_record, stop_record lub status\"}");
}

void api_heater_learning_reset() {
  clear_learning_state();
  configMgr.clear_learning_data();
  learning_mode = false;
  learning_profile = "";
  webServer.send(200, "application/json", "{\"ok\":true,\"learning_mode\":false,\"message\":\"Learning data cleared\"}");
}

void api_diffuser_profile_set() {
  if (!webServer.hasArg("mode")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametru mode\"}");
    return;
  }

  String mode = webServer.arg("mode");
  if (!set_diffuser_profile(mode)) {
    webServer.send(400, "application/json", "{\"error\":\"Niepoprawny tryb. Użyj fast, balanced lub eco\"}");
    return;
  }

  if (configMgr.save()) {
    String response = "{\"ok\":true,\"profile\":\"" + String(configMgr.cfg.diffusers.profile) + "\"}";
    webServer.send(200, "application/json", response);
  } else {
    webServer.send(500, "application/json", "{\"error\":\"Nie udało się zapisać konfiguracji\"}");
  }
}

void api_diffuser_set() {
  if (webServer.hasArg("auto")) {
    diffuser_manual_mode = false;
    diffuser_change_ms = millis();
    diffuser_on = false;
    diffusers_control(last_sensor);
    webServer.send(200, "application/json", "{\"ok\":true,\"mode\":\"auto\"}");
    return;
  }
  if (!webServer.hasArg("on")) {
    webServer.send(400, "application/json", "{\"error\":\"Brak parametru on\"}");
    return;
  }
  bool on = (webServer.arg("on").toInt() != 0);
  diffuser_manual_mode = true;
  if (webServer.hasArg("idx")) {
    int idx = webServer.arg("idx").toInt();
    if (idx >= 0 && idx < 2) diffuser_manual_on[idx] = on;
    diffusers_control(last_sensor);
    webServer.send(200, "application/json",
      "{\"ok\":true,\"mode\":\"manual\",\"idx\":" + String(idx) +
      ",\"on\":" + String(on ? "true" : "false") + "}");
  } else {
    diffuser_manual_on[0] = on;
    diffuser_manual_on[1] = on;
    diffusers_control(last_sensor);
    webServer.send(200, "application/json",
      "{\"ok\":true,\"mode\":\"manual\",\"on\":" + String(on ? "true" : "false") + "}");
  }
}

// ============================================================================
// WEB SERVER SETUP
// ============================================================================
bool isValidUploadPath(const String& path) {
  if (path.length() == 0) return false;
  if (path.indexOf("..") >= 0) return false;
  if (path.indexOf('\\') >= 0) return false;
  return true;
}

String normalizeUploadPath(String path) {
  while (path.startsWith("/")) path = path.substring(1);
  path.replace('\\', '/');
  return path;
}

void api_upload_page() {
  if (!webServer.authenticate(OTA_USER, OTA_PASSWORD)) {
    return webServer.requestAuthentication();
  }

  const char* html = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Inkubator: LittleFS upload</title></head><body><h1>Upload do LittleFS</h1><form method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\"><div><label>Ścieżka w LittleFS: <input type=\"text\" name=\"path\" value=\"index.html\" style=\"width:400px\"></label></div><div><label>Plik: <input type=\"file\" name=\"file\"></label></div><div><button type=\"submit\">Wyślij</button></div></form><p>Użyj tego formularza, aby nadpisać plik w LittleFS.</p></body></html>";
  webServer.send(200, "text/html", html);
}

void handleFileUpload() {
  HTTPUpload& upload = webServer.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (!webServer.authenticate(OTA_USER, OTA_PASSWORD)) {
      return webServer.requestAuthentication();
    }
    String path = normalizeUploadPath(webServer.arg("path"));
    if (path.length() == 0) {
      path = normalizeUploadPath(upload.filename);
    }
    if (!isValidUploadPath(path)) {
      Serial.println("[UPLOAD] invalid path");
      uploadPath = "";
      return;
    }
    uploadPath = "/" + path;
    if (uploadFile) {
      uploadFile.close();
    }
    uploadFile = LittleFS.open(uploadPath, "w");
    if (!uploadFile) {
      Serial.printf("[UPLOAD] cannot open %s\n", uploadPath.c_str());
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
      Serial.printf("[UPLOAD] saved %s\n", uploadPath.c_str());
      uploadResultPath = uploadPath;
      uploadPath = "";
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    if (uploadFile) {
      uploadFile.close();
      if (uploadPath.length()) LittleFS.remove(uploadPath);
      uploadPath = "";
    }
  }
}

void api_upload_response() {
  if (!webServer.authenticate(OTA_USER, OTA_PASSWORD)) {
    return webServer.requestAuthentication();
  }
  String msg = "<html><body><h1>Upload zakończony</h1>";
  if (uploadResultPath.length()) {
    msg += "<p>Plik zapisano jako: ";
    msg += uploadResultPath;
    msg += "</p>";
    uploadResultPath = "";
  } else {
    msg += "<p>Brak pliku lub błąd podczas uploadu.</p>";
  }
  msg += "<p><a href=\"/upload\">Wróć</a></p></body></html>";
  webServer.send(200, "text/html", msg);
}

void api_lfs_upload() {
  if (!webServer.authenticate(OTA_USER, OTA_PASSWORD)) {
    return webServer.requestAuthentication();
  }
  String path = normalizeUploadPath(webServer.arg("path"));
  if (!isValidUploadPath(path)) {
    webServer.send(400, "application/json", "{\"ok\":false,\"error\":\"Nieprawidłowa ścieżka\"}");
    return;
  }
  String body = webServer.arg("plain");
  if (body.length() == 0) {
    webServer.send(400, "application/json", "{\"ok\":false,\"error\":\"Brak treści do zapisu\"}");
    return;
  }
  File f = LittleFS.open("/" + path, "w");
  if (!f) {
    webServer.send(500, "application/json", "{\"ok\":false,\"error\":\"Nie można otworzyć pliku\"}");
    return;
  }
  f.print(body);
  f.close();
  webServer.send(200, "application/json", "{\"ok\":true,\"path\":\"" + path + "\"}");
}

void webserver_setup() {
  webServer.on("/", HTTP_GET, api_root);
  webServer.on("/index.html", HTTP_GET, api_root);
  webServer.on("/favicon.ico", HTTP_GET, api_favicon);
  webServer.on("/docs-v1-1", HTTP_GET, api_docs_v11);
  webServer.on("/documentation_v1_1.html", HTTP_GET, api_docs_v11);
  webServer.on("/api/status", HTTP_GET, api_status);
  webServer.on("/api/trays", HTTP_GET, api_trays);
  webServer.on("/api/tray/set-day", HTTP_POST, api_tray_set_day);
  webServer.on("/api/tray/set-start", HTTP_POST, api_tray_set_start);
  webServer.on("/api/tray/set-species", HTTP_POST, api_tray_set_species);
  webServer.on("/api/tray/set-active", HTTP_POST, api_tray_set_active);
  webServer.on("/api/tray/move", HTTP_POST, api_tray_move);
  webServer.on("/api/tray/stop", HTTP_POST, api_tray_stop);
  webServer.on("/api/tray/angle", HTTP_GET, api_tray_angle);
  webServer.on("/api/config", HTTP_GET, api_config_get);
  webServer.on("/api/config", HTTP_POST, api_config_set);
  webServer.on("/process_profiles.json", HTTP_GET, api_process_profiles);
  webServer.on("/api/species", HTTP_GET, api_species);
  webServer.on("/api/species/active", HTTP_GET, api_species_active);
  webServer.on("/api/schedule", HTTP_GET, api_schedule);
  webServer.on("/api/history", HTTP_GET, api_history);
  webServer.on("/api/history/csv", HTTP_GET, api_history_csv);
  webServer.on("/api/log", HTTP_GET, api_log);
  webServer.on("/api/journal", HTTP_GET, api_journal_get);
  webServer.on("/api/journal/download", HTTP_GET, api_journal_download);
  webServer.on("/api/journal", HTTP_POST, api_journal_set);
  webServer.on("/api/journal/clear", HTTP_POST, api_journal_clear);
  webServer.on("/api/reset", HTTP_POST, api_reset);
  webServer.on("/api/reboot", HTTP_POST, api_reboot);
  webServer.on("/api/heater/set", HTTP_POST, api_heater_set);
  webServer.on("/api/heater/profile", HTTP_GET, api_heater_profile_set);
  webServer.on("/api/heater/learning", HTTP_GET, api_heater_learning_set);
  webServer.on("/api/heater/learning/reset", HTTP_POST, api_heater_learning_reset);
  webServer.on("/api/diffuser/set", HTTP_POST, api_diffuser_set);
  webServer.on("/api/diffuser/profile", HTTP_GET, api_diffuser_profile_set);

  webServer.on("/upload", HTTP_GET, api_upload_page);
  webServer.on("/upload", HTTP_POST, api_upload_response, handleFileUpload);
  webServer.on("/api/lfs/upload", HTTP_POST, api_lfs_upload);

  webServer.onNotFound([]() {
    String uri = webServer.uri();
    if (uri == "/") {
      api_root();
      return;
    }
    if (serve_littlefs_uri(uri)) {
      return;
    }
    if (uri == "/favicon.ico") {
      api_favicon();
      return;
    }
    webServer.send(404, "text/plain", "404 Not Found");
  });

  httpUpdater.setup(&webServer, "/update", OTA_USER, OTA_PASSWORD);
  webServer.begin();
  Serial.println("[WEB] Serwer HTTP uruchomiony na porcie 80");
  Serial.println("[WEB] OTA update: http://<IP>/update");
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin(true)) {
    Serial.println("CRITICAL: LittleFS mount failed! Restart za 2s...");
    delay(2000);
    ESP.restart();
  }

  load_species_profiles();
  configMgr.load();
  configMgr.load_learning_data();

  for (int i = 0; i < MAX_SENSORS; i++) {
    last_sensor.temp[i] = -99.0f;
  }

  if (sanitize_legacy_usb_pin_conflicts()) {
    Serial.println("[CFG] Wykryto konflikt z pinami USB. Zapisuję poprawioną konfigurację.");
    configMgr.save();
  }

  for (int i = 0; i < MAX_TRAYS; i++) {
    int pin = configMgr.cfg.trays[i].pin;
    if (pin < 0 || pin == 19 || pin == 20) continue;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  delay(1000);
  Serial.println("\n=== INKUBATOR ESP32-S3 v2.0 ===");

  Serial.printf("\n[I2C] Skanuję magistralę na GPIO%d (SDA) i GPIO%d (SCL)...\n",
                configMgr.cfg.sensors.i2c_sda_pin,
                configMgr.cfg.sensors.i2c_scl_pin);
  pinMode(configMgr.cfg.sensors.i2c_sda_pin, INPUT_PULLUP);
  pinMode(configMgr.cfg.sensors.i2c_scl_pin, INPUT_PULLUP);
  Wire.begin(configMgr.cfg.sensors.i2c_sda_pin,
             configMgr.cfg.sensors.i2c_scl_pin);
  Wire.setClock(100000);
  byte count = 0;
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.printf("[I2C] Znaleziono urządzenie na 0x%02X\n", i);
      if (i == 0x77) Serial.println("      ^ To może być BMP180!");
      count++;
    }
  }
  if (count == 0) {
    Serial.println("[I2C] ⚠️  BRAK urządzeń! Sprawdź:");
    Serial.println("    - Czy BMP180 zasilany jest 3.3V?");
    Serial.println("    - Czy SDA=GPIO4 i SCL=GPIO5?");
    Serial.println("    - Czy są rezystory pull-up (4.7kΩ-10kΩ)?");
  }
  Serial.println();

  mpu_i2c_found = false;
  for (uint8_t addr : {0x68, 0x69}) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      mpu_i2c_found = true;
      Serial.printf("[MPU] Wykryto urządzenie MPU6050 na adresie 0x%02X\n", addr);
      break;
    }
  }
  if (!mpu_i2c_found) {
    Serial.println("[MPU] Nie znaleziono MPU6050 na magistrali I2C");
  }

  Serial.println("[MPU] Inicjalizacja MPU6050...");
  mpu_sensor_ok = false;
  if (mpu_i2c_found) {
    mpu_sensor_ok = mpu.begin();
    if (mpu_sensor_ok) {
      mpu.calcGyroOffsets();
      last_mpu_read_ms = 0;
      last_mpu_angle_deg = NAN;
      Serial.println("[MPU] MPU6050 gotowy");
    } else {
      Serial.println("[MPU] Nie udało się zainicjalizować MPU6050");
    }
  } else {
    Serial.println("[MPU] Pomijam inicjalizację MPU6050, bo nie znaleziono urządzenia I2C");
  }

  unsigned long long saved_start = 0;
  uint8_t saved_mode = 0;
  bool has_saved_start = load_start_time(saved_start, saved_mode);

  rtc_ok = rtc.begin();
  if (rtc_ok) {
    if (rtc.lostPower()) {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      Serial.println("[RTC] DS3231 stracił zasilanie - ustawiono czas kompilacji.");
    }
    unsigned long long now_unix = rtc.now().unixtime();
    if (has_saved_start && saved_mode == START_MODE_UNIX && saved_start > 0 && saved_start <= now_unix + 300ULL) {
      incubation_start_unix = saved_start;
      Serial.printf("[RTC] Start inkubacji z EEPROM (unix): %llu\n", incubation_start_unix);
    } else {
      incubation_start_unix = now_unix;
      save_start_time(incubation_start_unix, START_MODE_UNIX);
      Serial.printf("[RTC] Start inkubacji ustawiony na teraz: %llu\n", incubation_start_unix);
    }
    system_start_ms = 0;
    incubation_elapsed_base_ms = 0;
    last_saved_elapsed_ms = 0;
  } else {
    system_start_ms = millis();
    if (has_saved_start && saved_mode == START_MODE_MILLIS) {
      incubation_elapsed_base_ms = saved_start;
      last_saved_elapsed_ms = saved_start;
      Serial.printf("[RTC] Brak DS3231 - fallback elapsed z EEPROM: %llu ms\n", incubation_elapsed_base_ms);
    } else {
      incubation_elapsed_base_ms = 0;
      last_saved_elapsed_ms = 0;
      save_start_time(incubation_elapsed_base_ms, START_MODE_MILLIS);
      Serial.println("[RTC] Brak DS3231 - fallback elapsed start=0");
    }
    incubation_start_unix = 0;
  }

  sensors_init();
  tray_init(0);
  load_rotation_state();

  // PWM Grzałki – 25 kHz (cichsza praca)
  for (int ch = 0; ch < MAX_HEATERS_HW; ch++) {
    int pin = heater_pin_for_index(ch);
    if (pin < 0) continue;
    ledcSetup(ch, 25000, 10);
    ledcAttachPin(pin, ch);
    ledcWrite(ch, 0);
    Serial.printf("[LEDC] Heater CH%d: pin=%d, freq=25kHz, resolution=10bit\n", ch, pin);
  }

  pinMode(48, OUTPUT);
  digitalWrite(48, HIGH);
  Serial.println("[LED] GPIO 48 - forced OFF (HIGH)");

  if (configMgr.cfg.system.fan_pin >= 0) {
    pinMode(configMgr.cfg.system.fan_pin, OUTPUT);
    digitalWrite(configMgr.cfg.system.fan_pin, HIGH);
    Serial.printf("[FAN] Pin %d\n", configMgr.cfg.system.fan_pin);
  }

  for (int i = 0; i < MAX_TRAYS; i++) {
    int pin = configMgr.cfg.trays[i].pin;
    if (pin == 19 || pin == 20) {
      Serial.printf("[TRAY %d] POMINIETO GPIO%d (USB D-/D+). Ustaw bezpieczny pin w konfiguracji.\n", i, pin);
      continue;
    }
    if (pin >= 0) {
      int pwm_channel = 4 + i;
      ledcSetup(pwm_channel, 1000, 8);
      ledcAttachPin(pin, pwm_channel);
      tray_states[i].pin_pwm_channel = pwm_channel;
      ledcWrite(pwm_channel, configMgr.cfg.trays[i].signal_active_low ? 255 : 0);
      Serial.printf("[TRAY %d] PWM rotation: GPIO%d -> LEDC ch%d\n", i, pin, pwm_channel);
    }
  }

  // Dyfuzor – tylko pierwszy
  if (configMgr.cfg.diffusers.count > 0) {
    int pin = configMgr.cfg.diffusers.pins[0];
    if (pin >= 0) {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, HIGH);
      Serial.printf("[DIFFUSER] Główny dyfuzor na GPIO%d\n", pin);
    }
    for (int i = 1; i < configMgr.cfg.diffusers.count; i++) {
      int p = configMgr.cfg.diffusers.pins[i];
      if (p >= 0) {
        pinMode(p, OUTPUT);
        digitalWrite(p, HIGH);
        Serial.printf("[DIFFUSER] Dodatkowy dyfuzor %d na GPIO%d (wyłączony)\n", i, p);
      }
    }
  } else {
    Serial.println("[DIFFUSER] Brak skonfigurowanych dyfuzorów.");
  }

  if (!LittleFS.exists("/inkubator_log.csv")) csv_create_header();

  // Używamy tylko trybu AP na razie.
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(WIFI_AP_IP, WIFI_AP_IP, WIFI_AP_NETMASK);
  if (!WiFi.softAP(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println("[WiFi] Błąd uruchomienia AP! Sprawdź konfigurację i dostępność pinu WiFi.");
  } else {
    Serial.printf("\n[WiFi] === ACCESS POINT AKTYWNY ===\n");
  }
  Serial.printf("[WiFi] SSID: %s\n", WIFI_SSID);
  Serial.printf("[WiFi] Haslo: %s\n", WIFI_PASSWORD);
  Serial.printf("[WiFi] IP: %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("[WiFi] Otwórz: http://%s\n", WiFi.softAPIP().toString().c_str());

  wifi_connected = true;
  webserver_setup();

  Serial.println("[SETUP] Inkubator gotowy!\n");
}

// ============================================================================
// LOOP
// ============================================================================
void loop() {
  unsigned long long now = millis();
  bool test_mode = configMgr.cfg.system.test_mode;

  wifi_keepalive();
  webServer.handleClient();

  static unsigned long last_sensor_t = 0;
  static unsigned long last_control_t = 0;
  unsigned long sensor_interval = test_mode ? 5000 : 10000;
  unsigned long control_interval = test_mode ? 2000 : 5000;

  if (now - last_sensor_t > sensor_interval) {
    last_sensor_t = now;
    last_control_t = now;
    sensors_read(last_sensor);
    heaters_control(last_sensor);
    diffusers_control(last_sensor);
  } else if (now - last_control_t > control_interval) {
    last_control_t = now;
    heaters_control(last_sensor);
    diffusers_control(last_sensor);
  }

  handle_learning_record(last_sensor);

  static unsigned long last_tray_t = 0;
  if (now - last_tray_t > 100) {
    last_tray_t = now;
    tray_update_all(get_incubation_elapsed_ms());
    mpu_update_angle_if_rotating(now);
    persist_rotation_events_if_needed();
  }

  static unsigned long last_hist_t = 0;
  if (now - last_hist_t > (test_mode ? 45000 : 240000)) {
    last_hist_t = now;
    history[history_idx] = {
      static_cast<unsigned long long>(last_sensor.elapsed_ms / 60000),
      last_sensor.temp[0],
      last_sensor.temp[1],
      last_sensor.humidity,
      last_sensor.heater_pwm_pct,
      last_sensor.heater_pwm_raw,
      last_sensor.diffuser_on,
      last_sensor.diffuser_active_idx
    };
    history_idx = (history_idx + 1) % HISTORY_SIZE;
  }

  static unsigned long last_log_t = 0;
  if (now - last_log_t > (unsigned long)configMgr.cfg.system.log_interval_sec * 1000) {
    last_log_t = now;
    csv_write(last_sensor);
  }

  unsigned long long elapsed_ms = get_incubation_elapsed_ms();
  if (should_save_fallback_elapsed(elapsed_ms)) {
    last_saved_elapsed_ms = elapsed_ms;
    save_start_time(elapsed_ms, START_MODE_MILLIS);
  }

  handle_serial_commands();
  delay(1);
}