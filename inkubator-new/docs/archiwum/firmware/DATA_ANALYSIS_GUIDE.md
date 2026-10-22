# Instrukcja Analizy Danych - Exportowanie i Wizualizacja

## 📥 Krok 1: Wyciągnięcie pliku CSV z ESP32-S3

### Opcja A: Przez USB-UART (najprostsze)

```
1. Podłącz ESP32-S3 do komputera (USB-C)

2. Otwórz VS Code → Extensions
   └─ Zainstaluj: "Espressif IDF"

3. Terminal → New Terminal
   └─ Wpisz: idf.py monitor
   
4. W Serial Monitorze będziesz widział logi:
   ├─ [RAMP-UP] 50.0% (25.3°C -> Target 37.5°C)
   ├─ [HUMIDIFIER] ON
   └─ itp...

5. Po skończeniu testu (24h):
   └─ Naciśnij: Ctrl+C (stop monitor)
```

### Opcja B: Przez SPIFFS Web Interface (jeśli ESP ma WiFi)

```
Kod to zrobić (dodaj do main.cpp):

#include <WebServer.h>

WebServer server(80);

void handle_download() {
  File file = SPIFFS.open("/inkubator_log.csv", "r");
  server.sendHeader("Content-Disposition", 
                    "attachment; filename=inkubator_log.csv");
  server.streamFile(file, "text/plain");
  file.close();
}

void setup() {
  // ... poprzedni kod
  server.on("/download", handle_download);
  server.begin();
}
```

### Opcja C: Manualnie odczytać zawartość SPIFFS

```
W terminal ESP:
$ spiffs_ls
  /inkubator_log.csv  (15 KB)

$ spiffs_read /inkubator_log.csv > log.csv
```

---

## 📊 Krok 2: Otwarcie w Excelu

### Plik CSV wygląda tak:
```
Time(sec),Temp_Inside(C),Temp_Outside(C),Humidity(%),Water_Level(ADC),Door_Open,Heater_PWM(%),Humidifier_On
0,22.5,20.1,45,150,0,0,0
10,22.6,20.1,45,150,0,10,0
20,22.8,20.1,45,152,0,20,0
...
3600,37.4,21.0,60,500,0,48,0
3610,37.5,21.0,61,501,0,48,1
```

### Import do Excela:

```
1. Otwórz Excel
2. Data → From File → From Text/CSV
3. Wybierz: inkubator_log.csv
4. Import Settings:
   ├─ Delimiter: Comma
   ├─ Encoding: UTF-8
   └─ Click [Load]
```

---

## 📈 Krok 3: Tworzenie Wykresów

### Wykres 1: Temperatura w czasie

```
A. Zaznacz dane:
   ├─ Kolumna A: Time(sec)
   └─ Kolumna B: Temp_Inside(C)

B. Insert → Chart
   ├─ Type: XY (Scatter)
   ├─ Style: Points + Lines
   └─ Next

C. Data Labels:
   ├─ Legend: Yes
   ├─ Primary Vertical Axis: "Temperature (°C)"
   └─ Primary Horizontal Axis: "Time (minutes)"

D. Finalize: OK

WYGLĄDAĆ BĘDZIE:
┌─────────────────────────────────────────────┐
│  Temperatura w czasie (Test 1.1 - 1 grzałka)│
│                                             │
│ 40 ┤                                  ✓✓✓  │
│    │                            ✓✓✓✓✓      │
│ 35 ┤                      ✓✓✓✓✓             │
│    │                ✓✓✓✓✓                   │
│ 30 ┤         ✓✓✓✓✓                         │
│    │    ✓✓✓✓✓                              │
│ 25 ┤ ✓✓✓                                   │
│    │                                        │
│ 20 ├────────────────────────────────────── │
│    0   10   20   30   40   50   60   70   80│
│           Time (minutes)                    │
└─────────────────────────────────────────────┘

ANALIZA:
- Czy linia rosnąca liniowo? (OK)
- Czy osiąga 37.5°C? (OK/FAIL)
- Czy płaska po osiągnięciu? (OK = stabilna)
```

### Wykres 2: PWM vs Temperatura (2 osie)

```
A. Zaznacz:
   ├─ Kolumna A: Time(sec)
   ├─ Kolumna B: Temp_Inside(C)
   └─ Kolumna G: Heater_PWM(%)

B. Insert → Chart
   ├─ Type: Column
   └─ Next

C. Series:
   ├─ Axis dla Temp: Primary Vertical
   ├─ Axis dla PWM: Secondary Vertical
   └─ Włącz dual axis

WYGLĄDAĆ BĘDZIE:
┌──────────────────────────────────────────────┐
│ PWM vs Temperatura                           │
│                                              │
│ °C    100%                                   │
│ 40 ┤   ▓▓▓▓▓▓▓▓ 100      Temp ———            │
│    │   ▓▓▓▓▓▓▓▓  ┐        PWM  ▓▓▓            │
│ 37.5├───•••••───────     ──────────          │
│    │   ▓▓▓▓▓  75 │                          │
│ 35 ┤   ▓▓▓▓    ┐ 50                         │
│    │   ▓▓     50 │                          │
│ 30 ┤   ▓        25                          │
│    │   ▓         │                          │
│ 20 ├────────────┘ 0 ────────────────────── │
│    0    10   20   30   40   50   60         │
│           Time (minutes)                     │
└──────────────────────────────────────────────┘

ANALIZA:
- Czy PWM rośnie gdy temp niiska?
- Czy PWM maleje gdy temp wysoka?
- Czy PID działa? (regulacja responsywna?)
```

### Wykres 3: Porównanie testów

```
PROCEDURA:
1. Stwórz 3 arkusze (Sheet1, Sheet2, Sheet3)
   ├─ Sheet1: Test 1.1 (1 grzałka 50%)
   ├─ Sheet2: Test 1.2 (1 grzałka 100%)
   └─ Sheet3: Test 1.3 (2 grzałki 50%)

2. W każdym: Tym samym wykres Temp vs Time

3. W osobnym arkuszu: COMPARISON
   ├─ Wklej wszystkie 3 wykresy
   ├─ Albo stwórz tabelę:
   │
   │  Config         │ Czas do 37°C │ Min Temp │ Max Temp │ Stability
   │  1 grzałka 50%  │ 45 min       │ 22.5°C   │ 37.8°C   │ ±0.3°C
   │  1 grzałka 100% │ 25 min       │ 22.5°C   │ 39.2°C   │ ±0.8°C
   │  2 grzałki 50%  │ 28 min       │ 22.5°C   │ 37.5°C   │ ±0.2°C
   │
   └─ Stąd będziesz widział porównanie!
```

---

## 🧮 Krok 4: Obliczenia Statystyczne (Excel Formulas)

### Średnia temperatura (podczas stabilizacji)

```
=AVERAGE(B1000:B1200)    ← gdy temp jest stabilna (np. po 60 min)
Wynik: 37.42°C
```

### Standardowe odchylenie (stabilność)

```
=STDEV(B1000:B1200)       ← jak duże fluktuacje?
Wynik: 0.23°C  (OK - stabilny!)
       0.85°C  (źle - oscyluje)
```

### Min i Max

```
=MIN(B:B)   ← najniższa temperatura
=MAX(B:B)   ← najwyższa temperatura

Wynik:
  MIN: 22.5°C (start)
  MAX: 37.8°C (peak)
```

### Czas do osiągnięcia 37.5°C

```
=MATCH(37.5, B:B, 1) * 10 / 60    ← w minutach
(MATCH находит pierwszy row > 37.5, * 10 bo dane co 10 sec, / 60 na minuty)

Wynik: 45 minut
```

---

## 💾 Krok 5: Eksport Wykresy (do raportu)

```
1. Prawy klik na wykresie
2. Save as Image
   ├─ Format: PNG (1920x1080)
   └─ Folder: C:\inkubator_tests\results\
   
3. Nazwij:
   ├─ Test_1_1_Single_Heater_50pct.png
   ├─ Test_1_2_Single_Heater_100pct.png
   └─ Comparison_All_Tests.png

4. Stwórz raport Word:
   ├─ Title: "Inkubator Testing Results - Week 1"
   ├─ Sekcje:
   │  ├─ Executive Summary
   │  ├─ Test Procedure
   │  ├─ Results & Graphs
   │  ├─ Analysis
   │  └─ Recommendations
   └─ Paste graphs tu
```

---

## 📋 TEMPLATE - Analiza poszczególnych testów

### Po każdym teście: FILL THIS OUT

```
┌─────────────────────────────────────────────────────────┐
│ TEST: 1.1 - Single Heater @ 50% PWM                    │
│ DATE: 2026-05-20                                        │
│ DURATION: 6 hours                                       │
├─────────────────────────────────────────────────────────┤
│                                                         │
│ DATA POINTS:                                            │
│ ├─ Samples: 2160 (co 10 sekund przez 6h)               │
│ ├─ CSV File: test_1_1_single_heater_50pct.csv          │
│ └─ Size: ~45 KB                                        │
│                                                         │
│ RESULTS:                                                │
│ ├─ Target Temp: 37.5°C                                 │
│ ├─ Time to Target: 45 minutes                          │
│ ├─ Min Temp: 22.5°C (start)                            │
│ ├─ Max Temp: 37.8°C                                    │
│ ├─ Avg Temp (stable): 37.42°C                          │
│ ├─ Std Dev: 0.23°C ✅ (stabilny)                      │
│ ├─ Oscillations: <0.5°C (minimalne)                    │
│ └─ Power Consumption: ~12W average                     │
│                                                         │
│ OBSERVATIONS:                                          │
│ ├─ ✅ Osiąga cel
│ ├─ ✅ Stabilny po osiągnięciu
│ ├─ ⚠️  Trochę wolniej niż oczekiwane
│ └─ ✅ PWM ustabilizował się na ~48%                    │
│                                                         │
│ DECISION:                                              │
│ ├─ 1 grzałka @ 50% wystarczy
│ ├─ Ale powolny ramp-up
│ └─ NEXT: Test 1.3 (2 grzałki) dla szybszego rogozu    │
│                                                         │
│ NOTES:                                                 │
│ └─ Komora miała drzwi zamknięte, brak wentylacji      │
│    natural (zawsze włączony fan)                       │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

## 🔧 Skrypt do automatycznej konwersji (Python)

```python
# convert_csv.py - jeśli chcesz bardziej zaawansowaną analizę

import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime

# Wczytaj CSV
df = pd.read_csv('inkubator_log.csv')

# Konwersja czasu na minuty
df['Time_min'] = df['Time(sec)'] / 60

# Statystyki
print("=== TEST RESULTS ===")
print(f"Duration: {df['Time(sec)'].max() / 60:.1f} minutes")
print(f"Temp Min: {df['Temp_Inside(C)'].min():.2f}°C")
print(f"Temp Max: {df['Temp_Inside(C)'].max():.2f}°C")
print(f"Temp Avg: {df['Temp_Inside(C)'].mean():.2f}°C")
print(f"Temp StdDev: {df['Temp_Inside(C)'].std():.2f}°C")

# Wykres
plt.figure(figsize=(12, 6))
plt.plot(df['Time_min'], df['Temp_Inside(C)'], label='Temperature')
plt.axhline(y=37.5, color='r', linestyle='--', label='Target 37.5°C')
plt.xlabel('Time (minutes)')
plt.ylabel('Temperature (°C)')
plt.legend()
plt.grid()
plt.savefig('temperature_graph.png', dpi=300)
plt.show()

print("\n✅ Graph saved as temperature_graph.png")
```

---

## 🎯 Co szukać w danych?

```
✅ DOBRE ZNAKI:
├─ Temperatura osiąga cel w rozsądnym czasie (30-60 min)
├─ Po osiągnięciu jest płaska ±0.5°C
├─ Std Dev < 0.5°C
├─ Brak gwałtownych skoków
└─ PWM stabilizuje się na jednej wartości

⚠️  OSTRZEŻENIA:
├─ Temperatura oscyluje ±1-2°C (źle dla embrionów!)
├─ Nigdy nie osiąga 37.5°C (zbyt mało mocy?)
├─ Przegrzewa się powyżej 38°C (ryzyko)
├─ PWM cały czas zmienia się (niestabilny PID?)
└─ Po 6h temp nagle spada (problem z czujnikiem?)

❌ PROBLEMY:
├─ Temperatura oscyluje 35-39°C cyklicznie
│  └─ Rozwiązanie: zmniejsz PID gains (hysteresis)
├─ Nigdy nie osiąga temperatury
│  └─ Rozwiązanie: więcej mocy (2 grzałki) lub dłuższy ramp-up
├─ Dzikie skoki temperatury
│  └─ Rozwiązanie: sprawdź czujnik DHT22 (może być zły)
└─ Temperatura zimna przez całą noc
   └─ Rozwiązanie: sprawdź czy grzałka się włączyła!
```

---

## 📧 RAPORT SZABLONOWY (Po testach)

```
TEMAT: Inkubator Testing Report - Week 1

TREŚĆ:

Hello,

Przeprowadziłem serie testów w celu optymalizacji systemu grzania
i nawilżania dla inkubatora jaj.

TESTY:
1. Single heater @ 50% PWM - 45 min do 37.5°C, stabilny
2. Single heater @ 100% PWM - 25 min ale oscyluje ±0.8°C
3. Dual heater @ 50% PWM - 28 min, bardzo stabilny

WNIOSKI:
- 2 grzałki @ 50% to najlepszy kompromis
- Liniowy ramp-up bardziej pewny niż instant ON
- 1 nawilrzacz wystarczy, ON/OFF cycle (60s ON / 300s OFF)

NASTĘPNY KROK:
- Testy z grzałką 220V + termostat (bardziej niezawodne?)
- Weryfikacja na rzeczywistych jajach (piskliaki = test!)

Attachments:
- test_results.xlsx (wszystkie wykresy)
- detailed_analysis.pdf

Regards,
Inkubator System
```

---

**Data**: 2026-05-16  
**Wersja**: 1.0 - Instrukcja analizy danych
