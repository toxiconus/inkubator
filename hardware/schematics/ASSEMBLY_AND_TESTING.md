# Wytyczne montażu i testowania - Schemat elektryczny

## Procedura montażu krok po kroku

### Etap 1: Przygotowanie zasilania (KRYTYCZNE)

#### 1.1 Zasilacz Mean Well LRS-150-12
```
[ ] Wyłączyć zasilacz główny
[ ] Skonfigurować Mean Well LRS-150-12 na 12V (standardowe)
[ ] Sprawdzić wyjścia (multimetrem):
    - +12V (czerwony): 12.0V ±0.5V
    - GND (czarny): 0V (reference)
    - Brak napięcia poza normalnym zakresem
```

#### 1.2 Przetwornica LM2596-V3 (12V → 5V)
```
Podłączenie:
  IN+   → +12V z zasilacza
  IN-   → GND (wspólna masa)
  OUT+  → +5V (do ESP32 + logika)
  OUT-  → GND (wspólna masa)

[ ] Wyregulować potencjometr na LM2596 do dokładnie 5.0V
    (mierzyć między OUT+ i OUT-)
[ ] Podłączyć kondensator wyjściowy 100µF (tant. 10V)
    - Dodatni (+) na OUT+
    - Ujemny (-) na OUT-
[ ] Sprawdzić brak zwarć (multimeter w trybie oporności)
```

#### 1.3 Masa - PUNKT KRYTYCZNY
```
[ ] WSZYSTKIE urządzenia muszą mieć wspólną masę (GND)
[ ] Połączyć osobno:
    1. GND zasilacza 12V
    2. GND LM2596 (IN-)
    3. GND LM2596 (OUT-)
    4. GND ESP32-S3
    5. GND wszystkich modułów (MOSFET, Relay, itp.)
    
[ ] Wszystkie GND połączyć w jednym punkcie (szynę GND)
[ ] Sprawdzić ciągłość (multimeter w trybie oporności):
    - Każdy pin GND ↔ szyna GND: < 0.5Ω
```

---

### Etap 2: Moduł MOSFET (Sterowanie grzałkami)

#### 2.1 Podłączenie mocy
```
Zasilanie z zasilacza 12V:
  [ ] +12V → pozytywne wejścia MOSFET
  [ ] GND → wspólna masa (juz połączona w Etapie 1.3)

Obciążenie (grzałki):
  [ ] Każdy kanał MOSFET → jedna lub dwie grzałki 24W
  [ ] Szeregowo czy równolegle?
      - Szeregowo (24W + 24W = 48W @ 12V ~4A): Zalecane
      - Równolegle (24W II 24W = 12W @ 12V ~2A): Optymalne
      
  REKOMENDACJA: Równolegle (A) lub szeregowo (B)
  
  Opcja A (Równolegle - niski prąd):
    +12V ──┬─[HE1]─┬── GND
           ├─[HE2]─┤
           ├─[HE3]─┤
           └─[HE4]─┘
  
  Opcja B (Szeregowo - wysoki prąd):
    +12V ──[HE1]──[HE2]──[HE3]──[HE4]── GND
```

#### 2.2 Sygnały sterujące (GPIO)
```
[ ] GPIO1 → MOSFET IN1 (Pull-down 10kΩ do GND dla bezpieczeństwa)
[ ] GPIO2 → MOSFET IN2 (Pull-down 10kΩ do GND dla bezpieczeństwa)
[ ] GPIO3 → MOSFET IN3 (Pull-down 10kΩ do GND dla bezpieczeństwa)
[ ] GPIO4 → MOSFET IN4 (Pull-down 10kΩ do GND dla bezpieczeństwa)

Zasilanie modułu MOSFET:
[ ] VCC → +5V z LM2596
[ ] GND → wspólna masa
```

#### 2.3 TEST
```
[ ] Bez obciążenia (grzałek):
    - Zasilanie ON
    - Multimeter mierzy napięcie na wyjściu każdego MOSFET (powinno być +12V)
    - GPIO wyłączony - MOSFET jest nieaktywny (HIGH Z, brak prądu)
    
[ ] Z obciążeniem:
    - GPIO1 = HIGH (255 PWM) → napięcie na grzałce 1 powinno być +12V
    - Mierzyć prąd (multimeter w serii z grzałką)
    - Bez PWM: ~2A (dla 24W @ 12V)
    - Z PWM 50%: ~1A
```

---

### Etap 3: Relay Module (Wentylator 12V)

#### 3.1 Podłączenie
```
Zasilanie modułu Relay:
[ ] VCC → +5V z LM2596
[ ] GND → wspólna masa
[ ] IN → GPIO5

Wentylator 12V (obciążenie):
[ ] Dodatni (+) → + kontakt Relay NO (Normally Open)
[ ] Ujemny (-) → GND
```

#### 3.2 TEST
```
[ ] GPIO5 = LOW (0) → Relay wyłączony (wentylator OFF)
    Multimeter: brak napięcia na wentylatorze
    
[ ] GPIO5 = HIGH (1) → Relay włączony (wentylator ON)
    Multimeter: +12V na wentylatorze
    Słychać pracę wentylator
```

---

### Etap 4: Czujniki temperatury (DS18B20 x 2)

#### 4.1 OneWire Pull-up
```
+5V ──[4.7kΩ]──┬──────────────┐
               │              │
           [DS1]          [DS2]
          (GPIO6)        (GPIO7)
           GND            GND

Lub (jeśli dwa czujniki na jednym GPIO):
+5V ──[4.7kΩ]────┬────┬─────┐
                 │    │     │
              [DS1]  [DS2]  │
               GND   GND   GND
```

#### 4.2 TEST (Serial Monitor)
```cpp
// Wgrać sketch OneWire, powinien znaleźć:
// Found 2 devices:
// Device 0: 28:XXXX:XXXX:XXXX
// Device 1: 28:XXXX:XXXX:XXXX
```

---

### Etap 5: Czujnik wilgoci (DHT22)

#### 5.1 Podłączenie
```
DHT22 (4-pinowy):
  Pin 1 (+5V) → +5V (przez rezystor seriowy 1kΩ - opcja)
  Pin 2 (GND) → GND (przez rezystor 10kΩ pull-up do +5V!)
  Pin 3 (NC)  → nie podłączać
  Pin 4 (GND) → GND

Prawidłowe podłączenie:
+5V ──[10kΩ]──┬─────────┐
              │         │
         GPIO8─┤ DHT22   │
              │  Pin 2  │
             GND────────┘
```

#### 5.2 TEST (Serial Monitor)
```
Humidity: 60.5%
Temperature: 37.2°C
```

---

### Etap 6: Czujnik poziomu płynu (ADC)

#### 6.1 Podłączenie
```
Capacitive Level Sensor (3-wire):
  +5V → +5V
  GND → GND
  OUT → GPIO9 (ADC1_CH8)
  
Multimeter - Calibracja:
  [ ] Zbiornik PUSTY: analogRead() = ~200
  [ ] Zbiornik PEŁNY: analogRead() = ~800
  [ ] Dla wody: zwykla kalibracja liniowa
```

#### 6.2 TEST (Serial Monitor)
```
Water Level ADC: 200  (pusty)
Water Level ADC: 500  (połowa)
Water Level ADC: 800  (pełny)
```

---

### Etap 7: Kontakt na drzwiach

#### 7.1 Podłączenie
```
+5V ──[10kΩ]──┬─────────────────┐
              │                 │
         GPIO10         [Przełącznik]
              │                 │
             GND────────────────┘

Pull-up wewnętrzny (INPUT_PULLUP)
  - Drzwi zamknięte: GPIO10 = LOW (0)
  - Drzwi otwarte: GPIO10 = HIGH (1) - przez przełącznik
```

---

### Etap 8: Nawilrzacze ultrasoniczne (5V x 2)

#### 8.1 Podłączenie
```
Nawilrzacz #1:
  +5V → +5V
  -/GND → GND
  Sygnał → GPIO11 (PWM/ON-OFF)

Nawilrzacz #2:
  +5V → +5V
  -/GND → GND
  Sygnał → GPIO12 (PWM/ON-OFF)

PWM Sterowanie:
  0 (OFF)   → Nawilrzacz wyłączony
  128 (50%) → Nawilrzacz na 50%
  255 (ON)  → Nawilrzacz na 100%
```

#### 8.2 TEST (Serial Monitor)
```
[ ] PWM 0:   Nawilrzacz OFF
[ ] PWM 128: Słaba para
[ ] PWM 255: Pełna para
```

---

### Etap 9: Integracja ESP32-S3

#### 9.1 Wgranie GPIO_PIN_CONFIG
```
[ ] Skopiować definicje z GPIO_PIN_CONFIG.md do main.cpp
[ ] Wgrać sketch z setup_gpio_pins()
[ ] Sprawdzić Serial Monitor - brak błędów inicjalizacji
```

#### 9.2 Pełny test systemu
```
[ ] Wszystkie GPIO inizjalizowane
[ ] Grzałki PWM: 0 → 255 (test każdej)
[ ] Wentylator: ON/OFF
[ ] Nawilrzacze: ON/OFF (PWM)
[ ] Czujniki: Odczyt danych
[ ] Drzwi: Przełącznik działa
[ ] Woda: Poziom zmienia się
```

---

## LISTA BEZPIECZEŃSTWA

### Przed zasilaniem:

- [ ] Wszystkie przewody GND połączone (ciągłość < 0.5Ω)
- [ ] Napięcie +12V mierzy się poprawnie (11.8V - 12.2V)
- [ ] Napięcie +5V mierzy się poprawnie (4.9V - 5.1V)
- [ ] Brak zwarciopadach między +12V i GND
- [ ] Brak zwarciopadach między +5V i GND
- [ ] Moduł MOSFET wyłączony (GPIO = LOW, MOSFET wyłączony)
- [ ] Relay wyłączone (GPIO5 = LOW, wentylator OFF)
- [ ] Brak obciążeń na MOSFET (lub test bez obciążenia)

### Podczas zasilania:

- [ ] Brak dymu z komponentów
- [ ] Brak zbyt gorących przewodów
- [ ] Zapachy: jeśli jest zapach spalenizny → WYŁĄCZYĆ!
- [ ] Temperatura LM2596: nie gorąca (powinno się wspinać ~50°C)

### Po zasileniu:

- [ ] Wgrać sketch testowy
- [ ] Sprawdzić każdą funkcję osobno
- [ ] Nie włączać wszystkich grzałek jednocześnie do 100% (prąd!)
- [ ] Etapy: 1 grzałka 50% → 2 grzałki 50% → wzrost stopniowy

---

## Tabela napięć punktu testowego

| Punkt pomiarowy | Oczekiwane napięcie | Zakres akceptowalny | Instrukcja |
|-----------------|-------------------|-------------------|-----------|
| +12V zasilacza | 12V | 11.8V - 12.2V | Multimeter + GND |
| +5V LM2596 | 5V | 4.9V - 5.1V | Multimeter + GND |
| MOSFET wyjście (bez PWM) | 12V | > 11.5V | Multimeter bez obciążenia |
| MOSFET wyjście (PWM 50%) | 6V (średnia) | 5-7V | Multimeter do masy |
| Relay wyjście (ON) | 12V | > 11.5V | Multimeter na wentylatorze |
| GPIO5 (Relay) | 3.3V (HIGH) | > 2.8V | Logiczny analizator / Multimeter |
| GPIO6/7/8/11/12 (Data pins) | 3.3V (HIGH) | > 2.8V | Logiczny analizator |

---

**UWAGA**: W razie jakichkolwiek wątpliwości lub dziwnych symptomów:
1. Wyłączyć zasilanie (ZAWSZE!)
2. Czekać 30 sekund
3. Sprawdzić wszystkie połączenia
4. Sprawdzić wzrokową kontrolę (nigdzie nie leży kondensator)
5. Ponownie uruchomić systemu ze zwiększoną ostrożnością

---

**Data**: 2026-05-16  
**Wersja**: 1.0
