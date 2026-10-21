# 🐣 INKUBATOR NA ESP32

Projekt automatycznego inkubatora do jaj z kontrolą temperatury, wilgotności i automatycznym obracaniem tacy.

## ✨ Cechy

- **3x DS18B20** - pomiar temperatury w różnych punktach
- **1x DHT22** - pomiar wilgotności i temperatury
- **PWM Grzałka** - sterowanie grzałką z histerezą
- **PWM Wiatraki** - 3 niezależne systemy wentylacji:
  - Wentylator 1: Stały (wymuszony obieg powietrza)
  - Wentylator 2: Cykliczny na/off
  - Wentylator 3: Awaryjne chłodzenie (powyżej progu temp)
- **Relay Nawilżacz** - sterowanie nawilżaczem sonicznym
- **Relay Taca** - automatyczne obracanie tacy (dni 1-20)
- **Relay Światło** - włączanie od dnia kluwania (dzień 21+)
- **Profil czasowy** - parametry zmieniają się w zależności od dnia
- **Przygotowanie do LVGL** - struktura gotowa do podłączenia wyświetlacza

## 📋 Struktura projektu

```
inkubator/
├── platformio.ini          # Konfiguracja PlatformIO
├── include/
│   ├── config.h            # Wszystkie stałe i parametry
│   ├── sensors.h           # Klasa do obsługi sensorów
│   └── controller.h        # Klasa logiki sterowania
├── src/
│   ├── main.cpp            # Pętla główna
│   ├── sensors.cpp         # Implementacja sensorów
│   └── controller.cpp      # Implementacja kontrolera
├── simulator/
│   └── inkubator_simulator.ino  # Jeden plik do symulatora
├── wokwi/
│   ├── sketch.ino          # Kod dla Wokwi.com
│   ├── diagram.json        # Sremat fizyczny
│   └── README_WOKWI.md     # Instrukcja Wokwi
├── SCHEMATY.md             # 📐 PEŁNE SCHEMATY POŁĄCZEŃ
└── README.md               # Ten plik
```

## ⚙️ Pinout ESP32

**📐 Pełne schematy elektryczne i szczegółowe połączenia są w pliku [SCHEMATY.md](SCHEMATY.md)**

### PWM (Timer0) - ledcSetup()
| Funkcja | GPIO | Kanał |
|---------|------|-------|
| Grzałka | 2 | 0 |
| Wentylator 1 | 19 | 1 |
| Wentylator 2 | 18 | 2 |
| Wentylator 3 | 5 | 3 |

### GPIO Digital
| Funkcja | GPIO |
|---------|------|
| DS18B20 (OneWire) | 4 |
| DHT22 | 15 |
| Nawilżacz relay | 12 |
| Taca relay | 14 |
| Światło relay | 13 |
| Przycisk Menu | 35* |
| Przycisk OK | 34* |

*Tylko wejścia (ADC), nie mogą być wyjściami PWM/GPIO cyfrowego

## 📊 Parametry temperaturowe

| Okres | Temperatura | Wilgotność |
|-------|------------|------------|
| Dni 1-21 (Inkubacja) | 37,5°C | 40% |
| Dni 22-28 (Kluwanie) | 37,2°C | 75% |

*Można zmienić w `config.h`*

## 🔧 Konfiguracja

Wszystkie parametry są w `include/config.h`:

```cpp
#define TARGET_TEMP 37.5              // Docelowa temperatura
#define TEMP_TOLERANCE 0.5            // Tolerancja
#define TARGET_HUMIDITY_EARLY 40      // Wilg. dni 1-18
#define TARGET_HUMIDITY_LATE 75       // Wilg. dni 19+
#define TRAY_CYCLE_INTERVAL 3600      // Co ile sekund obrót
#define FAN1_PWM 200                  // Moc went. 1 (0-255)
```

## 📐 Dokumentacja Techniczna

### ⚡ [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - Szybki Przepis
Cheat sheet ze wszystkim czym potrzebujesz na szybko:
- Pinout - szybka tabela
- Parametry temperaturowe
- Główne funkcje
- Troubleshooting

### 🔌 [SCHEMATY.md](SCHEMATY.md) - Pełne Schematy Elektryczne
- Schemat blokowy systemu
- Schemat zasilania i sygnałów
- Szczegółowe połączenia dla każdego pinu
- Tabela ze wszystkimi pinami
- Schematy ochrony (MOSFET, Transistor, Diody)

### 🛠️ [MONTAZ.md](MONTAZ.md) - Przewodnik Montażu
Instrukcje krok po kroku:
- Przygotowanie breadboarda
- Montaż sensorów (DS18B20, DHT22)
- Podłączanie MOSFET/wiatraków
- Podłączanie Relay/nawilżacza
- Testy i checklist gotowości
## 🌐 Symulatory (bez fizycznego sprzętu!)

### Wokwi.com - Symulacja w przeglądarce
Folder `/wokwi` zawiera pełną symulację w [Wokwi.com](https://wokwi.com/):
- Rzeczywiste sensorami DS18B20 i DHT22
- Wizualne LED dla wszystkich sterów
- Serial Monitor z pełnym statusem
- **📖 INSTRUKCJA**: [INSTRUKCJA_POLACZENIA.md](wokwi/INSTRUKCJA_POLACZENIA.md) - jak podłączyć komponenty
- **Dokumentacja**: [README_WOKWI.md](wokwi/README_WOKWI.md)

### Simulator - Jeden plik do testowania
Folder `/simulator` zawiera `inkubator_simulator.ino`:
- Jeden plik - można wgrać bezpośrednio na ESP32
- Wbudowana symulacja czujników (bez rzeczywistych)
- Idealne do testowania logiki na biurku
- **Instrukcje**: [README_SIMULATOR.md](simulator/README_SIMULATOR.md)
## �🚀 Jak kompilować i wgrać

### Wymagania
- Platform IO IDE / VSCode + PlatformIO
- Sterowniki CH340 (dla USB)

### Kroki
1. Otwórz projekt w PlatformIO
2. `platformio.ini` zawiera już wszystkie biblioteki
3. Skompiluj: `PlatformIO: Build`
4. Wgraj: `PlatformIO: Upload`
5. Monitoruj: `PlatformIO: Open Serial Monitor` (115200 baud)

## 📡 Serial Monitor Output

```
┌─────────────────────────────────────────────────┐
│ DZIEŃ: 1/28                                     │
├─────────────────────────────────────────────────┤
│ 🌡️  TEMPERATURA:                                │
│    DS1:37.2°C  DS2:37.5°C  DS3:37.3°C           │
│    Średnia: 37.3°C  Cel: 37.5°C                 │
│ 💧 WILGOTNOŚĆ: 42.5%  Cel: 40%                  │
│ 🔥 GRZAŁKA: 200/255                             │
│ 💨 WIATRAKI: F1:ON F2:OFF F3:OFF                │
│ 💦 NAWILŻACZ: OFF                               │
│ 🔄 TACA: OBRACA                                 │
│ 💡 ŚWIATŁO: OFF                                 │
└─────────────────────────────────────────────────┘
```

## 🛠️ Do implementacji

- [ ] Logging do SPIFFS (CSV z danymi)
- [ ] Interfejs LVGL dla wyświetlacza ESP32-S3
- [ ] WebUI dla monitoringu WiFi
- [ ] Alarm wyłączenia prądu
- [ ] Przycisk Start/Stop inkubacji
- [ ] Kalibracja czujników

## 📝 Logika sterowania

### Temperatura (Grzałka)
- **Poniżej celu - HYSTEREZA**: Grzałka 100% (255)
- **W tolerancji**: Utrzymuj ostatni stan
- **Powyżej celu + HYSTEREZA**: Grzałka OFF (0)

### Wilgotność (Nawilżacz)
- **Poniżej celu - HYSTEREZA**: Włącz nawilżacz
- **Powyżej celu + HYSTEREZA**: Wyłącz nawilżacz

### Wiatraki
- **Fan1**: Zawsze włączony na stałą moc
- **Fan2**: Cykliczny (włącz 300ms → wyłącz 200ms)
- **Fan3**: Awaryjne chłodzenie (włącza się >38,5°C)

### Taca
- **Dni 1-20**: Obraca się co godzinę (10 sekund pracy)
- **Dni 21+**: Wyłączona (aby nie przerwać kluwania)

### Światło
- **Dni 1-20**: Wyłączone
- **Dni 21+**: Włączone (dla wspomagania wylęgu i pierwszych dni)

## 🔬 Testowanie

Aby przyspieszać dni dla testów, zmień w `main.cpp`:
```cpp
// Dla testów - 1 dzień = 60 sekund
int calculatedDay = (elapsedSeconds / 60) + 1;
```

## 📚 Biblioteki użyte

- `paulstoffregen/OneWire` - Protokół OneWire dla DS18B20
- `milesburton/DallasTemperature` - Czujniki Dallas (DS18B20)
- `adafruit/DHT sensor library` - Czujnik DHT22
- `adafruit/Adafruit Unified Sensor` - Wspólna biblioteka Adafruit

## 💡 Wskazówki

1. **Kalibracja temperaturki**: Porównaj odczyty 3x DS18B20 z termometrem referencyjnym
2. **PWM Grzałka**: Zamiast full 255, możesz używać PID dla mniejszej oscylacji
3. **Wilgotność**: DHT22 ma opóźnienie ~2s i niedokładność ±5%
4. **Wiatraki**: Upewnij się że ma wystarczającą przepustkowość powietrza

## ❓ FAQ

**P: Czujniki pokazują -999?**
A: Brak połączenia OneWire. Sprawdź rezystor pull-up (4,7kΩ zamiędzy VCC a GPIO4)

**P: DHT22 ciągle pokazuje NaN?**
A: Zbyt krótka linha lub szybkość SPI. DHT22 ma opóźnienie min. 2 sekundy między odczytami.

**P: Po ile godzin przesyłać tę na pierwszej godzinie?**
A: Na początku (dni 1-20) taca powinna się obracać co 1-2 godziny, mniej więcej 10 sekund per obrót.

## 📞 Autor

Projekt stworzony do automatyzacji inkubatora na ESP32.

---

**Powodzenia z wylęgiem! 🐣🐥**
