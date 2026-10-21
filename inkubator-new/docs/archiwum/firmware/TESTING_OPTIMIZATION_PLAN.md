# Plan Testów i Optymalizacji - Strategia Sterowania Grzałkami

## 🎯 Cel testów

Określić:
1. **Ile grzałek naprawdę potrzeba?** (1 vs 2 vs wszystkie)
2. **Jak szybko grzać?** (ramp-up curve vs instant)
3. **Ile nawilżaczy?** (2 vs 1 vs dyfuzor)
4. **Czy grzałka 220V 36°C byłaby lepsza?** (stała vs PWM)
5. **Ile energii to kosztuje?** (optimization)

---

## FAZA 1: Baseline Testing (Tydzień 1)

### Test 1.1: Tylko 1 grzałka @ 50% PWM

```
PROCEDURA:
┌─────────────────────────────────────────────────────────┐
│ [ ] Przygotuj kod: PIN_HEATER_1 = 128 (50%)            │
│ [ ] PIN_HEATER_2 = 0 (wyłączona)                        │
│ [ ] Nawilrzacze wyłączone (tylko mierzysz temp)         │
│ [ ] Włącz system (t=0)                                  │
│ [ ] Obserwuj co 5 minut przez 2 godziny                 │
│ [ ] Zapisz do CSV (automatycznie)                       │
│ [ ] Czekaj na stabilizację (kolejne 4 godziny)         │
└─────────────────────────────────────────────────────────┘

SZUKAJ W DANYCH:
┌─────────────────────────────────────────────────────────┐
│ 1. Czas do osiągnięcia 37.5°C (np. 45 minut?)          │
│ 2. Temperatura stabilna ±0.5°C?                         │
│ 3. Czy potrzebny PWM adjustment?                        │
│ 4. Czy 1 grzałka wystarczy?                            │
│                                                          │
│ WYNIK CSV (Excel):                                      │
│ - Narysuj wykres: Czas [min] vs Temperatura [°C]       │
│ - Szukaj trendu: czy rośnie liniowo? logarytmicznie?   │
│ - Szukaj plateau: gdzie się stabilizuje?               │
└─────────────────────────────────────────────────────────┘

PRZYJĘTE KRYTERIA:
✅ Dobrze  → Osiąga 37.5°C w 30-60 min, stabilny ±0.5°C
⚠️  Średnio → Osiąga w >90 min, lub fluktuuje ±1°C
❌ Źle    → Nie osiąga lub za dużo fluktuacji
```

### Test 1.2: 1 grzałka @ 100% PWM

```
PROCEDURA (identyczna):
┌─────────────────────────────────────────────────────────┐
│ PIN_HEATER_1 = 255 (100%)                              │
│ PIN_HEATER_2 = 0                                        │
└─────────────────────────────────────────────────────────┘

PORÓWNANIE Z 1.1:
- Czy szybciej osiąga cel? (ile minut szybciej?)
- Czy bardziej fluktuuje?
- Czy przegrzewa się powyżej 38°C?

DECYZJA:
├─ Jeśli 50% wystarczy → używaj 50% (tanio!)
├─ Jeśli potrzebny 100% → może być problem (termika?)
└─ Jeśli oscyluje gwałtownie → potrzebny PID (skomplikowane)
```

### Test 1.3: Obie grzałki @ 50% PWM

```
PROCEDURA:
┌─────────────────────────────────────────────────────────┐
│ PIN_HEATER_1 = 128 (50%)                               │
│ PIN_HEATER_2 = 128 (50%)                               │
└─────────────────────────────────────────────────────────┘

PORÓWNANIE Z 1.1 I 1.2:
- Czy razem @ 50% = szybciej niż sam @ 100%?
- Czy bardziej stabilne?
- Co jest najoptymalniejsze?

TABELA WYNIKÓW:
┌─────────────────┬──────────┬──────────┬────────────┐
│ Config          │ Czas 37°C│ Stability│ Power (W)  │
├─────────────────┼──────────┼──────────┼────────────┤
│ 1 grzałka 50%   │ 45 min   │ ±0.3°C   │ ~12W       │
│ 1 grzałka 100%  │ 25 min   │ ±0.8°C   │ ~24W       │
│ 2 grzałki 50%   │ 28 min   │ ±0.4°C   │ ~24W       │
│ 2 grzałki 100%  │ 18 min   │ ±1.5°C   │ ~48W       │
└─────────────────┴──────────┴──────────┴────────────┘

REKOMENDACJA:
└─ Najlepszy balans = 2 grzałki @ 50% PWM
   (szybko, stabilnie, rozsądna moc)
```

---

## FAZA 2: Ramp-Up Curve Testing (Tydzień 2)

### Test 2.1: Nagłe włączenie (instant ON)

```
PROCEDURA:
┌─────────────────────────────────────────────────────────┐
│ [ ] Kod: PIN_HEATER = 255 na start (100% natychmiast)  │
│ [ ] Obserwuj gwałtowny wzrost temperatury              │
│ [ ] Czy przegrzewa się powyżej 38°C?                   │
│ [ ] Jak długo do powrotu do 37.5°C?                    │
└─────────────────────────────────────────────────────────┘

PROBLEM:
- Rapid heating może "zszokiem" zaburzać embriom
- Oscylacje: 36°C → 39°C → 36°C (źle!)
```

### Test 2.2: RAMP-UP LINEAR (60 minut)

```
PROCEDURA:
┌─────────────────────────────────────────────────────────┐
│ [ ] Kod: PWM rosnący liniowo od 0 do 255 przez 60 min  │
│ [ ] Formula: pwm = (time_elapsed / 3600000) * 255      │
│ [ ] Po 60 min: maintain 37.5°C za pomocą PID           │
└─────────────────────────────────────────────────────────┘

ZALETY:
✅ Łagodne zwiększanie temperatury
✅ Mniej stresu termicznego dla jaj
✅ Bardziej stabilne utrzymanie
✅ Naturalnie (jak w naturze - samo się grzeje)

PORÓWNANIE 2.1 vs 2.2:
- Czy embriom preferuje ramp-up?
- Czy fluktuacje są mniejsze?
- Długoterminowa stabilność?
```

### Test 2.3: EKSPOTENCJALNY ramp-up (szybciej na początek)

```
PROCEDURA:
┌─────────────────────────────────────────────────────────┐
│ pwm = 255 * (1 - exp(-t / 900000))                      │
│       ↑ Szybko na początek, asymptotycznie do 255       │
│ Czas do 95% mocy: ~30 minut (zamiast 60)               │
└─────────────────────────────────────────────────────────┘

BALANS:
- Szybciej niż linear (ponad 2x)
- Nadal łagodny (nie instant shock)
- Może być kompromis?
```

---

## FAZA 3: Nawilrzacze - Optymalizacja (Tydzień 3)

### Test 3.1: Obie nawilrzacze na 100%

```
PROCEDURA:
┌─────────────────────────────────────────────────────────┐
│ [ ] Włącz oba nawilrzacze (PIN_HUM_1 = 255, HUM_2 = 255)│
│ [ ] Obserwuj wilgotność (powinna szybko rosnąć)        │
│ [ ] Ile czasu potrzeba do osiągnięcia 60%?             │
│ [ ] Czy przegrzewa się powyżej 70%?                    │
│ [ ] Woda czy para? (obserwacja wzrokowa)               │
└─────────────────────────────────────────────────────────┘

SZUKAJ:
- Czas narastania wilgotności (np. 15 minut?)
- Czy stabilne czy skacze?
```

### Test 3.2: Tylko 1 nawilrzacz

```
PROCEDURA:
┌─────────────────────────────────────────────────────────┐
│ [ ] Włącz HUM_1 (PIN_HUM_2 = 0)                         │
│ [ ] Porównaj z testem 3.1                              │
│ [ ] Czy ciągle wystarczająco szybki?                   │
└─────────────────────────────────────────────────────────┘

WYNIK:
├─ Jeśli "TAK" → oszczędzaj energię, użyj 1
└─ Jeśli "NIE" → potrzebne oba
```

### Test 3.3: PWM (pulsacyjnie) vs ON/OFF

```
PROCEDURA:
┌─────────────────────────────────────────────────────────┐
│ OPCJA A: ON/OFF cycle (np. 60s ON, 300s OFF)          │
│         └─ Nawilrzacz włącz/wyłącz cyklicznie          │
│                                                         │
│ OPCJA B: PWM 50% (128 zamiast 255)                      │
│         └─ Nawilrzacz cały czas ale na pół mocy        │
│                                                         │
│ Zmierz: Średnia wilgotność, stabilność, hałas         │
└─────────────────────────────────────────────────────────┘

ZAZWYCZAJ:
- ON/OFF jest bardziej niezawodny (nie traci efektywności w czasie)
- PWM może być bardziej precyzyjny ale ultradźwiękowy słabnie
```

---

## FAZA 4: Grzałka 220V (nowa idea!)

### Analiza: PWM vs Grzałka 220V 36°C

```
┌────────────────────────────────────────────────────────────────┐
│ GRZAŁKA PWM (obecna konfiguracja)                              │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│ ZALETY:                                                        │
│ ✅ Elastyczność - można modulować PWM                          │
│ ✅ Precyzja - PID control                                      │
│ ✅ Szybkie przełączanie                                        │
│                                                                 │
│ WADY:                                                          │
│ ❌ Wymaga ciągłego monitorowania                               │
│ ❌ Oscylacje temperatury (±1-2°C)                              │
│ ❌ Zużycie MOSFET (ciepło w elektronice)                       │
│ ❌ Hazard: jeśli ESP się zawiesi → temp szybko spada           │
│                                                                 │
└────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────┐
│ GRZAŁKA AKWARYSTYCZNA 220V + TERMOSTAT (nowa idea)             │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│ ZALETY:                                                        │
│ ✅ STAŁA TEMPERATURA 36°C - termostat zajmuje się tym         │
│ ✅ Brak monitorowania! (elektromechanika robi pracę)          │
│ ✅ PASYWNY system - zawsze bezpieczny                          │
│ ✅ Niezawodny - 50 lat sprawdzony w akwarystyce               │
│ ✅ Mniej energii - termostat wyłącza gdy temp = 36°C          │
│ ✅ Brak oscylacji - ±0.1-0.5°C (bardzo stała!)               │
│ ✅ Bezpieczeństwo - jeśli ESP się zawiesi, grzałka dalej      │
│                                                                 │
│ WADY:                                                          │
│ ❌ Kosztuje ~150-300 PLN (vs 20-30 PLN grzałka 12V)          │
│ ❌ Wymaga instalacji elektryka (230V)                         │
│ ❌ Mniej elastyczności (nie możesz zmienić 36°C na 35°C)     │
│ ❌ Wciąż potrzebna PWM do nawilżaczy (ale mniejszy obciążenie)│
│                                                                 │
└────────────────────────────────────────────────────────────────┘

HYBRID SYSTEM (rekomendacja):
┌──────────────────────────────────────────────────────────────────┐
│ Grzałka 220V 36°C (termostat)      ← DO GRZANIA                  │
│     ↓                                                              │
│ Grzałka 12V PWM (backup emergency) ← BACKUP (jeśli termostat fail)│
│     ↓                                                              │
│ ESP32 (tylko monitoruje, nie grzeje) ← CZUJNIKI + NAWILRZACZE    │
│                                                                    │
│ LOGIKA:                                                            │
│ - Termostat utrzymuje 36°C ±0.5°C (pasywnie)                    │
│ - ESP mierzy i steruje nawilrzaczami (precyzja)                  │
│ - PWM backup włącza się jeśli temp < 35°C (redundancja)         │
│ - Bezpieczno i niezawodność!                                      │
└──────────────────────────────────────────────────────────────────┘
```

---

## Wybór: PWM vs 220V + Termostat

### Rekomendacja (zależnie od celu)

```
🔥 JEŚLI CHCESZ: Najprostszy, najbardziej niezawodny system
   └─ WYBIERZ: Grzałka 220V + termostat
      ├─ Koszt: +200 PLN
      ├─ Kompleksość: Minimalna
      └─ Efekt: Stabilna 36°C, brak wymysłu

🧪 JEŚLI CHCESZ: Naukowy projekt, eksperymentować
   └─ WYBIERZ: PWM + ESP (lub hybrid)
      ├─ Koszt: +50 PLN (samo hardware)
      ├─ Kompleksość: Średnia
      └─ Efekt: Precyzja, elastyczność, learning

⚡ JEŚLI CHCESZ: Best-of-both worlds
   └─ WYBIERZ: HYBRID
      ├─ Grzałka 220V 36°C (główna)
      ├─ PWM 12V (backup)
      ├─ ESP steruje tylko navil + monitoring
      └─ Efekt: Bezpieczny + naukowy
```

---

## HARMONOGRAM TESTÓW

```
TYDZIEŃ 1 (Faza 1 - Baseline):
│
├─ Poniedziałek (3 dni test):
│   ├─ Test 1.1: 1 grzałka 50%
│   ├─ Test 1.2: 1 grzałka 100%
│   └─ Test 1.3: 2 grzałki 50%
│
└─ WYNIK: Ile grzałek potrzeba?

TYDZIEŃ 2 (Faza 2 - Ramp-up):
│
├─ Poniedziałek-Wtorek:
│   ├─ Test 2.1: Instant ON
│   ├─ Test 2.2: Linear ramp (60 min)
│   └─ Test 2.3: Exponential ramp
│
└─ WYNIK: Najlepsza krzywa grzania?

TYDZIEŃ 3 (Faza 3 - Nawilrzacze):
│
├─ Poniedziałek-Wtorek:
│   ├─ Test 3.1: 2 nawilrzacze @ 100%
│   ├─ Test 3.2: 1 nawilrzacz
│   └─ Test 3.3: PWM vs ON/OFF
│
└─ WYNIK: Ile nawilżaczy? Jaki cykl?

TYDZIEŃ 4 (Faza 4 - Grzałka 220V):
│
├─ Poniedziałek:
│   └─ Decyzja: PWM vs 220V?
│
└─ WYNIK: Finalna architektura systemu
```

---

## ANALIZA DANYCH CSV

### Po każdym teście:

1. **Eksportuj CSV ze SPIFFS** (przez USB lub UART)
   ```
   /inkubator_log.csv
   ```

2. **Otwórz w Excelu**
   ```
   File → Open → inkubator_log.csv
   ```

3. **Utwórz wykresy**
   ```
   Insert → Chart (XY Scatter)
   X-axis: Time (minutes)
   Y-axis: Temperature (°C)
   ```

4. **Analizuj metryki**
   ```
   - Średnia temperatura
   - Min/Max
   - Standardowe odchylenie (stabilność)
   - Czas do osiągnięcia celu
   - Liczba oscylacji
   ```

### Ejemplo:
```
CSV Data:
Time(sec),Temp_Inside(C),PWM(%)
0,22.5,0
60,23.1,50
120,24.2,50
...
3600,37.4,50
3660,37.5,50
3720,37.6,48    ← PID adjustment

ANALIZA:
- Czas do 37°C: ~60 minut ✅
- Fluktuacja: ±0.2°C ✅
- PWM @ stabilizacja: ~48% ✅
→ 1 grzałka @ 50% wystarczy!
```

---

## FINALNA REKOMENDACJA (po testach)

```
SCENARIUSZ NAJLICZNIEJSZY:

┌─────────────────────────────────────────────────┐
│ Hardware:                                       │
│ ├─ 2x Grzałka 12V 24W (sterowane PWM)          │
│ ├─ 1x Nawilrzacz ultrasoniczny 5V              │
│ ├─ Grzałka 220V 36°C (OPCJONALNIE - backup)    │
│ └─ ESP32-S3 + czujniki                         │
│                                                 │
│ Algorytm:                                       │
│ ├─ Ramp-up linear 60 minut                     │
│ ├─ PID control 37.5°C ±0.5°C                   │
│ ├─ Nawilrzacz: ON 60s / OFF 300s               │
│ └─ Tace: ciągły obrót                          │
│                                                 │
│ Energia:                                        │
│ ├─ Średnia: ~40W (bez grzałki 220V)           │
│ ├─ Peak: ~60W (obie grzałki + nawilrzacz)     │
│ └─ 21 dni: ~30 kWh (~20 PLN)                   │
│                                                 │
└─────────────────────────────────────────────────┘
```

---

**Data**: 2026-05-16  
**Status**: Plan testów do realizacji  
**Przyszłe**: Wyniki testów → Finalna optymalizacja
