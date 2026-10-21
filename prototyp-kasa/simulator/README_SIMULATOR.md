# 🔷 Symulator Inkubatora - Jeden Plik

Plik `inkubator_simulator.ino` to **jeden skonsolidowany plik** zawierający cały kod inkubatora z wbudowaną **symulacją czujników**.

## 🚀 Jak używać

### Opcja 1: Arduino IDE
1. Otwórz `inkubator_simulator.ino` w Arduino IDE
2. Ustaw: **Tools → Board → ESP32 Dev Module**
3. Ustaw port COM
4. **Sketch → Upload**
5. Otwórz **Serial Monitor (115200 baud)**

### Opcja 2: PlatformIO
```bash
cd inkubator/simulator
platformio run -e esp32-simulator -t upload
platformio device monitor -b 115200
```

## 🔷 Co to jest symulacja?

Zamiast rzeczywistych czujników DS18B20 i DHT22, kod **symuluje** ich zachowanie:

```cpp
#define ENABLE_SIMULATION true      // Włącz symulację
```

**Symulacja robi:**
- ✅ Temperatura **dąży do celu** (37.5°C dni 1-21, 37.2°C dni 22+)
- ✅ Wilgotność **reaguje na nawilżacz** (rośnie gdy ON, spada gdy OFF)
- ✅ 3 "czujniki" DS18B20 z **małymi odchyleniami** (realizm)
- ✅ Wszystkie sterowniki PWM **działają normalnie**
- ✅ Relay **steruje symulacją** (nawilżacz wpływa na wilgotność)

## 📊 Profil symulacji

```cpp
#define SIMULATION_TEMP_RATE 0.02        // Temp zmienia się o 0.02°C per 5s
#define SIMULATION_HUMIDITY_RATE 0.5     // Wilg. zmienia się o 0.5% per 5s
```

## ⏱️ Szybkie testowanie - DNI

Domyślnie:
- **1 dzień = 86400 sekund** (24 godziny)

Aby testować szybciej, zmień w `loop()`:

```cpp
// SZYBKIE TESTOWANIE - 1 dzień = 1 sekunda:
int calculatedDay = (elapsedSeconds / 1) + 1;
```

Wtedy całą 28-dniową inkubację przejdziesz w **28 sekund!** ⚡

## 🎯 Scenariusze testowe

### Test 1: Czy grzałka się włącza?
```cpp
// W setup():
setHeaterPower(0);
// Temperatura będzie spadać → grzałka się włączy → będzie rosnąć
```

### Test 2: Czy nawilżacz działa?
```cpp
// Włącz ręcznie:
setHumidifier(true);
// Wilgotność powinna rosnąć
```

### Test 3: Czy taca się obraca?
```cpp
// Serial będzie pokazywał:
// 🔄 TACA: OBRACA ... STOP (co godzinę, chyba że testujesz z przyspieszonym czasem)
```

### Test 4: Czy światło włącza się w dniu 21?
```cpp
// Po 21 dniach:
// 💡 ŚWIATŁO: ON
```

## 📋 Output Serial

```
┌─────────────────────────────────────────────────┐
│ DZIEŃ: 1/28                                     │
├─────────────────────────────────────────────────┤
│ 🌡️  TEMPERATURA:                                │
│    DS1:36.8°C  DS2:37.1°C  DS3:36.9°C           │
│    Średnia: 36.9°C  Cel: 37.5°C                 │
│ 💧 WILGOTNOŚĆ: 42.5%  Cel: 40%                  │
│ 🔥 GRZAŁKA: 255/255                             │  ← Grzeje!
│ 💨 WIATRAKI: F1:ON F2:ON F3:OFF                 │
│ 💦 NAWILŻACZ: OFF                               │
│ 🔄 TACA: OBRACA                                 │
│ 💡 ŚWIATŁO: OFF                                 │
│ 🔷 TRYB SYMULACJI                               │  ← Symulacja!
└─────────────────────────────────────────────────┘
```

## 🔧 Zmiana parametrów

Wszystkie zmienne są na **górze pliku**:

```cpp
#define TARGET_TEMP 37.5               // Zmień cel temp
#define TARGET_HUMIDITY_EARLY 40       // Zmień wilg. dni 1-18
#define FAN1_PWM 200                   // Zmień moc went. 1
#define TRAY_CYCLE_INTERVAL 3600       // Zmień interwal obrotu tacy
```

Zmień, żeby testować inne scenariusze!

## ⚠️ Ważne

1. **Piny PWM/GPIO będą pracować normalnie** - jeśli masz podłączone np. LED na pin 2, grzałka będzie migać 🔥
2. **Relay GPIO będą się włączać** - jeśli masz podłączone fizyczne cewki relay, będą klikać
3. **Czujniki rzeczywiste NIE będą czytane** - `ENABLE_SIMULATION true` wyłącza OneWire/DHT

## 📌 Przejście na rzeczywisty kod

Aby wrócić do normalnego trybu z rzeczywistymi czujnikami:

```cpp
#define ENABLE_SIMULATION false       // Wyłącz symulację
```

Wtedy kod będzie czytać rzeczywiste DS18B20 i DHT22.

---

**Wersja z symulacją idealna do testowania logiki na biurku bez podłączonych czujników!** 🎉

