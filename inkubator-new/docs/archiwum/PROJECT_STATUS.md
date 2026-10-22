# 📋 PROJECT CHECKLIST – Inkubator ESP32-S3 v2.1

## ✅ KOMPLETNE KOMPONENTY

### Firmware Core
- [x] **main.cpp** (700+ lines)
  - Loop: sensory, grzałki, dyfuzory, historia, CSV logging
  - WiFi keepalive
  - 14+ API endpoints
  - Circular buffer historia (300 points)
  
- [x] **config_manager.cpp** (200+ lines)
  - JSON serialization (ArduinoJson 7.2.0)
  - SPIFFS + Preferences dual persistence
  - CRC16 verification
  - Factory reset

- [x] **incubation_schedule.h** (470+ lines)
  - 6 gatunków: CHICKEN, DUCK, GOOSE, QUAIL, GUINEA, PHEASANT
  - Harmonogramy day-by-day (temp, humidity, rotation, lockdown, notes)
  - Helper functions: `get_species()`, `get_day()`

- [x] **config_manager.h**
  - Struktury: TrayConfig, SensorConfig, DiffuserConfig, SystemConfig, IncubatorConfig
  - `default_config()` inline function
  - ConfigManager class (load, save, to_json, from_json)

- [x] **tray_controller.h**
  - TrayState struct (64-bit timing)
  - Multi-tray control (max 3 trays)
  - Humidity calculation (`tray_get_chamber_humidity()`)
  - JSON export (`tray_status_json()`)

### Web & Config
- [x] **index.html** (13KB)
  - Aktualny podział zakładek: Status / Panel / Harmonogram / Dziennik Inkubacji / ESP, okablowanie i zabezpieczenia / Logi zdarzeń
  - Widok ESP + okablowanie + zabezpieczenia scalony w jedną zakładkę
  - Sekcja czujników rozszerzona o kondensatory (100nF, 1uF, opcjonalne filtry linii)
  - Mobile-friendly CSS

- [x] **config.json**
  - Taca 1 aktywna (DUCK), Taca 2 domyślnie OFF (rezerwa), Taca 3 OFF (rezerwa)
  - 2 aktywne DS18B20 na wspólnej linii GPIO6
  - DHT22 (GPIO8), RTC DS3231 + AT24C32 (GPIO4/GPIO5)
  - 2 dyfuzory aktywne: GPIO11 i GPIO12
  - BMP180 opcjonalny (domyślnie wyłączony)
  - test_mode: false
  - config_version: 106
  - grzałki: CH0 (GPIO1) + CH1 (GPIO2) aktywne, CH2/CH3 odłączone (`heater_pin_3=-1`, `heater_pin_4=-1`)

### Aktualny stan sprzętu (2026-06-12)
- Dyfuzory: podłączone 2 szt. (GPIO11, GPIO12)
- Grzałka 1 (CH0 / GPIO1): 6m, 12V, 48W (200% mocy pojedynczej grzałki 24W)
- Grzałka 2 (CH1 / GPIO2): 3m, 12V, 24W (100% mocy odniesienia)
- Grzałka 3: 3m, 12V, 24W (na razie odłączona / OFF)
- Tace: aktywna tylko Taca 1 (GPIO16), Taca 2 (GPIO17) i Taca 3 (GPIO18) OFF

### Build Config
- [x] **platformio.ini**
  - ESP32-S3 configuration
  - 921600 baud upload
  - SPIFFS filesystem
  - Build flags (MAX_TRAYS=3, MAX_SENSORS=4, HISTORY_SIZE=300)
  - Biblioteki: DHT, OneWire, DallasTemperature, ArduinoJson 7.2.0

### Documentation
- [x] **BUILD_GUIDE.md**
  - Kompilacja (Command Line + VS Code)
  - Upload (z GPIO0/BOOT workaround)
  - Test na urządzeniu
  - Troubleshooting
  - Checklist

---

## 🔌 API ENDPOINTS (14 Total)

### Status & Monitoring (3)
- [x] `GET /api/status` - temperatura, wilgotność, alerty
- [x] `GET /api/history` - dane do wykresu (300 points)
- [x] `GET /api/log` - pobierz CSV logi

### Trays (4)
- [x] `GET /api/trays` - status wszystkich tac
- [x] `POST /api/tray/set-day?tray=0&day=12`
- [x] `POST /api/tray/set-species?tray=0&species=DUCK`
- [x] `POST /api/tray/set-active?tray=0&active=true`

### Schedules & Config (4)
- [x] `GET /api/schedule?species=CHICKEN` - harmonogram
- [x] `GET /api/species` - lista gatunków
- [x] `GET /api/config` - pobierz config
- [x] `POST /api/config` - zapisz config (+ factory reset)

### System (3)
- [x] `POST /api/reset` - reset licznika
- [x] `POST /api/reboot` - restart ESP32
- [x] `GET /` - index.html

---

## 🎯 Features Implemented

### Hardware Support
- [x] DS18B20 (4 czujniki, OneWire)
- [x] DHT11 (wilgotność)
- [x] ADC water level
- [x] GPIO door sensor
- [x] PWM heater control (4 channel backend, 5kHz, 8-bit)
- [x] Diffuser control (3 pin GPIO)
- [x] Tray rotation motors (3 pin GPIO)

### Firmware Logic
- [x] HYBRID heating (aquarium base + PWM fine-tuning)
- [x] Ramp-up (0→50% over 60 min)
- [x] PID-like maintain (±0.5°C hysteresis)
- [x] Multi-tray independent timing
- [x] Automatic lockdown activation
- [x] Automatic rotation control
- [x] Humidity hysteresis + cyclic mode (dyfuzory)
- [x] Safe elapsed time calculation (64-bit, rollover protection)
- [x] Circular buffer history (300 samples)
- [x] CSV logging to SPIFFS

### Web Features
- [x] Responsive dashboard (6 zakładek)
- [x] Real-time status updates (3s refresh)
- [x] Harmonogram gatunków (day-by-day)
- [x] Zintegrowany panel ESP + okablowanie + zabezpieczenia
- [x] Export CSV + czyszczenie historii
- [x] Alarmy runtime i statusy topbar
- [x] Mobile-friendly design

### Persistence
- [x] EEPROM (start_time, 8 bytes, survives power cycles)
- [x] SPIFFS (/config.json, /inkubator_log.csv, /index.html)
- [x] Preferences (NVS backup with CRC32)
- [x] Config versioning (CONFIG_VERSION=102)

### WiFi & Network
- [x] WiFi keepalive (30s check)
- [x] Automatic reconnect
- [x] HTTP WebServer on port 80
- [x] JSON API responses
- [x] CORS headers (if needed)

### Wiring & Protection Notes
- [x] Pull-up DS18B20: 4.7kΩ do 3V3
- [x] Pull-up DHT11: 10kΩ do 3V3
- [x] Kondensatory lokalne: 100nF (DS18B20/BMP180), 100nF + 1uF (DHT11)
- [x] Ochrona mocy: bezpieczniki gałęzi + diody flyback + pull-down gate

---

## 📊 Project Statistics

| Metrika | Wartość |
|---------|---------|
| **Firmware Size** | ~817 KB (26.6% of 3072 KB) |
| **RAM Usage** | ~160 KB (49.9% of 320 KB) |
| **SPIFFS Size** | ~20 KB (index.html, config.json) |
| **Total Lines of Code** | ~1500+ (excluding comments) |
| **Compilation Time** | ~60-90 seconds |
| **API Endpoints** | 14+ |
| **Supported Species** | 6 (CHICKEN, DUCK, GOOSE, QUAIL, GUINEA, PHEASANT) |
| **Max Trays** | 3 (hardware configurable) |
| **Max Heaters (firmware)** | 4 (CH0-CH3) |
| **Max Sensors** | 4 (DS18B20 + DHT11) |
| **History Points** | 300 (circular buffer) |

---

## 🚀 DEPLOYMENT CHECKLIST

### Pre-Upload
- [x] Code compiles without errors
- [x] All headers have `#pragma once`
- [x] No undefined references
- [x] SPIFFS assets built correctly

### Upload Process
- [ ] **Firmware**: `platformio run --target upload`
- [ ] **SPIFFS**: `platformio run --target uploadfs`
- [ ] **Monitor**: `platformio device monitor --baud 115200`

### First Boot
- [ ] Serial output appears (=== INKUBATOR ===)
- [ ] WiFi connects (CFG loaded, WiFi OK)
- [ ] WebServer starts ([WEB] Server on port 80)
- [ ] Sensors detected ([SENSOR] DS18B20 #0)

### WebUI Test
- [ ] Dashboard loads: `http://192.168.1.100`
- [ ] Status tab shows live data
- [ ] Schedule dropdown works
- [ ] API endpoints respond with JSON

### Production Setup
- [ ] Set test_mode = false
- [ ] Configure species for each tray
- [ ] Verify heater PWM response
- [ ] Test humidity control
- [ ] Monitor CSV logs

---

## 📝 Known Limitations & TODOs

### Current Limitations
- ⚠️ GPIO0/BOOT bug requires manual GPIO0→GND during upload
- ⚠️ test_mode uses simulated data (real sensors when false)
- ⚠️ No SD card support (SPIFFS only, ~20KB usable)
- ⚠️ No authentication on WebUI (local network only)
- ⚠️ WebUI obsługuje ręcznie tylko 2 grzałki (CH0/CH1), CH2/CH3 działają w automacie po konfiguracji pinów

### Future Enhancements
- [ ] ESPAsyncWebServer (better performance)
- [ ] MQTT integration (home automation)
- [ ] Cloud logging (Firebase, AWS)
- [ ] Mobile app (PWA)
- [ ] Advanced analytics (ML predictions)
- [ ] Backup power (UPS monitoring)
- [ ] Multi-device mesh network
- [ ] Over-the-air (OTA) firmware updates

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| v2.2 | 2026-05-17 | WebUI: dedykowana zakładka okablowania i zabezpieczeń; dodane kondensatory dla linii sygnałowych; aktualizacja dokumentacji i konfiguracji runtime |
| v1.0 | 2026-05-16 | **DEPLOYMENT READY** – Complete firmware, WebUI, 14 API endpoints, all 6 species, multi-tray control |
| v2.0 | 2026-05 | Complete refactor (incubation_schedule, config_manager, main.cpp) |
| v2.1 | 2026-05 | API endpoints, WebUI, config.json, BUILD_GUIDE |

**Backup**: `j:\inkubator_v1.zip` (12.35 MB) – Complete source code snapshot

---

## 📞 Support

### Debug Output Expected
```
=== INKUBATOR ESP32-S3 v2.1 ===
[CFG] Wczytano z SPIFFS
[SENSOR] DS18B20 #0 na GPIO6 (Komora jaj)
[SENSOR] DHT11 na GPIO8
[WiFi] Łączenie...................
[WiFi] OK → http://192.168.1.100
[WEB] Serwer HTTP uruchomiony na porcie 80
[SETUP] Inkubator gotowy!
```

### Next Steps
1. **Compile & Upload** (see BUILD_GUIDE.md)
2. **Test on Device** (verify sensors)
3. **Configure Species** (WebUI or API)
4. **Monitor Logs** (CSV + WebUI)
5. **Deploy** (set test_mode=false)

---

✅ **PROJECT STATUS: READY FOR DEPLOYMENT**

Generated: 2026-05-16
