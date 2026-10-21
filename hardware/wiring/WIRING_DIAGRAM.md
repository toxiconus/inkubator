# Diagram okablowania - Fizyczne połączenia

## Przegląd przestrzenny

```
        ┌─────────────────────────────────────────────────────────┐
        │              ZASILACZ 12V DC (Mean Well)                │
        │              150W, 12.5A                                │
        │         [RED + WIRE] [BLACK - WIRE] [GND]               │
        └────────────┬─────────────────────────┬───────────────────┘
                     │                         │
                 +12V (RED)               GND (BLACK)
                     │                         │
        ╔════════════╩═════════════════════════╩════════════╗
        ║                                                    ║
        ║              GŁÓWNA SZYNKA GND (0V)              ║
        ║   (Aluminiowa lub miedziany drut, punkt zbioru)  ║
        ║                                                    ║
        ╚═════╦═════╦═════╦═════╦═════╦═════╦═════╦═════╝
              ║     ║     ║     ║     ║     ║     ║
            WSZYSTKIE MASY WSZYSTKICH MODUŁÓW TUTAJ!
```

---

## KROK 1: Połączenia zasilacza (12V, 5V logic power)

```
┌─ ZASILACZ Mean Well LRS-150-12 ─┐
│  [+12V out] [GND out]           │
└────┬───────────┬────────────────┘
     │           │
  +12V         GND
     │           │
     ├──[Bezp. 15A]──┐
     │               │
     │         ┌─────┴─────┐
     │         │  SZYNKA  │
     │         │   GND    │
     │         └─────┬─────┘
     │               │
     │         ┌─────┴────────────────────────────┐
     │         │  LM2596-V3 (12V → 5V)            │
     │         │  ┌─────────────────────┐         │
     │         │  │                     │         │
     └─────────┼──┤ IN+ (12V)           │         │
             │ │  IN- (GND) ◄──────────┼─────┐   │
             │ │                       │     │   │
             │ │  OUT+ (5V) ───┐       │     │   │
             │ │  OUT- (GND) ◄─┼───────┘     │   │
             │ │                │            │   │
             │ └────────────────┤────────────┘   │
             │                  │                 │
             │            [C100µF tan]           │
             │                  │                 │
             │                  │                 │
             │         +5V ─────┼─────────┐       │
             │                  │         │       │
             │                GND ────────┴───────┴──► SZYNKA GND
             │
             └──► +12V szynka (dla grzałek, wentylator, etc.)
```

---

## KROK 2: ESP32-S3 N8R2 (Zasilanie)

```
                    ┌──────────────────────────────┐
                    │     ESP32-S3 N8R2 (44-pin)   │
                    │                              │
   +5V (LM2596) ────┤ PIN 2:  3V3 (VDD)           │
   (z kondensatorem)│ PIN 3:  GND                 │
                    │ ...                          │
                    │ PIN 14: GND                 │
                    │ PIN 15: GND                 │
                    │ PIN 16: GND                 │
                    │ PIN 17: GND                 │
                    │                              │
                    │ PIN 1:  GPIO1 (Heater PWM)  │
                    │ PIN 2:  GPIO2 (Heater PWM)  │
                    │ ...                          │
   GND ─────────────┤ (wszystkie PIN GND)         │
   (wspólna masa)   │                              │
                    └──────────────────────────────┘
                          │
                    [C100µF elektrolit + C10µF film]
                    (między +5V a GND)
```

---

## KROK 3: Moduł MOSFET (Grzałki - 12V PWM)

```
+12V ─────────┬──────────────────┬──────────────────┬──────────────────┐
              │                  │                  │                  │
         [MOSFET Ch1]       [MOSFET Ch2]       [MOSFET Ch3]       [MOSFET Ch4]
         Gate ← GPIO1       Gate ← GPIO2       Gate ← GPIO3       Gate ← GPIO4
              │                  │                  │                  │
         [R10kΩ]             [R10kΩ]             [R10kΩ]             [R10kΩ]
         (pull-down)         (pull-down)         (pull-down)         (pull-down)
              │                  │                  │                  │
         [C10µF]             [C10µF]             [C10µF]             [C10µF]
         (filtr.)            (filtr.)            (filtr.)            (filtr.)
              │                  │                  │                  │
         ┌────▼───────────────────▼───────────────────▼───────────────────▼────┐
         │                                                                       │
         │  MOSFET Module (jedynie 2 kanały, ale szeregowo = 4 wyjścia)        │
         │                                                                       │
         │  +12V (VDD) ◄─────┐                                                 │
         │  GND        ◄──────┴─────────────► GND szynka                        │
         │                                                                       │
         │  IN1 ◄─ GPIO1                                                       │
         │  IN2 ◄─ GPIO2                                                       │
         │                                                                       │
         │  OUT1 ──► [GRZAŁKA 1] (lub 1+2 szeregowo)                          │
         │  OUT2 ──► [GRZAŁKA 2] (lub 3+4 szeregowo)                          │
         │                                                                       │
         └───────────────────────────────────────────────────────────────────────┘
                         │                         │
                    [HE-24W]                  [HE-24W]
                    (lub 2x)                  (lub 2x)
                         │                         │
                         └──────────┬──────────────┘
                                    │
                                   GND
```

---

## KROK 4: Moduł Relay (Wentylator 12V)

```
+5V (z LM2596) ────────┬────────────────────────┐
                       │  Relay Module 5V       │
                       │  SRD-05VDC-SL-C       │
                       │                        │
  GPIO5 ──────────────┤ IN (trigger)           │
                       │ VCC ◄──┐              │
  GND ────────────────┤ GND    │              │
                       │        │              │
                       │  ┌─────┴──┐           │
                       │  NO (Normally Open)   │
                       │  NC (Normally Closed) │
                       │  COM (Common)         │
                       │                        │
                       └────┬───────┬───────────┘
                            │       │
                     Relay kontakty:
                            │       │
                      +12V ─┤       ├─ GND
                            │       │
                        [Wiatrak 12V]
                            │       │
                           GND    GND
```

---

## KROK 5: Czujniki temperatury (DS18B20 - OneWire)

```
                    ┌──────────────────────────────┐
                    │  DS18B20 #1  (DQ pin)        │
                    │                              │
+5V ──[4.7kΩ]──┬───┤ DQ (data)                   │
               │    │ VDD ◄─ +5V                 │
         [C100nF]   │ GND ◄─ GND                 │
               │    │                              │
             GND    └──────────────────────────────┘
               │
               ├───────────────┬────────────────────┐
               │               │                    │
              GPIO6          DS#2 (opcja)         GND
             (ESP32)
               │
         [C100nF]
         (filtr.)
               │
              GND
              
Zamiast drutu, można też oba na tym samym pinie (GPIO6):

+5V ──[4.7kΩ]──┬──[DQ DS1]──┬──[DQ DS2]──┬─ GPIO6
               │             │             │
         [C100nF]       [C100nF]      (ESP32)
               │             │             │
              GND ◄──────────┴─────────────┴── GND
```

---

## KROK 6: Czujnik wilgoci (DHT22)

```
             ┌────────────────────────┐
             │      DHT22             │
             │  (4-pin sensor)        │
             │                        │
    +5V ────┤ VDD (+5V)              │
             │ NC (nie podłączaj)     │
    GPIO8 ──┤ DATA (pin 2 lub 3)     │
    +5V  ─┤ GND (przez R10kΩ)       │
             │ (pin 4)                │
             │                        │
             └───────────────┬────────┘
                             │
                        [R10kΩ]
                        pull-up
                             │
                        [C100nF]
                        (filtr.)
                             │
                            GND
```

---

## KROK 7: Czujnik poziomu wody (Capacitive 3-wire)

```
         ┌──────────────────────────────┐
         │  Capacitive Level Sensor     │
         │  3-wire module               │
         │                              │
  +5V ──┤ VCC (+5V)                   │
         │ GND ◄─ GND                 │
         │ OUT (analog)               │
         │                              │
         └────────────┬─────────────────┘
                      │
                  [C100nF]
                  (filtr.)
                      │
                   GPIO9 (ADC input)
                   ESP32-S3
                      │
         [R10kΩ]       │
         (pull-down)   │
           │           │
          GND ─────────┴──► GND
```

---

## KROK 8: Czujnik drzwi (Reed Switch + Magnes)

```
             ┌──────────────────┐
      +5V ──┤ R10kΩ (pull-up)  │
             │                  │
             ├──── GPIO10       │
             │  (ESP32-S3)      │
             │                  │
      GND ──┤ [Reed Switch]    │
             │ + Magnes         │
             │                  │
             └──────────────────┘

Albo z kondensatorem (debouncing):

      +5V ──[R10kΩ]─┬──── GPIO10
                     │
              [C100nF]
                     │
      GND ──────────┴──── GND
              (switch)
```

---

## KROK 9: Nawilrzacze ultrasoniczne (2x 5V PWM)

```
           ┌─────────────────────────────────┐
    +5V ──┤ MOSFET/Tranzystor #1            │
    GPIO11┤ Gate/Base ◄─ GPIO11             │
           │ Drain → Nawilrzacz 1 (+)       │
           │ Source → GND                    │
           │                                 │
           └─────────────────────────────────┘
                      │
                  [R10kΩ]
                  pull-down
                      │
                     GND

           ┌─────────────────────────────────┐
    +5V ──┤ MOSFET/Tranzystor #2            │
    GPIO12┤ Gate/Base ◄─ GPIO12             │
           │ Drain → Nawilrzacz 2 (+)       │
           │ Source → GND                    │
           │                                 │
           └─────────────────────────────────┘
                      │
                  [R10kΩ]
                  pull-down
                      │
                     GND


Nawilrzacze (równolegle do tej samej +5V):

          +5V
           │
        ┌──┴──┐
        │     │
    [NAV1] [NAV2]
    (5V)   (5V)
        │     │
        └──┬──┘
           │
    (sterowany PWM poprzez MOSFET)
           │
          GND
```

---

## KOMPLETNY SCHEMAT POŁĄCZEŃ - TABELA PRZEWODÓW

### Zasilanie (najgrubsze przewody!)

| Od | Do | Sygnał | Przewód | Długość | Uwagi |
|----|-------|--------|---------|---------|--------|
| Zasilacz +12V | Bezpiecznik | +12V | Czerwony 1.5mm² | ~30cm | MUSI być |
| Bezpiecznik | +12V szynka | +12V | Czerwony 1.5mm² | ~50cm | Grubieńkie! |
| +12V szynka | LM2596 IN+ | +12V | Czerwony 1.5mm² | ~30cm | Stabilność |
| +12V szynka | MOSFET VCC | +12V | Czerwony 1.5mm² | ~50cm | Dla grzałek |
| +12V szynka | Relay COM | +12V | Czerwony 1.5mm² | ~40cm | Dla wentylator |
| Zasilacz GND | GND szynka | GND | Czarny 1.5mm² | ~100cm | Główna masa! |

### Logika 5V

| Od | Do | Sygnał | Przewód | Długość | Uwagi |
|----|-------|--------|---------|---------|--------|
| LM2596 OUT+ | ESP32 VDD | +5V | Czerwony 0.75mm² | ~20cm | Z kondensatorami |
| GND szynka | ESP32 GND | GND | Czarny 0.75mm² | ~20cm | Wszystkie GND |
| LM2596 OUT+ | Relay VCC | +5V | Czerwony 0.5mm² | ~30cm | Zasilanie logiki |

### GPIO i sygnały

| Od | Do | Sygnał | Przewód | Długość | Uwagi |
|----|-------|--------|---------|---------|--------|
| GPIO1 | MOSFET IN1 | PWM | Niebieski 0.5mm² | ~30cm | Grzałka 1 |
| GPIO2 | MOSFET IN2 | PWM | Niebieski 0.5mm² | ~30cm | Grzałka 2 |
| GPIO5 | Relay IN | Digital | Niebieski 0.5mm² | ~40cm | Wentylator |
| GPIO6 | DS18B20 DQ | OneWire | Zielony 0.5mm² | ~50cm | Czujnik temp |
| GPIO8 | DHT22 Data | DHT | Fioletowy 0.5mm² | ~40cm | Wilgotność |
| GPIO9 | Water Sensor | ADC | Pomarańczowy 0.5mm² | ~60cm | Poziom wody |
| GPIO10 | Reed Switch | Digital | Żółty 0.5mm² | ~80cm | Drzwi |
| GPIO11 | Nawilrzacz 1 | PWM | Brązowy 0.5mm² | ~50cm | Nawilrzacz 1 |
| GPIO12 | Nawilrzacz 2 | PWM | Różowy 0.5mm² | ~50cm | Nawilrzacz 2 |

---

## 🔴 NAJCZĘSTSZE BŁĘDY OKABLOWANIA

```
❌ BŁĄD 1: Słaba masa (GND)
   └─ Symptomy: Czujniki "szaleją", ESP32 się resetuje
   └─ Rozwiązanie: Upewnij się że WSZYSTKIE GND są na tej samej szynie

❌ BŁĄD 2: Przewody zasilania za cienkie
   └─ Symptomy: Napięcie 12V spada do 8V pod obciążeniem
   └─ Rozwiązanie: Użyj min 1.5mm² dla 12V mocy

❌ BŁĄD 3: Brak kondensatorów
   └─ Symptomy: ESP32 się zawiesza, czujniki mają anomalnie
   └─ Rozwiązanie: MUSZĄ być! (100µF na LM2596, 100nF na czujnikach)

❌ BŁĄD 4: Pull-up/pull-down w złej stronie
   └─ Symptomy: GPIO zawsze wysoki lub zawsze niski
   └─ Rozwiązanie: Pull-down = do GND, Pull-up = do +5V

❌ BŁĄD 5: MOSFET Gate bez pull-down
   └─ Symptomy: Grzałki się włączają losowo
   └─ Rozwiązanie: 10kΩ pull-down na każdy MOSFET Gate
```

---

## 📐 KOLEJNOŚĆ MONTAŻU (KOLEJNIE!)

1. **Zasilanie** (Mean Well + Bezpiecznik + LM2596) - wszystko testować multimetrem
2. **ESP32-S3** (zasilanie tylko, bez GPIO)
3. **Grzałki** (MOSFET + Pull-down + obciążenie)
4. **Relay** (Wentylator 12V)
5. **Czujniki** (DS18B20, DHT22, poziom)
6. **Przełączniki** (Drzwi)
7. **Nawilrzacze** (na koniec)

---

**REGUŁA ZŁOTĄ**: Zawsze testuj **każdy krok** zanim przejdziesz do następnego!

**Data**: 2026-05-16  
**Wersja**: 1.0
