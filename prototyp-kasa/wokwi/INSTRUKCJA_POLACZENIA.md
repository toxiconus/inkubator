# 🔗 INSTRUKCJA PODŁĄCZEŃ - Wokwi

Krok po kroku jak podłączyć wszystkie komponenty w Wokwi.com

## � ZŁĄCZA PRAKTYCZNE (Dla fizycznej budowy)

### 💰 Rekomendacja: Złącze GX (zakręcane)

Jeśli budujesz rzeczywisty inkubator, rekomendujemy **złącza GX** (rozłączne, wodoszczelne IP65):

```
GX12-3pin ← DS18B20 (3 piny)
GX13-4pin ← DHT22 (4 piny)
```

⚠️ **WAŻNE:** Liczba przy GX (12, 13, 16...) to ROZMIAR obudowy, nie liczba pinów!
- GX12 może mieć: 2-pin, 3-pin, 4-pin, 5-pin...
- GX16 może mieć: 4-pin, 5-pin, 6-pin, 7-pin...

**Przy zakupie szukaj konkretnie:** `GX12-3pin` lub `GX13-4pin`

| Opcja | Cena | Zalety | Gdzie |
|-------|------|--------|-------|
| GX12-3pin | 0.50-2 zł | Wodoszczelne (IP65), solidne, ładnie wygląda | Aliexpress |
| GX13-4pin | 0.60-2 zł | Wodoszczelne (IP65), 4 piny dla DHT22 | Aliexpress |
| JST-XH | 0.30-0.50 zł | Tańsze, popularne, ale bez IP65 | Wszędzie |
| Dupont | 0.05-0.15 zł | Najtańsze, rozłączne | Zestawy Arduino |

---

## �📍 Lista Komponentów w Wokwi

```
✓ ESP32 Dev Kit V1
✓ 7x LED (Grzałka, Fan1, Fan2, Fan3, Nawilżacz, Taca, Światło)
✓ 3x DS18B20 (Temperature Sensor)
✓ 1x DHT22 (Temperature + Humidity)
✓ Rezystory (4.7kΩ, 10kΩ, 100Ω)
```

---

## 🔌 SEKCJA 1: ZASILANIE (GND - Common Ground)

### Wszystkie GND razem!

```
ESP32 GND──────×──────────── Breadboard GND (lewsza szyna)
             /|\
            / | \
           /  |  \
      LED1GND LED2  LED3GND
           |   |    |
         .....(wszystkie GND na jednej szynie).....
```

**KROKI:**
1. Podłącz ESP32 GND pin do GND szyny breadboarda (zwykle czarna linia)
2. Wszystkie czarne przewody (GND) z LED podłącz do tej szyny GND

---

## ⚡ SEKCJA 2: ZASILANIE (+3.3V)

```
ESP32 3.3V ─────×────── Breadboard +3.3V (czerwona szyna)
             /|\
            / | \
           /  |  \
      DHT22  DS18B20  Rezystory
         |    |      |
     (wszystko na szynie +3.3V)
```

**KROKI:**
1. Podłącz ESP32 pin 3.3V do +3.3V szyny (czerwona linia)
2. DHT22 pin VCC → +3.3V szyna
3. DS18B20 pin VCC (każdy) → +3.3V szyna
4. Rezystor 4.7kΩ (jeden koniec) → +3.3V szyna

---

## 🔴 SEKCJA 3: LED STERÓW (PWM)

### LED Grzałka - GPIO2 (CZERWONA LED)

```
ESP32 GPIO2 ─────[220Ω Rezystor]─────→ LED ANODE (dłużka)
                                           │
                                        LED KATODA (krótka)
                                           │
                                          GND szyna
```

**Kolory na breadboardzie:**
- Red wire: GPIO2 → Rezystor
- LED anode (+) na rezystorze
- LED katoda (-) na GND

### LED Wentylator 1 - GPIO19 (NIEBIESKA LED)

```
ESP32 GPIO19 ────[220Ω Rezystor]───→ LED ANODE
                                        │
                                      GND
```

### LED Wentylator 2 - GPIO18 (NIEBIESKA LED)

```
ESP32 GPIO18 ────[220Ω Rezystor]───→ LED ANODE
                                        │
                                      GND
```

### LED Wentylator 3 - GPIO5 (NIEBIESKA LED)

```
ESP32 GPIO5 ─────[220Ω Rezystor]───→ LED ANODE
                                        │
                                      GND
```

### LED Nawilżacz - GPIO12 (CYAN LED)

```
ESP32 GPIO12 ────[220Ω Rezystor]───→ LED ANODE
                                        │
                                      GND
```

### LED Taca - GPIO14 (ŻÓŁTA LED)

```
ESP32 GPIO14 ────[220Ω Rezystor]───→ LED ANODE
                                        │
                                      GND
```

### LED Światło - GPIO13 (POMARAŃCZOWA LED)

```
ESP32 GPIO13 ────[220Ω Rezystor]───→ LED ANODE
                                        │
                                      GND
```

---

## 🌡️ SEKCJA 4: DS18B20 (OneWire - 3 czujniki)

### Schemat OneWire (wszyscy w PARALLEL)

```
                    +3.3V
                       │
                    [R 4.7kΩ]
                       │
ESP32 GPIO4 ───────────┼───────┬─── DS18B20 #1 (DQ)
                       │       ├─── DS18B20 #2 (DQ)
                       │       └─── DS18B20 #3 (DQ)
                       │
                      GND
```

### Krok po kroku:

**1️⃣ Rezystor pull-up (4.7kΩ):**
```
Koniec A: + (do +3.3V szyny)
Koniec B: → GPIO4 oraz łączy do wszystkich DQ czujników
```

**2️⃣ DS18B20 #1:**
```
Pin GND → GND szyna
Pin DQ  → GPIO4 (przez rezystor 4.7kΩ)
Pin VCC → +3.3V szyna
```

**3️⃣ DS18B20 #2:**
```
Pin GND → GND szyna
Pin DQ  → GPIO4 (ten sam punkt co #1!)
Pin VCC → +3.3V szyna
```

**4️⃣ DS18B20 #3:**
```
Pin GND → GND szyna
Pin DQ  → GPIO4 (ten sam punkt co #1 i #2!)
Pin VCC → +3.3V szyna
```

### Wizualne połączenia (top view):

```
                Pin layout DS18B20:
                1(GND)  2(DQ)  3(VCC)
                  │      │      │
                  ↓      ↓      ↓
    GND szyna ◄────┘      │      └───► +3.3V szyna
                          │
                    GPIO4 z rezystorem
```

---

## 💧 SEKCJA 5: DHT22 (Temperatura + Wilgotność)

### Pinout DHT22 (od lewej do prawej):

```
1 (VCC)   2 (DATA)   3 (NC)    4 (GND)
  │         │        skip       │
  │         │                   │
  ↓         ↓                   ↓
+3.3V    GPIO15    [R 10kΩ]    GND
           ▲        (pull-up)
           │
           └────────────────┐
                           │
                    10kΩ rezystor
                    do +3.3V
```

### Krok po kroku:

**1️⃣ DHT22 VCC (pin 1):**
```
DHT22 VCC → +3.3V szyna
```

**2️⃣ DHT22 GND (pin 4):**
```
DHT22 GND → GND szyna
```

**3️⃣ DHT22 DATA (pin 2) z pull-up:**
```
ESP32 GPIO15 ──[R 10kΩ]──┬─→ +3.3V szyna
                         │
                    DHT22 DATA (pin 2)
```

**4️⃣ DHT22 NC (pin 3):**
```
Nie podłączaj - zostaw pusty!
```

### Wizualne na breadboardzie:

```
ESP32 GPIO15 ──────┐
              R 10kΩ├──► +3.3V szyna
                    │
              DHT22 DATA
```

---

## 📋 CHECKLIST PODŁĄCZEŃ

### Zasilanie ✓
- [ ] ESP32 GND połączony z GND szyna
- [ ] ESP32 3.3V połączony z +3.3V szyna
- [ ] Wszystkie czarne kable wspólnie na GND
- [ ] Wszystkie czerwone kable wspólnie na +3.3V

### LED (PWM) ✓
- [ ] LED Grzałka (GPIO2) - CZERWONA
- [ ] LED Fan1 (GPIO19) - NIEBIESKA
- [ ] LED Fan2 (GPIO18) - NIEBIESKA
- [ ] LED Fan3 (GPIO5) - NIEBIESKA
- [ ] LED Nawilżacz (GPIO12) - CYAN
- [ ] LED Taca (GPIO14) - ŻÓŁTA
- [ ] LED Światło (GPIO13) - POMARAŃCZOWA
- [ ] Każda LED ma rezystor 220Ω do GND

### Sensory ✓
- [ ] DS18B20 #1: GND→GND, DQ→GPIO4, VCC→+3.3V
- [ ] DS18B20 #2: GND→GND, DQ→GPIO4, VCC→+3.3V
- [ ] DS18B20 #3: GND→GND, DQ→GPIO4, VCC→+3.3V
- [ ] Rezystor 4.7kΩ pull-up: +3.3V → GPIO4
- [ ] DHT22 VCC → +3.3V
- [ ] DHT22 GND → GND
- [ ] DHT22 DATA → GPIO15 z rezystorem 10kΩ do +3.3V
- [ ] DHT22 NC nie podłączony

---

## 🔍 Test Połączeń - Serial Monitor

Gdy wszystko podłączysz, sprawdź Serial Monitor (115200):

```
✓ Powinno widać:
  - DS18B20: Znaleziono 3 czujników
  - DHT22: Inicjalizacja OK
  - PWM: OK
  - GPIO: OK

✗ Jeśli błędy:
  - DS18B20: 0 czujników → sprawdź rezystor 4.7kΩ
  - DHT22 NaN → sprawdź rezystor 10kΩ i GPIO15
  - Brak wiadomości → sprawdź GND
```

---

## 🎯 Schemat Pudełkowy - Szybki Przegląd

```
┌─────────────────────────────────────────┐
│              ESP32                      │
│  GND ───┐                          ┌─ 3.3V
│         │                          │
│    GPIO2 ──→ LED1 (Heater)         │
│    GPIO19 ─→ LED2 (Fan1)           │
│    GPIO18 ─→ LED3 (Fan2)           │
│    GPIO5 ──→ LED4 (Fan3)           │
│    GPIO12 ─→ LED5 (Humidifier)     │
│    GPIO14 ─→ LED6 (Tray)           │
│    GPIO13 ─→ LED7 (Light)          │
│    GPIO4 ───→ DS18B20 x3 (OneWire) │
│    GPIO15 ──→ DHT22 (Data)         │
│         │                          │
└─────────┼──────────────────────────┼──┘
          │                          │
         GND szyna          +3.3V szyna
         (czarna)           (czerwona)
          │                          │
     ║Breadboard║
```

---

## 🚀 Gotowe! Teraz co?

1. **Wgraj kod**: sketch.ino
2. **Otwórz Serial Monitor**: 115200 baud
3. **Obserwuj LED** - powinny migać zgodnie z logiką
4. **Czytaj wartości** - temperatura, wilgotność w Serial

---

## 📞 Troubleshooting Szybko

| Problem | Przyczyna | Rozwiązanie |
|---------|-----------|------------|
| LED nie świeci | Brak zasilania lub źle podłączona | Sprawdź GPIO i rezystor |
| DS18B20 nie widać | Rezystor pull-up | Sprawdź 4.7kΩ między GPIO4 a +3.3V |
| DHT22 pokazuje NaN | Za wolno czyta (NaN = error) | Sprawdź rezystor 10kΩ i GPIO15 |
| Serial Monitor pusty | Baud rate | Zmień na 115200 |
| Grzałka zawsze ON/OFF | Błąd logiki sterowania | Normalnie - to prawidłowe |

---

**Powodzenia! Teraz masz pełną dokumentację podłączeń! 🎉**

