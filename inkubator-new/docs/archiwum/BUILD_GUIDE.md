# 🚀 QUICK START – Kompilacja i Upload

## ✅ Status Projektu
- ✅ Firmware kompletny (`main.cpp`, `config_manager.cpp`)
- ✅ WebUI gotowy (`index.html` z 4 tabs)
- ✅ Harmonogramy 6 gatunków (`incubation_schedule.h`)
- ✅ Konfiguracja JSON (`config.json`)
- ✅ API endpoints 14+ (`/api/status`, `/api/trays`, itp.)
- ✅ Bez błędów kompilacji

---

## 1️⃣ Przygotowanie

### Wymagania
- **PlatformIO** (`pip install platformio` lub VS Code extension)
- **Python 3.8+**
- **USB-C kabel** do ESP32-S3
- **Terminal/PowerShell** z dostępem do `platformio` lub `python -m platformio`

Na Windows w tym repo najpewniejsze jest użycie:
`%USERPROFILE%\\.platformio\\penv\\Scripts\\platformio.exe`

### Struktura (OK ✅)
```
j:\inkubator/
├── platformio.ini                    ✅ Root config
├── firmware/
│   ├── src/
│   │   ├── main.cpp                 ✅ Firmware 700+ lines
│   │   └── config_manager.cpp       ✅ JSON impl. 200+ lines
│   ├── include/
│   │   ├── incubation_schedule.h    ✅ 6 gatunków
│   │   ├── config_manager.h         ✅ Struktury
│   │   └── tray_controller.h        ✅ Multi-tray control
│   └── data/
│       ├── config.json              ✅ Default config (LittleFS)
│       ├── index.html               ✅ WebUI (LittleFS)
│       └── dev/                     ✅ Dev assets for WebUI build workflow
└── BUILD_GUIDE.md
```

---

## 2️⃣ Kompilacja

### Command Line
```bash
cd j:\inkubator
%USERPROFILE%\.platformio\penv\Scripts\platformio.exe run -e esp32s3
```

### VS Code
1. **Otwórz folder**: `j:\inkubator`
2. **PlatformIO** (lewy pasek) → **Build**
3. Czekaj: ~60-90 sekund

### Oczekiwany wynik
```
RAM:   159.9 KB / 320.0 KB (49.9%)
Flash: 817.4 KB / 3072.0 KB (26.6%)
========= [SUCCESS] Took X.XX seconds =========
```

### ⚠️ Jeśli błędy:
- **"Undefined reference..."** → Sprawdź czy wszystkie `.h` mają `#pragma once`
- **"Cannot find..."** → Uruchom `platformio run --target clean`
- **Brak bibliotek** → `platformio lib install "DHT sensor library"`

---

## 3️⃣ Upload (ESP32-S3)

### Jedno źródło WebUI (ważne)
- Aktywny plik UI dla ESP: `firmware/data/index.html`
- `uploadfs` wysyła do LittleFS tylko zawartość katalogu `firmware/data/`
- Root `index.html` jest informacyjny (nie jest deployowany na ESP)
- Backup starszego UI: `backups/index.backup.2026-05-19_175255.html`
- Pliki developerskie WebUI mają teraz swoje źródła w `firmware/data/dev/`.
- Narzędzie do ich ekstrakcji i składania: `tools/build_webui.py`.

#### Dev workflow
- Wyodrębnij dev assets z aktualnego `firmware/data/index.html`:
  - `python tools/build_webui.py --extract`
- Złóż `firmware/data/index.html` z dev sources:
  - `python tools/build_webui.py --build`
- Aby zminimalizować CSS/JS przy buildzie, dodaj `--minify`.

### ⚠️ WAŻNE: GPIO0/BOOT Bug
ESP32-S3 DevKit ma problem z wejściem w bootloader. Rozwiązania:

#### Opcja A: Manualna
```bash
1. Podłącz ESP32 via USB
2. Sprawdź port: COM3, COM4, itp.
3. Uruchom: platformio run --target upload
4. Gdy zobaczysz "Connecting...", przytrzymaj GPIO0→GND (1-2 sek)
5. Puść – powinno się wgrać
```

#### Opcja B: Modyfikacja sprzętu (deweloper)
- Zmień capacitor na CH340 (jeśli jest)
- Lub wyślij GPIO0 bezpośrednio na DTR przez rezystor

#### Opcja C: esptool bezpośrednio
```bash
python -m esptool --chip esp32s3 --port COM3 --baud 921600 \
  --before default_reset --after hard_reset \
  write_flash -z --flash_mode dio --flash_freq 80m 0x0 .pio/build/esp32s3/firmware.bin
```

### Upload Steps

#### 1. Firmware
```bash
cd j:\inkubator
%USERPROFILE%\.platformio\penv\Scripts\platformio.exe run -e esp32s3 --upload-port COM8 --target upload
```

**Output:**
```
Connecting...
Chip is ESP32-S3 (revision 0)
Crystal is 40MHz
Mac address: XX:XX:XX:XX:XX:XX
```

#### 2. LittleFS (WebUI + config.json)
```bash
%USERPROFILE%\.platformio\penv\Scripts\platformio.exe run -e esp32s3 --upload-port COM8 --target uploadfs
```

**Output:**
```
LittleFS image will be 19672 bytes
Uploading 20480 bytes of LittleFS image (19672 non-padding bytes)
Wrote 19672 bytes at address 0x2f0000
```

#### 3. Monitor Serial
```bash
platformio device monitor --baud 115200
```

**Oczekiwany output:**
```
=== INKUBATOR ESP32-S3 v2.1 ===
[CFG] Wczytano z LittleFS
[SENSOR] DS18B20 #0 na GPIO6 (Komora jaj)
[SENSOR] DHT22 na GPIO8
[WiFi] Łączenie...................
[WiFi] OK → http://192.168.1.100
[WEB] Serwer HTTP uruchomiony na porcie 80
[SETUP] Inkubator gotowy!
```

---

## 4️⃣ Test na Urządzeniu

### WiFi
1. Szukaj sieci: `INKUBATOR`
2. Hasło: `5465INKUBATOR5465`
3. IP: `192.168.1.100` (lub `http://INKUBATOR.local`)

### WebUI
```
http://192.168.1.100
```

#### Dashboard (Tab 1)
- ✅ Powinno pokazać: "DNI DO WYLĘGU: --"
- ✅ Temperatura: "--.-- °C" (test_mode=true → dane symulowane)
- ✅ Wykres: Historia (może być pusta)

### API Test
```bash
# Status
curl http://192.168.1.100/api/status | jq

# Harmonogram Kury
curl http://192.168.1.100/api/schedule?species=CHICKEN | jq

# Tace
curl http://192.168.1.100/api/trays | jq
```

---

## 5️⃣ Konfiguracja

### test_mode
```json
// firmware/data/config.json
"system": {
  "test_mode": true  ← Dane symulowane (demo)
}
```

**Zmiana:**
1. Edytuj `firmware/data/config.json`
2. `test_mode: false`
3. Uruchom: `platformio run --target uploadfs`
4. Restart: `http://IP/api/reboot`

### Gatunek & Tace
- Zmień w WebUI → **Harmonogram** tab (dropdown)
- Lub: `POST /api/tray/set-species?tray=0&species=DUCK`

---

## 🔧 Porty GPIO

| Pin | Funkcja | Rola |
|-----|---------|------|
| GPIO1, GPIO2 | PWM Heater | Grzałki CH0 i CH1 (UI + AUTO) |
| GPIO (konfigurowalne) | PWM Heater | Grzałki CH2 i CH3 (AUTO; gotowe w firmware) |
| GPIO4, GPIO5 | I2C | RTC DS3231 (0x68) + AT24C32 (0x57), opcjonalnie BMP180 (0x77) |
| GPIO6 | OneWire | DS18B20 sensory (wspólna linia) |
| GPIO8 | Data | DHT11 (wilgotność) |
| GPIO9 | ADC | Water level |
| GPIO10 | GPIO | Door sensor |
| GPIO16, GPIO17 | PWM | Tray motors (LEDC CH4, CH5) |
| GPIO18 | PWM | Tray motor rezerwa (LEDC CH6) |
| GPIO21 | GPIO | Diffuser (nawilżacz) |
| GPIO19, GPIO20 | USB | Natywne USB D-/D+ (NIE używać do sterowania) |

---

## 📊 Monitorowanie

### Serial Port
```bash
platformio device monitor --port COM3 --baud 115200 --filter esp32_exception_decoder
```

### CSV Log
```bash
# Pobierz logi
curl http://192.168.1.100/api/log > inkubator_log.csv

# Format: time_sec, temp_chamber, temp_secondary, humidity, water_level, door, heater_pct
```

### JSON Endpoints
| Endpoint | Cel |
|----------|-----|
| `/api/status` | Status bieżący |
| `/api/history` | Dane do wykresu |
| `/api/config` | Konfiguracja |
| `/api/trays` | Stan tac |
| `/api/schedule?species=CHICKEN` | Harmonogram |

Uwaga o grzałkach:
- Firmware obsługuje 4 kanały grzałek (CH0-CH3).
- Aktualny HTML pokazuje i steruje ręcznie tylko CH0 i CH1.
- CH2 i CH3 uruchamiają się po ustawieniu `heater_pin_3` / `heater_pin_4` w konfiguracji i pracują w automacie.

---

## 🆘 Troubleshooting

### ❌ Upload fails: "Failed to connect to COM3"
- Sprawdź czy device widoczny w Device Manager
- Zmień port w `platformio.ini`: `upload_port = COM4`
- Spróbuj: `esptool erase_flash` (wymaga GPIO0→GND)

### ❌ Serial garbage output
- Zmień `monitor_speed = 115200` (Check vs DevKit)
- Spróbuj: `platformio run --target monitor` (resetuje device)

### ❌ "Brak czujników" (test_mode=false)
- Sprawdź połączenie DS18B20 / DHT22
- Uruchom test_mode=true i obserwuj symulowane dane
- Sprawdź GPIO pins w Serial Monitor: `[SENSOR] DS18B20 #0 na GPIO6`

### ❌ WebUI się nie ładuje
- Sprawdź czy LittleFS wgrany: `platformio run --target uploadfs`
- Sprawdź czy masz połączenie WiFi
- Otwórz DevTools (F12) i sprawdź Network tab

---

## ✅ Checklist Przed Wdrożeniem

- [ ] Kompilacja bez błędów (`platformio run` → SUCCESS)
- [ ] Firmware wgrany (`platformio run --target upload`)
- [ ] LittleFS wgrany (`platformio run --target uploadfs`)
- [ ] WebUI dostępny: `http://192.168.1.100`
- [ ] `/api/status` zwraca JSON
- [ ] Czujniki widoczne w Serial Monitor
- [ ] test_mode=false dla produkcji
- [ ] Grzałki reagują na temperatuę
- [ ] Dyfuzory działają w trybie cyklicznym

---

## 📞 Następne Kroki

1. **Testowanie czujników**: Zmień temperatuę ręcznie i obserwuj odczyt
2. **Wgranie rzeczywistych jaj**: Zmień test_mode=false, ustaw gatunek
3. **Monitoring**: Obserwuj WebUI co 2-4 godziny
4. **Logi**: Pobierz CSV logi dla analizy po zakończeniu inkubacji

---

Powodzenia! 🥚✨
