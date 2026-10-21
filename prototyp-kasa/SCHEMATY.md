# 📐 SCHEMATY I POŁĄCZENIA INKUBATORA

Pełne schematy i opisy fizycznych połączeń dla projektu inkubatora.

## 🔌 Ogólny schemat blokowy

```
┌─────────────────────────────────────────────────────────────┐
│                         ESP32                               │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  PWM (ledcSetup)     GPIO Digital      ADC (Input)    │  │
│  │  ┌──────────────┐    ┌──────────────┐  ┌──────────┐   │  │
│  │  │ GPIO2  CH0   │    │ GPIO4 OneWire│  │ GPIO35   │   │  │
│  │  │ GPIO19 CH1   │    │ GPIO15 DHT   │  │ GPIO34   │   │  │
│  │  │ GPIO18 CH2   │    │ GPIO12 Relay1│  └──────────┘   │  │
│  │  │ GPIO5  CH3   │    │ GPIO14 Relay2│                 │  │
│  │  └──────────────┘    │ GPIO13 Relay3│                 │  │
│  │                       └──────────────┘                 │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
        │                        │                    │
        │                        │                    │
  ┌─────┴──────┐        ┌────────┴────────┐    ┌─────┴─────┐
  │   WYJŚCIA  │        │    SENSORY      │    │  WEJŚCIA  │
  │   PWM      │        │                 │    │ (REZERWА) │
  └─────┬──────┘        └────────┬────────┘    └───────────┘
        │                        │
        ├─ Grzałka              ├─ 3x DS18B20 (OneWire)
        ├─ Wentylator 1         ├─ DHT22
        ├─ Wentylator 2         └─ Przycisk (future)
        └─ Wentylator 3
```

## 🔋 Schemat zasilania i sygnałów

```
┌──────────────────────────────────────────────────────────────────┐
│                    ZASILANIE 5V / GND                             │
└──────────────────────────────────────────────────────────────────┘
    ▲                                           │
    │                                           │
    │ +5V                                       ▼ GND (Common)
    │
    ├─→ [GRZAŁKA 220V+ REZYSTOR]─→ MOSFET ←─ GPIO2 (PWM)
    │
    ├─→ [FAN1 SILNIK 12V]──→ MOSFET ←─ GPIO19 (PWM)
    │
    ├─→ [FAN2 SILNIK 12V]──→ MOSFET ←─ GPIO18 (PWM)
    │
    ├─→ [FAN3 SILNIK 12V]──→ MOSFET ←─ GPIO5 (PWM)
    │
    ├─→ [RELAY NAWILŻACZ]──→ Cewka ←─ GPIO12 (Digital)
    │
    ├─→ [RELAY TACA]───────→ Cewka ←─ GPIO14 (Digital)
    │
    └─→ [RELAY ŚWIATŁO]────→ Cewka ←─ GPIO13 (Digital)


┌──────────────────────────────────────────────────────────────────┐
│                 SENSORY (3.3V/5V Mixed)                           │
└──────────────────────────────────────────────────────────────────┘
    +3.3V (lub +5V pull-up)          GND (Common)
       │                               │
       ├─ [Rezystor 4.7kΩ]   ┌────→──┴─→ 3x DS18B20  ←─ GPIO4 (OneWire)
       │                       │
       └─ +3.3V ─→ DHT22 ←───┴─ GPIO15 (Data)
                   │
                   └─ GND
```

## 📱 Szczegółowe połączenia po pinie

### ⚡ PWM WYJŚCIA (GPIO HIGH SIDE)

#### GPIO2 - Grzałka (PWM0)
```
ESP32 GPIO2 (PWM0, 0-255)
         │
         └──→ [MOSFET N-channel] ──→ GRZAŁKA
              Gate (tu 3.3V PWM)
              │
              Source ──→ GND
              Drain  ──→ GRZAŁKA(-) ──→ +5V ZASIL
```
**Rezystor**: 10-20Ω w serii z bramką (zalecany)
**Kondensator**: 100nF pomiędzy Gate-Source (ochrona)

#### GPIO19 - Wentylator 1 (PWM1)
```
ESP32 GPIO19 (PWM1, 0-255)
         │
         └────→ [MOSFET] ──→ SILNIK 12V FAN1
               Duty: 200/255 (~78%)
```
**Napięcie**: 12V DC
**Prąd**: ~0.5A (wymagany driver MOSFET)

#### GPIO18 - Wentylator 2 (PWM2)
```
ESP32 GPIO18 (PWM2, cykliczny)
         │
         └────→ [MOSFET] ──→ SILNIK 12V FAN2
               ON: 300ms (255/255)
               OFF: 200ms (0/255)
```

#### GPIO5 - Wentylator 3 (PWM3)
```
ESP32 GPIO5 (PWM3, awaryjny)
         │
         └────→ [MOSFET] ──→ SILNIK 12V FAN3
               Włącza się: temp > 38.5°C
```

### 🔌 GPIO WYJŚCIA (RELAY / DIGITAL OUTPUT)

#### GPIO12 - Nawilżacz (5V RELAY)
```
ESP32 GPIO12 (HIGH=Włącz)
         │
         └───→ [NPN Transistor] ──→ RELAY Cewka
              Collector ──→ +5V
              Base  ←───── GPIO12
              Emitter ──→ GND
    
      RELAY NO Contact ────→ Nawilżacz Soniczny
```
**Rezystor**: 1-2kΩ pomiędzy GPIO12 a Base transistora

#### GPIO14 - Taca (obracanie jaj)
```
ESP32 GPIO14 (HIGH=Włącz)
         │
         └───→ [NPN Transistor] ──→ RELAY Cewka
              
      RELAY NO Contact ────→ Silnik Tacy
      ON: 10 sekund co godzinę (dni 1-20)
```

#### GPIO13 - Światło LED
```
ESP32 GPIO13 (HIGH=ON)
         │
         └───→ [100Ω Rezystor] ──→ LED ANODE
                                        │
                                       GND
      
      LED włącza się: dzień ≥ 21
```

### 🌡️ SENSORY WEJŚCIA

#### GPIO4 - OneWire (DS18B20 x3)
```
+3.3V (przez rezystor 4.7kΩ pull-up)
  │
  ├─ [R 4.7kΩ] ──┬──→ GPIO4 (OneWire)
  │              │
  └──────────────┼──→ DS18B20 #1 (DQ pin)
                 ├──→ DS18B20 #2 (DQ pin)
                 └──→ DS18B20 #3 (DQ pin)

GND ←──── DS18B20 #1 (GND pin)
     ←──── DS18B20 #2 (GND pin)
     ←──── DS18B20 #3 (GND pin)

+3.3V ──→ DS18B20 #1 (VCC pin)
      ──→ DS18B20 #2 (VCC pin)
      ──→ DS18B20 #3 (VCC pin)
```
**Ważne**: Wszystkie 3 czujniki są w **PARALLEL** na jednej linii (bus OneWire)

#### GPIO15 - DHT22 (Temperatura + Wilgotność)
```
+3.3V (bezpośrednio)
  │
  ├─→ [DHT22 VCC]
  │
  ├─→ [Rezystor ~10kΩ pull-up]
  │   │
  │   ├──→ GPIO15 (Data)
  │   │
  │   └──→ [DHT22 DAT pin]
  │
  └─→ [DHT22 GND] ──→ GND
```

## 📊 Podłączenia - Tabela szybka

| Pin ESP32 | Typ | Funkcja | Połączenie | Napięcie | Prąd |
|-----------|-----|---------|-----------|----------|------|
| GPIO2 | PWM | Grzałka | MOSFET Gate | 3.3V | ~50mA |
| GPIO19 | PWM | Fan1 | MOSFET Gate | 3.3V | ~50mA |
| GPIO18 | PWM | Fan2 | MOSFET Gate | 3.3V | ~50mA |
| GPIO5 | PWM | Fan3 | MOSFET Gate | 3.3V | ~50mA |
| GPIO12 | GPIO | Nawilżacz | Transistor Base | 3.3V | ~10mA |
| GPIO14 | GPIO | Taca | Transistor Base | 3.3V | ~10mA |
| GPIO13 | GPIO | Światło | Rezystor 100Ω | 3.3V | ~20mA |
| GPIO4 | Input | OneWire (3x DS) | Pull-up 4.7kΩ | 3.3V | ~1mA |
| GPIO15 | Input | DHT22 Data | Pull-up 10kΩ | 3.3V | ~1mA |
| GND | - | Common Ground | Wszystko | 0V | - |
| 3.3V | - | Logic Supply | Sensory | 3.3V | ~200mA |

## 🛠️ Komponenty niezbędne

### Ze strony zasilania (MOSFET/RELAY):
- [ ] 4x MOSFET N-channel (np. IRF540N / 2N7000) - dla PWM grzałka + wiatraki
- [ ] 3x NPN Transistor (2N3904 / BC547) - dla GPIO relay
- [ ] 4x Rezystor 1-2kΩ - pull-down dla transistorów
- [ ] 4x Rezystor 10-20Ω - protection dla MOSFET gate
- [ ] Rezystor 4.7kΩ - pull-up OneWire
- [ ] Rezystor 10kΩ - pull-up DHT22
- [ ] Rezystor 100Ω - dla LED światła
- [ ] Kondensatory 100nF - bypass dla MOSFET

### Sensory:
- [x] 3x DS18B20 (1-Wire)
- [x] 1x DHT22 (Temperature + Humidity)

### Zasilanie:
- [ ] PSU 5V/3A (dla logiki + sensory)
- [ ] PSU 12V/8A (dla silników wiatraków)
- [ ] PSU 220V/400W (dla grzałki) - **Z CHŁODNICA GRZE**

## 🔐 Bezpieczeństwo i Ochrona

### Ochrona PWM (MOSFET):
```
        R protect
         ┃ 20Ω
ESP32 ───┃───┤ MOSFET ├──→ Load
         │    Gate
         └─ GND (fast discharge)
```

### Ochrona GPIO (Transistor):
```
        R protect
         ┃ 2kΩ
ESP32 ───┃───┤ Transistor ├──→ Relay Cewka
         │    Base
    [Dioda] ←
      1N4148    (Freewheeling diode)
```

### Diody Freewheeling (dla indukcyjnych obciążeń):
```
    +5V
     │
    ┌┴┐
    │ │  RELAY/SILNIK (indukcyjny)
    │ │
    └┬┘
     │  *(Dioda 1N4007 w REVERSE*
     │   do ochrony przed spike'ami)
    GND
```

## 📋 Checklist Montażu

- [ ] ESP32 na breadboardzie
- [ ] Rezystor pull-up 4.7kΩ na OneWire (GPIO4)
- [ ] Rezystor pull-up 10kΩ na DHT22 (GPIO15)
- [ ] 3x DS18B20 podłączone równolegle
- [ ] DHT22 podłączony na GPIO15
- [ ] 4x MOSFET dla PWM (grzałka + 3x wiatraki)
- [ ] 3x Transistor NPN dla GPIO Relay
- [ ] Wszystkie diody freewheeling na miejscu
- [ ] Zasilanie 5V i 12V podłączone
- [ ] GND jest wspólny dla wszystkich obwodów (połączony)
- [ ] Wszystkie połączenia sprawdzane multimetrem

## 🧪 Testowanie Fizyczne

### Test 1: Sensory
```
1. Podłącz tylko sensory (DS18B20, DHT22)
2. Wgraj kod, otwórz Serial
3. Powinny być bieżące odczyty temperatury
```

### Test 2: PWM LED
```
1. Podłącz LED na GPIO2 (przez rezystor 100Ω)
2. Ustaw: setHeaterPower(128);
3. LED powinna świecić półjaśnie
```

### Test 3: Relay
```
1. Podłącz LED na GPIO12 (przez transistor)
2. Ustaw: setHumidifier(true);
3. LED powinna się zaświecić
```

---

**Wszystko gotowe do montażu! 🔧⚡**

