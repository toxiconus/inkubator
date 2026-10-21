# 🔧 PRZEWODNIK MONTAŻU - Krok po kroku

Praktyczny guide do fizycznego montażu inkubatora na ESP32.

## 📦 Co będziesz potrzebować

### Komponenty elektroniczne
```
□ 1x ESP32 Dev Kit V1
□ 1x Breadboard 830 punktów (lub 2x 400pt)
□ 3x DS18B20 (1-Wire Temperature Sensor)
□ 1x DHT22 (Temperature + Humidity)
□ 4x MOSFET N-channel (IRF540N lub 2N7000)
□ 3x NPN Transistor (2N3904 lub BC547)  
□ Rezystor 4.7kΩ (x2) - pull-up OneWire + DHT
□ Rezystor 10kΩ (x3) - pull-down transistor
□ Rezystor 20Ω (x4) - MOSFET gate protection
□ Rezystor 100Ω (x1) - LED
□ Rezystor 1kΩ (x3) - pull-up relay
□ Kondensator 100nF (x4) - bypass MOSFET
□ 3x Dioda 1N4007 - freewheeling (silniki)
□ 1x Dioda 1N4148 - fast switching
□ 1x LED czerwona (do testowania)
```

### Zasilanie
```
□ PSU 5V/3A - logika + sensory
□ PSU 12V/8A - silniki wiatraków
□ PSU 220V/1000W - grzałka (opcjonalnie na test 5V)
□ Kable zasilające AWG 18 (dla 12V silników)
□ Kable zasilające AWG 14 (dla 220V grzałki)
```

### Narzędzia
```
□ Multimetr
□ Lutownica 30-40W
□ Przypajalnik / Kabel припойowy
□ Zacisk do kabli
□ Kable DuPont (Male-Female)
□ Izolacja elektryczna / taśma Teflon
```

## 🛠️ FAZA 1: Przygotowanie Breadboarda

### Krok 1: Zainstaluj ESP32 na środku
```
Breadboard:
┌─────────────────────────────┐
│  A B C D E    F G H I J K   │
├─────────────────────────────┤
│ •  •  •  •  •  ESP32  •  •  │  ← Umieść ESP32 pośrodku
│ •  •  •  •  •  pins   •  •  │
│ •  •  •  •  •  •  •   •  •  │
│                              │
└─────────────────────────────┘

Dolna część ESP32 na pinach (A-C), górna część na (F-H)
```

### Krok 2: Zasilanie - szyny 5V i GND
```
Lewo od ESP32:
┌───────────┐
│ +5V |  • GND (czarny drut) ← GND
│ +3.3V, GND                  
│ (czerwony drut) ← +5V
│ (niebieski drut) ← +3.3V
└───────────┘

Prawo od ESP32:
┌───────────┐
│ GND |  • +12V (żółty drut) ← +12V
│ GND |  • +5V_AUX
└───────────┘
```

**Ważne**: Wszystkie masy (GND) muszą być połączone razem!

## 🌡️ FAZA 2: Sensory

### Krok 1: Montaż DS18B20 x3 (OneWire)

```
ESP32 GPIO4 (OneWire)
    │
    ├─→ [Rezystor 4,7kΩ] ──→ +3.3V
    │ (pull-up)
    │
    ├─→ DS18B20 #1 (DQ pin)  ┐
    ├─→ DS18B20 #2 (DQ pin)  ├── WSZYSTKIE W PARALLEL
    └─→ DS18B20 #3 (DQ pin)  ┘

Każdy DS18B20:
    GND ──→ GND
    +3.3V ← +3.3V
    DQ ──→ GPIO4 (razem z pull-up)
```

**Rozmieszczenie na breadboardzie:**
```
Col: |A  |B   |C   |D   |E  |
     ├───┼────┼────┼────┤
+3.3V  • ─┼────┼────┼────┤
       |  |R 4.7k|     |
Gnd    • ─┼────┼────┼────┤
       |  |    DS  DS  DS │
       |  |    #1  #2  #3 │
GPIO4  • ─┼────●────●────●─
```

### Krok 2: Montaż DHT22

```
ESP32 GPIO15 (DHT22 Data)
    │
    ├─→ [Rezystor 10kΩ] ──→ +3.3V
    │ (pull-up)
    │
    └─→ DHT22 DAT pin

DHT22 pins (patrząc od przodu):
    1 (VCC)   → +3.3V
    2 (DATA)  → GPIO15 (z pull-up 10kΩ)
    3 (NC)    → nie podłączamy
    4 (GND)   → GND
```

## ⚡ FAZA 3: PWM Wyjścia (Grzałka + Wiatraki)

### Schemat ogólny MOSFET
```
ESP32 GPIO (3.3V PWM)
    │
    ├─→ [Rezystor 20Ω] ──→ MOSFET Gate
    │   (protection)     │
    │                   Source
    │                     │
    └────────────────→ GND

        Drain
          │
    [Obciążenie 12V/220V]
```

### GPIO2 - Grzałka (PWM0)

```
COMPONENT LAYOUT:
┌─────────────────────────────────┐
│ +5V/12V ZASILANIE              │
│    │                            │
│    ├─→ [GRZAŁKA] ──→ DRAIN     │
│    │   lub 220V      MOSFET    │
│    │                  IRF540   │
│    └─────┬──────────────────→   │
│  GND     │                      │
└─────────────────────────────────┘
           │
         [Gate]←─ [R 20Ω] ←─ GPIO2
           │
         [Source] → GND
```

**Na breadboardzie:**
```
                        Col D   Col E
GPIO2 ─ [R20Ω] ─ MOSFET Gate ─ •
                 │
                 MOSFET Drain ─ HIGH VOLTAGE OUT
                 │
                 MOSFET Source─ GND (Common)
```

### GPIO19 - Wentylator 1 (PWM1)
```
GPIO19 ─ [R20Ω] ─ MOSFET Gate
                       │
                  [FAN 12V] ← Drain
                       │
                      GND ← Source
```

### GPIO18 - Wentylator 2 (PWM2)
```
GPIO18 ─ [R20Ω] ─ MOSFET Gate
                       │
                  [FAN 12V] ← Drain
                       │
                      GND ← Source
```

### GPIO5 - Wentylator 3 (PWM3)
```
GPIO5  ─ [R20Ω] ─ MOSFET Gate
                       │
                  [FAN 12V] ← Drain
                       │
                      GND ← Source
```

## 🔌 FAZA 4: Relay (GPIO Wyjścia)

### Schemat ogólny Relay
```
ESP32 GPIO (3.3V)
    │
  [R 1-2kΩ] ─→ NPN Transistor Base
               │
               Emitter → GND
               │
            Collector → [Relay Cewka +5V]
                             │
                        [1N4007 Dioda] ← freewheeling
                             │
                            GND
```

### GPIO12 - Nawilżacz
```
GPIO12 ─ [R 2kΩ] ─ Transistor Base (BC547)
                               │
                          Emitter → GND
                               │
                            Collector ──→ +5V
                                  │
                            [Relay Cewka]
                                  │
                            [Dioda 1N4007]
                                  │
                                 GND
```

### GPIO14 - Taca
```
GPIO14 ─ [R 2kΩ] ─ Transistor Base
                               │ (sama logika)
                             ...
```

### GPIO13 - Światło LED (TEST)
```
GPIO13 ─ [R 100Ω] ─ LED ANODE
                         │
                       GND
```

## 🧪 FAZA 5: Testowanie

### Test 1: Zasilanie
```
□ Multimetr: pomiar między +5V a GND
  Powinno być: ≈ 5V ± 0.2V

□ Multimetr: pomiar między +12V a GND
  Powinno być: ≈ 12V ± 0.5V

□ Multimetr: pomiar między +3.3V a GND
  Powinno być: ≈ 3.3V ± 0.1V
```

### Test 2: DS18B20 (OneWire)
```
□ Wgraj: inkubator_simulator.ino
□ Otwórz Serial Monitor (115200 baud)
□ Powinno widać: DS18B20: Znaleziono 3 czujników
□ Adresy czujników wypisane

Jeśli błąd "Znaleziono 0":
  - Sprawdź rezystor pull-up 4,7kΩ
  - Spawdź czy wszystkie GND są popołączone
  - Sprawdź czy pin GPIO4 jest powiedziony na prosto
```

### Test 3: DHT22
```
□ Odczytuj Serial: Temperatura i Wilgotność
□ Powinno być: 20-30°C, 30-60%
□ Jeśli "Błąd odczytu": czekaj 2 sekundy (DHT ma limit)
```

### Test 4: PWM LED (GPIO2)
```
□ Podłącz LED + rezystor 100Ω na GPIO2
□ W kodzie: setHeaterPower(128);  // Połowa mocy
□ LED powinna świecić PÓŁJAŚNIE (nie pełni)
□ setHeaterPower(255);  → PEŁNA JASNOŚĆ
□ setHeaterPower(0);    → OFF
```

### Test 5: GPIO LED (GPIO13)
```
□ Podłącz LED na GPIO13
□ W kodzie: setLight(true);
□ LED powinna się zaświecić
□ setLight(false);
□ LED powinna zgasnąć
```

## ✅ Checklist Gotowości

- [ ] Wszystkie sensory działają (DS18B20 x3, DHT22)
- [ ] PWM LED świecą i regulują jasność
- [ ] GPIO LED świecą (ON/OFF)
- [ ] GND jest wspólne wszędzie
- [ ] Rezystory pull-up na miejscu
- [ ] Diody freewheeling na silnikach
- [ ] Kanały PWM nie "kłócą się" (każdy na innym GPIO)
- [ ] Serial Monitor wyświetla prawidłowe wartości

## 🎉 Gratulacje!

System jest gotowy do:
1. Instalacji w obudowie inkubatora
2. Podłączenia rzeczywistych grzałek (220V)
3. Podłączenia rzeczywistych silników (12V)
4. Startu inkubacji! 🐣

---

**Powodzenia z montażem!** 🔧⚡

