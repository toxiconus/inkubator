# 🔧 Hybrid System - Implementacja & Troubleshooting

## KROK 1: Przygotuj Akwarium

### Zakup
```
✓ Zbiornik plastikowy: 15-20L (myj, wiaderko laboratoryjne)
✓ Grzałka akwarystyczna: 50-100W z termostatem (EHEIM/Aquael/Sera)
✓ Przewód 220V: 2x1.5mm² minimum (~5m)
✓ Wtyczka 220V + rozgałęzienie (jeśli potrzeba)
```

### Umiejscowienie
```
KOMORA JAJ (góra)
┌──────────────┐
│              │
│   🥚 Jaja    │  Izolacja styropian ~10cm
│   📊 DHT22   │
│   🔥 PWM     │
└──────────────┘
       ↑ ciepło
    przepływ ciepła
       ↑ para
┌──────────────┐
│   AKWARIUM   │  Woda 36°C
│   🐠 Woda    │  Grzałka 220V + termostat
│   🌡️ DS18B20│
│   🔌 Grzałka│
└──────────────┘
```

### Połączenia wody
```
❌ NIE: Akwarium wewnątrz komory (zaguli wszystko)
✓ TAK: Akwarium poniżej komory
     Przepływ: naturalny (ciepło w górę, para w górę)
```

---

## KROK 2: Elektryka Grzałki 220V

### BEZPIECZEŃSTWO
```
⚠️ UWAGA: To jest napięcie sieciowe (220V AC)!

✓ Zawsze:
  1. Wyłącz zasilanie PRZED podłączeniem
  2. Użyj przedłużacza z bezpiecznikiem (16A)
  3. Grzałka musi być w WODZIE (nigdy suchą!)
  4. Test: Włącz na 5 minut, czuj obudowę (powinna być ciepła, nie gorąca)

❌ Nigdy:
  - Nie podłączaj do głównego bezpiecznika
  - Nie zanurzaj całej grzałki (tylko element grzewczy)
  - Nie dotykaj 220V razem z ESP32!
```

### Schemat
```
           Zasilacz 220V
           ↓
      [Bezpiecznik 16A]
           ↓
     [Grzałka termostat]
           ↓
      [Przewód 2x1.5]
           ↓
      [Wtyczka]
           ↓
        AKWARIUM
```

### Izolacja
```
✓ Grzałka: w WODZIE (pełna ochrona IP65+)
✓ Przełącznik termostatu: zewnątrz wody
✓ Całe 220V: daleko od ESP32 i 12V!

Minimalny dystans: 30cm między 220V a 12V przewodami
```

---

## KROK 3: Połączenie DS18B20 Akwarium

### GPIO
```
GPIO7 (nowy): DS18B20 w akwarium
    └─ PIN:  Data
    └─ GND:  GND
    └─ +3.3V: +3.3V (poprzez 4.7kΩ)

Rezystor pull-up: 4.7kΩ między DATA a +3.3V
```

### Schemat
```
           +3.3V
            │
         [4.7kΩ]
            │
        ┌───┼───┐
        │   │   │
       GND Data +3.3V
             │
          GPIO7
             │
          ESP32

(Taki sam schemat dla DS18B20 jaj na GPIO6!)
```

### Rozmieszczenie
```
KABEL OneWire:
- Pin DATA: podłącz do GPIO7 (długi kabel OK, max 100m)
- PIN GND: podłącz do wspólnej GND
- PIN 3.3V: poprzez rezystor do +3.3V

ROBI SIĘ W AKWARIUM:
- Czujnik na dnie (lub na boku, ale w wodzie)
- Kabel przez szczelinę (izolacja!)
- Czubek czujnika w wodzie, złącze na zewnątrz
```

---

## KROK 4: Testowanie Akwarium Przed Testem

### TEST 0A: Grzałka sama
```
1. Napełnij akwarium 15L wody (temperaturą pokojową)
2. Włóż grzałkę 220V
3. Ustaw termostat na 36°C
4. Czekaj 30 minut
5. Zmierz multimetrem termometrycznym (jeśli masz)
   lub użyj DS18B20 bez ESP32:
   - Połącz DS18B20 do arduino/tester OneWire
   - Odczytaj temperaturę
   - Powinno być 36°C ± 0.5°C

JEŚLI NIE:
- Zbyt zimno? Zwiększ nastawę termostatu o 0.5°C
- Zbyt gorąco? Zmniejsz o 0.5°C
- Wcale się nie grzeje? Sprawdź czy wtyczka jest włączona!
```

### TEST 0B: DS18B20 sam
```
PRZED wgraniem kodu:
1. Połącz DS18B20 do GPIO6 (jaja) + GPIO7 (akwarium)
2. Zasilanie: +3.3V, GND, pull-up 4.7kΩ

PRÓBA ODCZYTU:
- Użyj Arduino IDE + Simple DS18B20 sketch
- Powinno wyświetlić: Device Address: 28:XXXXXXXXXXX
- Temperatura: 20-30°C (zależnie od otoczenia)

JEŚLI "CRC Error" lub "Device not found":
  → Rezystor pull-up jest zły (sprawdź 4.7kΩ!)
  → OneWire kabel jest uszkodzony
  → Zły GPIO
```

### TEST 0C: Oba razem
```
1. Wgraj main.cpp (wersja HYBRID)
2. Otwórz Serial Monitor (9600 baud)
3. Powinieneś zobaczyć:
   ✅ SPIFFS initialized
   ✅ GPIO pins configured
   ✅ DS18B20 ready
   ✅ System ready

4. Włóż DS18B20 jaj do naczynia z wodą 36°C
5. Obserwuj Serial:
   - "Aquarium: 36.0°C" (powinno być stałe)
   - "Eggs: 36.0°C" (bo ta sama temperatura)
   - "Humidity: XX%" (DHT22)

BŁĘDY:
- "-127°C" → Rezystor pull-up za duży lub źle podłączony
- "85°C" → Czujnik się nie zainicjalizował
- "85°C, 36°C" → Pierwszy DS18B20 ma problem
```

---

## KROK 5: Test 1 - Hybrid System (Pierwsza godzina)

### Setup
```
✓ Akwarium zalane wodą (15L)
✓ Grzałka 220V zainstalowana + termostat na 36°C
✓ Obie grzałki 12V PWM podłączone (ale niedostęp jeszcze)
✓ DS18B20 x2 w wodzie (jaja + akwarium)
✓ DHT22 w komorze (suchy, nie w wodzie!)
✓ Komora izolowana styropianem
✓ Serial Monitor gotowy
```

### Procedure
```
MINUTA 0:
  - Włącz zasilanie 12V (ESP32 się uruchomi)
  - Włącz zasilanie 220V (grzałka akw. włączy się)
  
MINUTA 1-5:
  - Serial Monitor powinien pokazać:
    🐠 Aq: 20°C (jeszcze zimne)
    🥚 Eg: 20°C
    💧 XX% Humidity
    (czujniki pracują!)

MINUTA 5-30:
  - Temperatura akwarium powinna rosnąć
  - PWM powinno rosnąć liniowo (0% → 50% w 60 min)
  - "🐠 Aq: 25°C" → "🐠 Aq: 30°C" → ... → "🐠 Aq: 36°C"

MINUTA 30:
  - Temperatura akwarium powinna być bliska 36°C
  - Temperatura jaj powinna być 35-36°C (z PWM ~30%)
  - Wilgotność powinna być 50-60% (rosnie powoli)

MINUTA 60:
  - 🐠 Aq: 36.0°C (stała!)
  - 🥚 Eg: 37.5°C (docelowa!)
  - 💧 60-70% (naturalnie z wody)
  - PWM: ~50% (równowaga)
  
✅ SUKCES: System działa!
```

### Anomalie
```
⚠️ Temp_Eggs > Temp_Aquarium o 2°C+
   → Może być artefakt czujnika
   → Sprawdź czy oba DS18B20 są w wodzie

⚠️ Temp_Eggs rośnie bardzo powoli
   → Izolacja komory może być słaba
   → Dodaj więcej styropianu

⚠️ PWM nie rośnie liniowo
   → Sprawdź serial output (mogą być błędy odczytu)
   → Reset ESP32 (przycisk RST)

⚠️ Wilgotność nie rośnie
   → Woda w akwarium może być zbyt zimna (DHT22 czuli na temp)
   → Lub DHT22 jest źle podłączony
   → Test: Zanurz DHT22 nad gorącą wodą (wilgotność powinna skoczyć)

⚠️ Grzałka 220V nie grzeje
   → Sprawdź czy termostat jest ustawiony (pokrętło)
   → Sprawdź czy wtyczka 220V jest wciśnięta
   → Sprawdź czy bezpiecznik nie spalił się
```

---

## KROK 6: Interpretacja CSV

### Co odczytywać
```
CSV:
Time(sec), Temp_Eggs(C), Temp_Aquarium(C), Humidity(%), Water(ADC), Door, PWM(%)
0,         20.2,        20.1,             45,          512,         0,    0
60,        20.5,        20.3,             46,          510,         0,    1
120,       21.0,        21.2,             48,          508,         0,    2
...
3600,      37.5,        36.0,             68,          500,         0,    50

TRENDY:
1. Temp_Eggs powinna rośnąć gładko (S-curve)
2. Temp_Aquarium powinna być ~36°C przez cały czas
3. PWM powinna rosnąć liniowo w pierwsze 60 minut
4. Humidity powinna rosnąć i stabilizować się ~60-70%

Różnica Temp_Eggs - Temp_Aquarium:
- 0-30 min: rosnąca (0°C → 1.5°C)
- 30-60 min: rosnąca (1.5°C → 1.5°C)
- 60+ min: stała ~1.5°C (równowaga PWM vs konwekcja)
```

### Problemy do wykrycia
```
❌ Temp_Eggs spada po 30 min
   → PWM maleje (błąd algorytmu?)
   → Sprawdź czy DS18B20 jaj jest w ciepłej przestrzeni

❌ Temp_Aquarium nie stabilizuje się na 36°C
   → Grzałka 220V ma problem
   → Termostat może być psuty
   → Test: Ustaw na 35°C, potem 37°C (sprawdź reaguje)

❌ Humidity oscyluje (68% → 50% → 68%)
   → DHT22 może być w miejscu z turbulencją
   → Przesuń czujnik dalej od przepływu powietrza

❌ PWM nie rośnie (zawsze 0%)
   → Pin PWM nie jest podłączony
   → Lub elecWrite() się nie wywoła
   → Sprawdź GPIO1 i GPIO2 są OUTPUT
```

---

## KROK 7: Backup Plan (Jeśli cos pójdzie nie tak)

### Grzałka 220V zawodna
```
PLAN B: Samo PWM 100% (powrót do starego systemu)
- Zmień w kodzie: HEATER_RAMP_UP_TIME * 2 (zamiast 50%, przejdź do 100%)
- Przejdź do testów PWM-only
```

### DS18B20 zawodny
```
PLAN B: Użyj DHT22 do pomiaru temperatury
- DHT22 ma sensor temperatury (±2°C)
- Mniej precyzyjny, ale działa
- Zmień w kodzie: temp_inside = dht.readTemperature()
```

### Całkowita awaria
```
PLAN C: Procesor do przodu 60 min
- Otwórz main.cpp
- Zmień: system_start_time -= 3600000 (cofnij czas)
- Kod będzie myśleć że ramp-up już się skończył
- Ustaw PWM ręcznie na 50% dla bieżacych testów
```

---

## ✅ Checklist przed testem

```
HARDWARE:
☐ Akwarium zalane (15L wody)
☐ Grzałka 220V w akwarium
☐ Termostat ustawiony na 36°C
☐ Wciśnięta wtyczka 220V
☐ Bezpiecznik 16A w rozgałęzieniu
☐ Grzałki 12V PWM podłączone (GPIO1, GPIO2)
☐ DS18B20 #1 (GPIO6) - w komorze jaj
☐ DS18B20 #2 (GPIO7) - w akwarium
☐ Rezystory pull-up 4.7kΩ (dwa!)
☐ DHT22 (GPIO8) - w komorze jaj
☐ Wentylator lub przepływomierz (opcja)
☐ Izolacja: komora owinięta styropianem ~10cm

SOFTWARE:
☐ main.cpp wgrany (HYBRID version)
☐ PlatformIO pokazuje: "Upload successful"
☐ Serial Monitor otwarty (9600 baud)
☐ Logi pojawiają się co 60 sekund

BEZPIECZEŃSTWO:
☐ 220V ODŁĄCZONY (do momentu startu)
☐ Multimetr w ręku (do pomiaru napięć)
☐ Osoba trzecia w pobliżu (na wypadek)
☐ Zgaszony ogień i inne zagrożenia

START:
☐ Włącz zasilanie 12V (ESP32 uruchomit się)
☐ Włącz zasilanie 220V (grzałka włączy się)
☐ Obserwuj Serial co 1 minutę (powinno pracować)
☐ Zrób zrzut ekranu pierwszych logów
```

---

## 🚀 Powodzenia!

Jeśli coś nie działa - najpierw sprawdzaj **od dołu**:
1. ✅ Czy zasilanie włączone? (wielometr)
2. ✅ Czy przewody podłączone? (sprawdź styki)
3. ✅ Czy kod się wgrał? (Serial Monitor)
4. ✅ Czy czujniki działają? (odczyta temperatury)
5. ✅ Czy logika się wykonuje? (PWM rośnie)

Nie zmieniaj kodu zanim się nie upewnisz że hardware pracuje!

---

**Tekst dla systemów hybrydowych - Wersja 1.0**  
**Data: 2026-05-16**  
**Status: Gotowy do wdrażania**
