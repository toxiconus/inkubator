# ⚡ QUICK REFERENCE - Szybki Przepis

Cheat sheet do szybkiego podglądu PIN-ów, konfiguracji i polece​ń.

## 📌 Pinout - Szybka Tabela

| LPH | GPIO | Typ | Funkcja | Zasilanie | Obciążenie | Stan Domyślny |
|-----|------|-----|---------|-----------|-----------|---------------|
| 1 | 2 | PWM | Grzałka | 3.3V | +12V/220V | 0/255 |
| 2 | 19 | PWM | Fan 1 | 3.3V | +12V | 200/255 |
| 3 | 18 | PWM | Fan 2 | 3.3V | +12V | 0/255 (cykliczny) |
| 4 | 5 | PWM | Fan 3 | 3.3V | +12V | 0/255 (awaryjny) |
| 5 | 12 | GPIO | Nawilżacz | 3.3V | Relay 5V | LOW |
| 6 | 14 | GPIO | Taca | 3.3V | Relay 5V | LOW |
| 7 | 13 | GPIO | Światło | 3.3V | LED/Relay | LOW |
| 8 | 4 | Input | OneWire (DS18B20 x3) | 3.3V + pull-up 4.7kΩ | 3.3V | Input |
| 9 | 15 | Input | DHT22 (Temp+Humidity) | 3.3V + pull-up 10kΩ | 3.3V | Input |
| 10 | 35 | ADC | Przycisk Menu | 3.3V | - | Future |
| 11 | 34 | ADC | Przycisk OK | 3.3V | - | Future |

## 🌡️ Parametry Temperaturowe

| Parametr | Wartość | Uwagi |
|----------|---------|-------|
| Dni 1-21 | 37.5°C | Okres inkubacji |
| Dni 22-28 | 37.2°C | Okres kluwania |
| Tolerancja temp | ±0.5°C | Histereza do zmiany stanu |
| Wilg. dni 1-18 | 40% | Początek |
| Wilg. dni 19+ | 75% | Koniec (kluwanie) |
| Tolerancja wilg | ±5% | Histereza |

## 🔧 Konfiguracja (config.h)

```cpp
// TEMPERATURY
#define TARGET_TEMP 37.5
#define TEMP_TOLERANCE 0.5
#define TEMP_DAY_1_21 37.5
#define TEMP_DAY_22_HATCH 37.2

// WILGOTNOŚĆ
#define TARGET_HUMIDITY_EARLY 40    // Dni 1-18
#define TARGET_HUMIDITY_LATE 75     // Dni 19+
#define HUMIDITY_TOLERANCE 5

// WIATRAKI
#define FAN1_PWM 200                // Zawsze 200/255
#define FAN2_CYCLE_ON 300           // ms ON
#define FAN2_CYCLE_OFF 200          // ms OFF
#define FAN3_TEMP_THRESHOLD 38.5    // Włącza się >38.5°C

// TACA (obracanie jaj)
#define TRAY_CYCLE_INTERVAL 3600    // Co 1h (sekundy)
#define TRAY_ROTATION_TIME 10       // 10s pracy
#define TRAY_STOP_DAY 21            // Wyłączy się dzień 21+

// ŚWIATŁO
#define LIGHT_HATCH_DAY 21          // Włącza się dzień 21

// OPÓŹNIENIA
#define SENSOR_READ_INTERVAL 2000   // Co 2s odczyt
#define CONTROL_LOOP_INTERVAL 5000  // Co 5s sterowanie
```

## 🎮 Główne Funkcje

### Sensory (sensors.h / sensors.cpp)
```cpp
sensors.begin();                        // Inicjalizacja
sensors.update();                       // Odczyt
float temp = sensors.getDS18B20Temp(0); // Odczyt DS18B20 #1-3
float humidity = sensors.getDHTHumidity();
float tempDHT = sensors.getDHTTemp();
```

### Kontroler (controller.h / controller.cpp)
```cpp
controller.begin();                     // Inicjalizacja
controller.update();                    // Pętla logiki

// PWM (0-255)
controller.setHeaterPower(255);         // Grzałka 100%
controller.setFan1Power(200);           // Fan1 78%
controller.setFan2Power(0);             // Fan2 OFF
controller.setFan3Power(255);           // Fan3 ON

// GPIO (HIGH/LOW)
controller.setHumidifier(true);         // Nawilżacz ON
controller.setTray(true);               // Taca obraca ON
controller.setLight(false);             // Światło OFF

// Zarządzanie dniem
controller.setIncubationDay(1);         // Ustaw dzień
int day = controller.getIncubationDay();
```

## 📡 Serial Output - Interpretacja

```
DZIEŃ: 1/28                     ← Bieżący dzień / 28
───────────────────────────────────
🌡️  TEMPERATURA:
    DS1:37.2°C DS2:37.5°C DS3:37.3°C   ← Każdy czujnik osobno
    Średnia: 37.3°C Cel: 37.5°C        ← Średnia i cel

💧 WILGOTNOŚĆ: 42.5% Cel: 40%        ← Aktualna vs cel

🔥 GRZAŁKA: 200/255                   ← PWM 0-255
                                      ← 0=OFF, 255=100%, 200≈78%

💨 WIATRAKI: F1:ON F2:OFF F3:OFF      ← Status każdego
                                      ← F1 stały, F2 cykliczny, F3 awaryjny

💦 NAWILŻACZ: OFF                     ← ON/OFF

🔄 TACA: OBRACA                       ← OBRACA / STOP

💡 ŚWIATŁO: OFF                       ← ON/OFF
```

## 🚀 Komendy Build & Upload

### PlatformIO (VSCode)
```bash
# Build
platformio run

# Upload na ESP32
platformio run -t upload

# Serial Monitor
platformio device monitor -b 115200

# Czyszczenie SPIFFS
platformio run -t erase

# Upload + Serial (jedną linią)
platformio run -t upload && platformio device monitor -b 115200
```

### Arduino IDE
```bash
# Kompiluj: Sketch → Verify
# Upload: Sketch → Upload
# Serial: Tools → Serial Monitor (115200)
```

## 🧪 Szybkie Testy w Symulatorze

### Przyspieszenie dni (dla testów)
Zmień w `main.cpp` / `sketch.ino`:
```cpp
// NORMALNE: 1 dzień = 86400 sekund (24h)
int calculatedDay = (elapsedSeconds / 86400) + 1;

// SZYBKIE: 1 dzień = 1 sekunda
int calculatedDay = (elapsedSeconds / 1) + 1;

// ŚREDNIE: 1 dzień = 10 sekund
int calculatedDay = (elapsedSeconds / 10) + 1;
```

### Ręczne włączanie sterów (w setup / loop)
```cpp
controller.setHeaterPower(255);    // Włącz grzałkę na 100%
controller.setHumidifier(true);    // Włącz nawilżacz
controller.setFan1Power(255);      // Maksymalna moc Fan1
controller.setLight(true);         // Włącz światło
```

## 🔍 Troubleshooting

| Problem | Przyczyna | Rozwiązanie |
|---------|-----------|------------|
| DS18B20 pokazuje -999 | Brak połączenia OneWire | Sprawdź rezystor pull-up 4.7kΩ, GND |
| DHT22 pokazuje NaN | Zbyt szybkie odczyty | Czekaj ≥2s między odczytami |
| PWM nie działa | Błędny GPIO/Kanał | Sprawdź config.h, czy GPIO ma PWM |
| Grzałka nie grzeje | Logika OFF lub 0 PWM | Sprawdź setHeaterPower(), targetTemp |
| Serial Monitor pusty | Zły baud rate | Ustaw 115200 baud, sprawdź RX pin |
| Wiatraki nie pracują | Brak zasilania 12V | Sprawdź czy +12V jest na MOSFET Drain |

## 📚 Linki do Dokumentacji

- [Pełne Schematy](SCHEMATY.md)
- [Przewodnik Montażu](MONTAZ.md)
- [Simulator na Wokwi](wokwi/README_WOKWI.md)
- [Symulator - One File](simulator/README_SIMULATOR.md)

## 🎯 Workflow Development

```
1. TESTOWANIE LOGIKI
   ↓
2. arduino_simulator.ino → PlatformIO → Serial Monitor
   ↓
3. TESTY SENSORÓW
   ↓
4. Podłącz czujniki (DS18B20, DHT22) na breadboard
   ↓
5. TESTY PWM/GPIO
   ↓
6. Podłącz LED na PWM GPIO
   ↓
7. INTEGACJA HARDWARE
   ↓
8. Podłącz rzeczywiste silniki / relay
   ↓
9. DEPLOYMENT
   ↓
10. Wgraj kod na ESP32 w obudowie
```

## ⚡ Stałe czasowe

| Stała | Domyślna | Opis |
|-------|----------|------|
| SENSOR_READ_INTERVAL | 2000ms | Jak często czytać DS/DHT |
| CONTROL_LOOP_INTERVAL | 5000ms | Jak często sterować |
| FAN2_CYCLE_ON | 300ms | Fan2 włączony |
| FAN2_CYCLE_OFF | 200ms | Fan2 wyłączony |
| TRAY_CYCLE_INTERVAL | 3600s | Co ile sekund obracać tacę |
| TRAY_ROTATION_TIME | 10s | Jak długo pracuje motor |

## 🎨 Status LED - Co oznacza?

| LED | Kolor | Stan | Znaczenie |
|-----|-------|------|-----------|
| Grzałka | Czerwona | ON | Grzeje |
| Wiatraki | Niebieskie | ON | Pracują |
| Nawilżacz | Cyan | ON | Nawilża |
| Taca | Żółta | ON | Obraca |
| Światło | Pomarańczowa | ON | Włączone |

---

**Zawsze odwołuj się do tego dokumentu dla szybkiego dostępu! 🚀**

