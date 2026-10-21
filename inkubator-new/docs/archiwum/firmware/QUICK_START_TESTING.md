# QUICK START - Wdrażanie Systemu Testowego

## Aktualizacja 2026-05-24 - stan aktualny

- Backend firmware: obsługa 4 kanałów grzałek PWM (CH0-CH3).
- UI WWW: ręczne sterowanie tylko CH0 i CH1.
- CH2 i CH3 działają w automacie po przypisaniu pinów `heater_pin_3` i `heater_pin_4`.
- Jeśli urządzenie nie widzi nowych pól po uploadfs, wykonaj factory reset (Preferences ma wyższy priorytet niż SPIFFS).

## ⚡ 5-MINUTOWY QUICKSTART

### Krok 1: Przygotuj kod

```
Skopiuj: main.cpp (z /firmware/src/)
         ↓
Wgraj na ESP32-S3 (PlatformIO)
         ↓
Obejrzyj Serial Monitor (upewnij się że inicjalizuje)
```

### Krok 2: Uruchom Test #1

```
Hardware:
├─ Tylko PIN_HEATER_1 = 128 (50% PWM)
├─ PIN_HEATER_2 = 0 (wyłączona)
├─ Nawilrzacze OFF
├─ Wentylator ON (zawsze)
└─ Drzwi zamknięte

Software:
├─ Kod już ma ustawione TARGET_TEMP = 37.5°C
├─ Ramp-up: liniowo 60 minut
└─ CSV logging: co 10 sekund

Uruchomienie:
[ ] Włącz zasilanie (+12V, +5V)
[ ] Otwórz Serial Monitor (115200 baud)
[ ] Czekaj... system się grzeje
[ ] Co 60 sekund widzisz status: [03 min] 🌡️  25.3°C ↑ (15% PWM)
[ ] Po 45 minutach: osiąga 37.5°C, PWM ustabilizuje się
[ ] Czekaj jeszcze 4 godziny (obserwuj płaskość wykresu)
```

### Krok 3: Wyciągnij dane

```
Po 6 godzinach testu:

Option A - USB Serial:
├─ Ctrl+C w Serial Monitor
└─ (CSV był zapisywany live w SPIFFS)

Option B - Odczyt pliku (gdy ESP wyłączy się):
├─ sudo minicom -D /dev/ttyUSB0 -b 115200
├─ help → fs_read /inkubator_log.csv
└─ Copy-paste do pliku
```

### Krok 4: Analiza (5 min)

```
1. Skopiuj CSV do Excela
2. Insert → Chart (XY Scatter)
3. Zaznacz kolumny: Time vs Temp_Inside
4. Obejrzyj wykres:
   ├─ Czy osiąga 37.5°C? ✅ / ❌
   ├─ Czy płaska po osiągnięciu? ✅ / ❌
   └─ Ile minut? (powinna ~45 min dla 50%)
5. Zapisz screenshot wykresu
```

---

## 📋 CHECKLIST PRE-TEST

### Sprzęt

```
[ ] ESP32-S3 podłączona (USB dla programowania)
[ ] +12V zasilacz testowy (nie koniecznie Mean Well, taki zwykły)
[ ] +5V przetwornica (LM2596) pracuje (check multimetr)
[ ] 1-2x Grzałka 12V 24W (podłączona przez MOSFET)
[ ] DS18B20 wewnątrz komory (w ciepłym miejscu, nie na powietrzu)
[ ] DHT22 (na czujniku poziomu wody - zbiornik)
[ ] Multimetr (do weryfikacji napięć)
[ ] Komora testowa (pudełko izolacyjne, np. styropian)
```

### Software

```
[ ] PlatformIO zainstalowany (VSCode)
[ ] main.cpp skopiowany
[ ] platformio.ini skonfigurowany (board = esp32-s3-devkitc-1)
[ ] Biblioteki:
    ├─ DHT sensor library
    ├─ OneWire
    ├─ DallasTemperature
    └─ (SPIFFS jest wbudowany w ESP32)
[ ] Kompilacja bez błędów (Ctrl+Shift+B)
[ ] Wgranie na ESP32 (Ctrl+U)
```

### Procedura

```
[ ] Zasilanie wyłączone
[ ] Sprawdź połączenia grzałek (czy na pewno na piny 1 i 2?)
[ ] Sprawdź DS18B20 (czy ma pull-up 4.7kΩ?)
[ ] Sprawdź DHT22 (czy ma pull-up 10kΩ?)
[ ] Zasilanie włączone (najpierw +12V, potem USB)
[ ] Serial Monitor: powinno pokazać: "✅ SPIFFS zainicjalizowany"
[ ] Pierwsze pomiary: temp ~20-25°C (jeśli wyższa - źle!)
```

---

## 📊 PODCZAS TESTU

### Co powinno być widoczne:

```
✅ POPRAWNIE:
═══════════════════════════════════════════════════════
✅ SPIFFS zainicjalizowany
✅ Wentylator włączony (zawsze ON)
✅ Tace ustawione na ciągły obrót
✅ System gotowy do testów

[000 min] 🌡️  In: 22.5°C ↑ (0% PWM) | 💧 45.0% [OFF] | 🚰 245
[001 min] 🌡️  In: 22.7°C ↑ (5% PWM) | 💧 45.1% [OFF] | 🚰 245
[002 min] 🌡️  In: 23.1°C ↑ (10% PWM) | 💧 45.1% [OFF] | 🚰 246
...
[045 min] 🌡️  In: 37.5°C → (48% PWM) | 💧 60.2% [NAV ON] | 🚰 500
[046 min] 🌡️  In: 37.4°C → (48% PWM) | 💧 59.8% [OFF] | 🚰 501
═══════════════════════════════════════════════════════

❌ PROBLEMY:
═══════════════════════════════════════════════════════
❌ "BŁĄD: Nie mogę zainicjować SPIFFS!"
   └─ Rozwiązanie: Prawdopodobnie SPIFFS nie ma miejsca
      Format: espressif.py erase_flash, dann wgraj ponownie

❌ "⚠️  DHT22 odczyt failed!"
   └─ Rozwiązanie: Sprawdź połączenie DHT22
      Możliwe pull-up za silny/słaby

❌ Temp nie rośnie (zawsze 22.5°C)
   └─ Rozwiązanie: 
      1. Grzałka naprawdę się grzeje? (dotknij ją - gorąca?)
      2. DS18B20 podłączona? (check multimetr między VDD a GND)
      3. PWM na MOSFET wygląda OK? (pin 1 lub 2 - sprawdzić oscyloskopem?)

❌ Temp przeskakuje 30-40°C (nie stabilna)
   └─ Rozwiązanie:
      1. Czujnik DS18B20 może być zły
      2. Drut OneWire za długi (szumy?)
      3. Brak pull-up na OneWire?
═══════════════════════════════════════════════════════
```

---

## ⏱️ TIMELINE TESTU (Dla każdej konfiguracji)

```
GODZINA 0 - START
├─ Włącz zasilanie
├─ Otwórz Serial Monitor
├─ Zanotuj początkową temperaturę
└─ Zrób screenshot (dla raportu)

GODZINA 0-1
├─ Obserwuj ramp-up (PWM powinno rosnąć liniowo)
├─ Co 10 minut: screenshot
└─ Notacja: Temp, PWM, Humidity

GODZINA 1-2
├─ Temp zbliża się do celu
├─ Obserwuj oscylacje
└─ Co 10 minut: notatka

GODZINA 2-6
├─ Temperatura powinna być stabilna (37.5°C ±0.5°C)
├─ PWM nie powinno się drastycznie zmieniać
├─ Co godzinę: screenshot (dla pewności)
└─ Jeśli stabilne: możesz się oddalić (obserwuj zdalnie jeśli WiFi)

GODZINA 6 - KONIEC
├─ Włączeń zasilanie
├─ Odczytaj CSV
├─ Przeprowadź analizę
└─ Zanotuj wyniki
```

---

## 🔧 TROUBLESHOOTING

### Problem: ESP32 się zawiesza

```
SYMPTOMY:
- Serial Monitor: nic się nie wyświetla
- ESP nie reaguje na komendy
- Wymuszony restart: naciśnij EN lub RESET

ROZWIĄZANIE:
1. Sprawdź zasilanie (multimeter 5V - powinno być)
2. Sprawdź USB kabel (czy nie uszkodzony?)
3. Wgraj ponownie: idf.py -p COM3 build flash monitor
4. Jeśli wciąż się zawiesza - może SPIFFS jest pełny?
   └─ Format: idf.py -p COM3 erase_flash
```

### Problem: Temperatury nie osiąga

```
DIAGNOSTYKA:
1. Włącz multimetr na Ohm (Ω)
   - Dotknij przewody grzałki
   - Powinno pokazać ~50Ω (dla 24W @ 12V)
   - Jeśli ∞ (OPEN) = grzałka spalona!
   - Jeśli 0Ω = grzałka zwarta!

2. Włącz multimetr na V DC
   - Zmierz napięcie na grzałce
   - Powinno być ~12V gdy PWM = 100%
   - Jeśli 0V = MOSFET nie działa!

3. Sprawdzaj liniowo:
   - Czy grzałka się grzeje?
   - Czy DS18B20 czyta poprawnie?
   - Czy MOSFET włączył się?
```

### Problem: Hałas w danych (skok temp co 30 sec)

```
SYMPTOMY:
- Temp: 37.4°C → 36.8°C → 37.5°C → 36.9°C (chaos!)
- Std Dev: 1.5°C (zły!)

PRZYCZYNA:
- Dwaj czujniki DS18B20 interferują
- Pull-up za słaby (szumy)
- Czujnik za daleko od grzałki (czytany przepływ powietrza)

ROZWIĄZANIE:
1. Zwiększ pull-up z 4.7kΩ na 2.2kΩ (bardziej agresywny)
2. Umieść DS18B20 tuż obok grzałki
3. Przełącz na jeden DS18B20 (jeśli masz 2)
4. Dodaj kondensator bypass 100nF na DS18B20
```

---

## 📈 POST-TEST ANALYSIS (Szybko!)

### Gdzie szukać wyniku?

```
1. Terminal → Open with code
   └─ Mapa: j:\inkubator\
   
2. Plik CSV znajduje się na ESP32 (SPIFFS)
   └─ Trzeba wyciągnąć przez UART lub USB
   
3. Polecenie (jeśli zinstalowałeś esptool):
   esptool.py -p COM3 read_flash 0x390000 0x100000 spiffs.bin
   
4. Rozpakuj SPIFFS:
   pymkspiffs.py -b 4096 -p 256 -s 1572864 -u inkubator_log.csv spiffs.bin
   
(Lub po prostu skopiuj CSV z Serial Monitor)
```

### Szybka analiza (1 min):

```
1. Otwórz Excel
2. Import CSV
3. Select: Time vs Temp
4. Insert → Chart
5. Czy wygląda OK? ✅ / ❌

JEŚLI OK:
└─ Przejdź do Test #1.2 (1 grzałka @ 100%)

JEŚLI NIE OK:
└─ Sprawdź troubleshooting powyżej
```

---

## 💾 PLIKI WYNIKOWE

```
Po każdym teście powinieneś mieć:

📁 Test Results/
│
├─ 📄 test_1_1_single_50pct.csv
│  └─ Raw data: czas, temp, PWM, wilgotność
│
├─ 📊 test_1_1_single_50pct_chart.png
│  └─ Wykres: Temperatura vs Czas (Excel)
│
├─ 📋 test_1_1_analysis.txt
│  ├─ Min: 22.5°C
│  ├─ Max: 37.8°C
│  ├─ Avg: 37.42°C
│  └─ Time to target: 45 min
│
└─ 🗒️  NOTES.md
   └─ Obserwacje, problemy, co zrobić dalej
```

---

## ✅ FINALIZACJA TESTU

```
KONIEC TESTU? Checklist:

[ ] Zasilanie wyłączone
[ ] Grzałka ostudziła się (wait 30 min)
[ ] Bezpiecznik sprawdzony (czy się nie przepalił?)
[ ] CSV wyciągnięty i backup (2x kopia!)
[ ] Wykresy zrobione (screenshot do raportu)
[ ] Analiza zapisana
[ ] Następny test przygotowany
[ ] Wszystkie pliki w folder: "Test Results"
```

---

**Data**: 2026-05-16  
**Wersja**: 1.0 - Quick Start  
**Czas trwania całego testu**: ~6 godzin na konfigurację
