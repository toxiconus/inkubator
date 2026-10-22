# Firmware - ESP32-S3 Inkubator

Framework: **PlatformIO + Arduino**
Platforma: **espressif32**
Board: **esp32-s3-devkitc-1** (lub odpowiednik dla N8R2)

---

## Planowana struktura kodu

```
firmware/
├── platformio.ini            # Konfiguracja PlatformIO
├── src/
│   ├── main.cpp              # Pętla główna (setup/loop)
│   ├── heating.cpp/.h        # Sterowanie grzałkami (MOSFET PWM)
│   ├── sensors.cpp/.h        # Odczyt czujników temp/wilgotności
│   ├── pid.cpp/.h            # Regulator PID temperatury
│   ├── humidity.cpp/.h       # Sterowanie nawilżaczem (faza 2)
│   ├── egg_turner.cpp/.h     # Obracanie jaj (faza 3)
│   ├── display.cpp/.h        # Wyświetlacz OLED (faza 1+)
│   ├── webserver.cpp/.h      # Web UI przez WiFi (faza 4)
│   └── config.h              # Stałe: piny, nastawy, progi
├── include/
│   └── types.h               # Struktury danych (IncubatorState itp.)
├── lib/                      # Lokalne kopie bibliotek
└── test/
    └── test_pid.cpp          # Test regulatora PID
```

---

## Zależności (biblioteki)

| Biblioteka | Zastosowanie |
|------------|--------------|
| `OneWire` + `DallasTemperature` | DS18B20 (jeśli wybrany) |
| `Adafruit SHT31` | SHT31 temp+wilgotność |
| `Adafruit SSD1306` | OLED 0.96" I2C |
| `RTClib` (Adafruit) | DS3231 RTC |
| `ESP32 LEDC` | PWM (wbudowany w Arduino ESP32) |
| `ESPAsyncWebServer` | Web UI (faza 4) |
| `ArduinoJson` | API JSON (faza 4) |

---

## platformio.ini (szkielet)

```ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
upload_speed = 921600

; Dla N8R2: PSRAM Quad (OPI dla N16R8)
build_flags =
    -DBOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue
    -DCORE_DEBUG_LEVEL=3

lib_deps =
    ; do uzupełnienia po wyborze sensorów
```

---

## Notatki implementacyjne

- **PWM grzałek**: ESP32-S3 `ledcSetup()` + `ledcWrite()`, freq ~1kHz, rozdzielczość 8-bit (0-255)
- **PID**: nastawy Kp/Ki/Kd eksperymentalne, start: Kp=20, Ki=0.1, Kd=5
- **Temperatura docelowa**: 37.5°C (kurczęta), nastawiana przez web UI
- **Bezpieczeństwo**: górny próg 39°C → wyłączyć grzałki + alarm
- **Watchdog**: ESP32 sprzętowy WDT, reset przy blokadzie pętli
