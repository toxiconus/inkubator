# Schemat Elektryczny Inkubatora - Wersja 1.1

## Przegląd ogólny

```
┌─────────────────┐
│ Zasilacz 12V DC │
│ (Mean Well      │
│ LRS-150-12)     │
│ 150W, 12.5A     │
└────────┬────────┘
         │
         ├──────────────────────────────┬──────────────────────────┐
         │                              │                          │
    ┌────▼─────┐               ┌────────▼────────┐        ┌────────▼────────┐
    │ MOSFET    │               │ LM2596-V3       │        │ Wiatrak 12V     │
    │ Driver    │               │ (12V → 5V)      │        │ + Power Relay   │
    │ 2x 15A    │               │ ~3A             │        │                 │
    └────┬─────┘               └────────┬────────┘        └────────┬────────┘
         │                              │                          │
    ┌────▼─────────────────────────────────┬────────────────┬───────▼────────┐
    │                                      │                │                │
┌───▼──────────┐    ┌──────────────┐  ┌───▼───┐      ┌──────▼──┐     ┌──────▼──────┐
│ Grzałki (x4) │    │ Nawilrzacze  │  │ Czujn.│      │ Seria   │     │ Pozostałe   │
│ 4x 24W 12V   │    │ (1-2 szt)    │  │ DS/   │      │ Czujnik │     │ czujniki    │
│              │    │ 5V, ~1A ea   │  │ Temp  │      │ Wilg.   │     │ (zwrotnica) │
└──────────────┘    └──────────────┘  └───────┘      └─────────┘     └─────────────┘
                            ↓
                    ┌─────────────────┐
                    │ Logika zasilania│
                    │ (FET driver     │
                    │ + transistory)  │
                    └─────────────────┘
                            ↓
                    ┌───────────────┐
                    │  ESP32-S3     │
                    │  N8R2         │
                    │               │
                    │ GPIO / I2C /  │
                    │ ADC / PWM     │
                    └───────────────┘
```

---

## Schemat połączeń detailowy

### Blok zasilania

```
12V DC (Mean Well)
  │
  ├─ +12V  ───────────────────┬─────────────────┬──────────────┐
  │                           │                 │              │
  │                      [MOSFET Module]  [Relay Module]  [Reserve]
  │                      (Grzałki)       (Wiatrak)
  │                           │                 │
  │                           ▼                 ▼
  │                       [Grzałki]        [Wiatrak 12V]
  │                       [Tace PWM]       (zawsze przy starcie)
  │
  └─ GND (0V) ───────────────────────────────────────────────┐
                                                              │
LM2596-V3 (12V → 5V):                                        │
  IN+:   +12V ◄────────────────────────────────────────────┐ │
  IN-:   GND  ◄─────────────────────────────────────────────┴─┤
  OUT+:  +5V  ──────────────────────┐                         │
  OUT-:  GND  ──────────────────────┼─────────────────────────┤
                                    │                         │
                         ┌──────────▼──────────┐               │
                         │   ESP32-S3 N8R2    │               │
                         │                    │               │
                         │ +5V (VDD)   ◄──────┘               │
                         │ GND (0V)    ◄───────────────────────┘
                         │                    │
                         │ GPIO pins...       │
                         └────────────────────┘
```

---

## Tabela pinów ESP32-S3 N8R2

### Zasilanie
| Pin | Funkcja | Napięcie | Uwagi |
|-----|---------|----------|-------|
| 2, 3 | +5V (VDD) | +5V | Z LM2596 (rekomendacja: C tantał 100µF) |
| 14, 15, 16, 17 | GND | 0V | Co najmniej 2 pinów GND |

### Sterowanie grzałkami (PWM via MOSFET Module)
| Pin | Funkcja | Typ | Przeznaczenie | Uwagi |
|-----|---------|-----|---------------|-------|
| 1 | GPIO1 | GPIO/PWM | Grzałka 1 (lub grupa) | PWM, do MOSFET IN1 |
| 2 | GPIO2 | GPIO/PWM | Grzałka 2 (lub grupa) | PWM, do MOSFET IN2 |
| 3 | GPIO3 | GPIO/PWM | Grzałka 3 (jeśli trzeba) | PWM |
| 4 | GPIO4 | GPIO/PWM | Grzałka 4 (jeśli trzeba) | PWM |

### Sterowanie wentilatorami
| Pin | Funkcja | Typ | Przeznaczenie | Uwagi |
|-----|---------|-----|---------------|-------|
| 5 | GPIO5 | GPIO | Wiatrak 12V (zawsze ON) | Sygnał do Relay Module (HIGH = ON) |
| 18 | GPIO18 | GPIO | Tace (zasilanie/PWM) | Sterowanie obrotem tac |

### Czujniki temperatury (DS18B20 - OneWire)
| Pin | Funkcja | Typ | Przeznaczenie | Uwagi |
|-----|---------|-----|---------------|-------|
| 6 | GPIO6 | OneWire | Czujnik DS18B20 #1 | + Pull-up 4.7kΩ do +3.3V |
| 7 | GPIO7 | OneWire | Czujnik DS18B20 #2 (opcja) | + Pull-up 4.7kΩ do +3.3V |

### Czujnik wilgoci (DHT11 - DHT protocol)
| Pin | Funkcja | Typ | Przeznaczenie | Uwagi |
|-----|---------|-----|---------------|-------|
| 8 | GPIO8 | DHT | DHT11 signal pin | + Pull-up 10kΩ do +3.3V |

### Czujnik poziomu płynu (analog)
| Pin | Funkcja | Typ | Przeznaczenie | Uwagi |
|-----|---------|-----|---------------|-------|
| 9 | GPIO9 | ADC | Czujnik poziomu płynu | ADC1_CH8 |

### Kontakt na drzwiach (opcja)
| Pin | Funkcja | Typ | Przeznaczenie | Uwagi |
|-----|---------|-----|---------------|-------|
| 10 | GPIO10 | GPIO/Input | Przełącznik drzwi | Pull-up wewnętrzny, LOW = otwarte |

### Nawilrzacze ultrasoniczne (2x 5V)
| Pin | Funkcja | Typ | Przeznaczenie | Uwagi |
|-----|---------|-----|---------------|-------|
| 11 | GPIO11 | GPIO | Nawilrzacz #1 trigger/PWM | MOSFET / Tranzystor do 5V |
| 12 | GPIO12 | GPIO | Nawilrzacz #2 (opcja) | MOSFET / Tranzystor do 5V |

---

## Schemat bloku mocy (Grzałki)

```
+12V ─────────────┬─────────────┬─────────────┬─────────────┐
                  │             │             │             │
              [R1-10kΩ]     [R2-10kΩ]     [R3-10kΩ]     [R4-10kΩ]
                  │             │             │             │
           ┌──────┴─────┐┌──────┴─────┐┌──────┴─────┐┌──────┴─────┐
           │  MOSFET1   ││  MOSFET2   ││  MOSFET3   ││  MOSFET4   │
           │  N-channel ││  N-channel ││  N-channel ││  N-channel │
           │  (15A)     ││  (15A)     ││  (15A)     ││  (15A)     │
           │            ││            ││            ││            │
           │  Gate←GPIO1 ││  Gate←GPIO2 ││  Gate←GPIO3 ││  Gate←GPIO4 │
           │            ││            ││            ││            │
           └──────┬─────┘└──────┬─────┘└──────┬─────┘└──────┬─────┘
                  │             │             │             │
             [HE-24W]       [HE-24W]       [HE-24W]       [HE-24W]
             (12V 24W)      (12V 24W)      (12V 24W)      (12V 24W)
                  │             │             │             │
                  └─────────────┴─────────────┴─────────────┘
                                    │
                                   GND
                                (0V common)
```

---

## Schemat wentylatorów i zaworu wody

```
+12V ─────────────────────────────┐
                                  │
                     [Relay Module]
                       In ← GPIO5  
                       VCC ← +5V
                       GND ← GND
                                  │
                     ┌────────────┴─────────────┐
                     │                          │
              [Wiatrak 12V]            [Zawór/Pompa wody]
              (zawsze ON                (sterowany GPIO18)
               przy starcie)
                     │                          │
                     └────────────┬─────────────┘
                                  │
                                 GND
```

---

## Schemat czujników

### Czujniki temperatury (DS18B20 - OneWire)
```
+3.3V ────[4.7kΩ Pull-up]─────┬─────────────────────────┐
                              │                         │
                      ┌───────┴──────┐         ┌───────┴──────┐
                      │ DS18B20 #1   │         │ DS18B20 #2   │
                      │              │         │              │
                      │ VDD ← +5V    │         │ VDD ← +5V    │
                      │ DQ ← GPIO6   │         │ DQ ← GPIO7   │
                      │ GND ← GND    │         │ GND ← GND    │
                      └──────────────┘         └──────────────┘
                              │                         │
                              └──────────┬──────────────┘
                                         │
                                        GND
```

### Czujnik wilgoci (DHT11)
```
+3.3V ────[10kΩ Pull-up]────────┬────────────┐
                               │            │
                      ┌────────┴──────┐     │
                      │ DHT11         │     │
                      │               │     │
                      │ VDD ← +5V     │     │
                      │ GND ← GND     │     │
                      │ DATA ← GPIO8  │◄────┘
                      └───────────────┘
                             │
                            GND
```

### Czujnik poziomu płynu (Capacitive/Resistive)
```
+5V ──[Resistor]──┬──────────┐
                  │          │
            [Sensor]     [R Pull-down]
             (float)     (10kΩ)
                  │          │
                  └────┬─────┘
                       │
                     GPIO9 (ADC input)
                       │
                      GND
```

---

## Kondensatory - czujniki i linie sygnałowe

| Obwód | Kondensator | Miejsce montażu | Cel |
|------|-------------|-----------------|-----|
| DS18B20 (każdy czujnik) | 100nF ceramiczny (X7R) | Między VDD i GND, możliwie blisko obudowy czujnika | Odfiltrowanie zakłóceń i stabilny odczyt 1-Wire |
| DHT11 | 100nF + 1uF | Równolegle między VDD i GND przy module DHT11 | Tłumienie pików prądowych i stabilizacja zasilania |
| BMP180 (I2C) | 100nF | Między 3.3V i GND przy module | Ograniczenie zakłóceń magistrali I2C |
| Wejścia GPIO z długą wiązką (>30 cm) | 1nF-4.7nF (opcjonalnie) | Od sygnału do GND przy ESP32 | Redukcja zakłóceń impulsowych (stosować tylko przy realnych problemach) |

### Notatka praktyczna
- Pull-upy sygnałowe czujników podłączaj do 3.3V, nie do 5V.
- Przy przewodach czujników prowadzonych obok linii mocy 12V rozważ skrętkę sygnał + GND.
- Dla sekcji mocy zostaw osobne kondensatory elektrolityczne przy obciążeniach (grzałki, dyfuzor).

---

## Schemat nawilrzaczy ultradźwiękowych (5V)

```
+5V ─────┬─────────────────┬─────────────────┐
         │                 │                 │
    [Tranzystor/MOSFET #1] [Tranzystor/MOSFET #2]
    Gate/Base ← GPIO11     Gate/Base ← GPIO12
         │                 │
    [Nawilrzacz 1]    [Nawilrzacz 2]
    (5V, ~1A ea.)     (5V, ~1A ea.)
         │                 │
         └────────┬────────┘
                  │
                 GND
```

---

## Łączna tabela zasilania

| Urządzenie | Napięcie | Prąd (max) | Moc | Ilość | Suma |
|------------|----------|-----------|-----|-------|------|
| Grzałki (x4) | 12V | 2A ea | 24W ea | 4 | 96W |
| Wiatrak 12V | 12V | 0.5A | 6W | 1 | 6W |
| Nawilrzacze (x2) | 5V | 1A ea | 5W ea | 2 | 10W |
| Czujniki (DS + DHT) | 5V | ~0.2A | ~1W | - | 1W |
| ESP32-S3 + logika | 5V | 0.5A | 2.5W | - | 2.5W |
| LM2596 (overhead) | - | - | - | - | 5W |
| **SUMA** | | | | | **~120W** |

**Zasilacz:** Mean Well LRS-150-12 (150W) - WYSTARCZAJĄCY, marża: ~25%

---

## Zalecenia montażowe

1. **Kondensatory**: 
   - 100uF low-ESR @ +5V (wyjście LM2596)
   - 100uF elektrolit @ +12V (blisko źródła)
   - 100nF ceramiczne przy każdym czujniku cyfrowym
   - 100nF + 1uF przy DHT11

2. **Pull-ups/Pull-downs**:
   - GPIO5 (wiatrak): opcjonalny pull-down 10kΩ
   - GPIO6/7 (DS18B20): 4.7kΩ pull-up do +3.3V
   - GPIO8 (DHT11): 10kΩ pull-up do +3.3V
   - GPIO10 (drzwi): pull-up wewnętrzny ESP32

3. **Rezystory ochronne** (do MOSFET):
   - 10kΩ pull-down na Gate każdego MOSFET (bezpieka przy starcie)

4. **Przewody**:
   - Zasilanie 12V grzałek: min. 1.5 mm² (lub grubsze dla całego zestawu)
   - Sygnały GPIO: 0.5 mm² wystarczająco
   - GND: połączyć wszystkie punkty masy centralnie

5. **Ochrona**:
   - Bezpiecznik 15A na wyjściu zasilacza 12V
   - Varistor / Dioda Schottky'ego na obciążeniach indukcyjnych (wentylator, pompa)

---

## Pliki referencyjne

- **Datasheet ESP32-S3 N8R2**: `/docs/datasheets/ESP32-S3-N8R2.pdf`
- **Datasheet DHT11**: do dodania w `docs/datasheets/`
- **Datasheet DS18B20**: `/docs/datasheets/DS18B20.pdf`
- **Mean Well LRS-150-12**: `/docs/datasheets/LRS-150-12.pdf`

---

**Wersja**: 1.1  
**Data**: 2026-05-16  
**Status**: Zaktualizowany (kondensatory i poziomy sygnałów 3.3V)
