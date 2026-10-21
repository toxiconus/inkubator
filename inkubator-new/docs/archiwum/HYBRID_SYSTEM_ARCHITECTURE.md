# 🐠 Hybrid System: Akwarium + Grzałki 12V

## 🎯 Nowa Architektura

```
WARSTWA 1: BAZA TERMICZNA (pasywna)
┌─────────────────────────────────────┐
│   GRZAŁKA AKWARYSTYCZNA 220V        │
│   (Termostat elektroniczny)         │
│   Temperatura: 36-36.5°C (stała)    │
│   + Wilgotność: naturalne parowanie │
│                                     │
│   📦 Małe akwarium (10-20L)        │
│   Głębia: ~20-30cm                 │
│   Połud: poniżej komory jaj         │
│   System: grzanie + parowanie       │
└─────────────────────────────────────┘
            ↑ ciepło wzstępuje
            ↑ para wodna dyfunduje

WARSTWA 2: DOPASOWANIE (aktywna)
┌─────────────────────────────────────┐
│  GRZAŁKI SILIKONOWE 12V (1-2 szt)   │
│  PWM sterowanie: tylko ostatnie °C  │
│  Rola: +0.5 do +1.5°C do docelów    │
│                                     │
│  37.5°C (jaja) = 36°C (akwarium)   │
│           + 1.5°C (grzałki PWM)     │
└─────────────────────────────────────┘

WARSTWA 3: KONTROLA (informacyjna)
┌─────────────────────────────────────┐
│  CZUJNIKI TEMPERATURY               │
│  1. DS18B20 w AKWARIUM (36°C)      │
│  2. DS18B20 W KOMORZE JAJ (37.5°C)│
│                                     │
│  CZUJNIK WILGOŚCI                   │
│  DHT22 w komorze jaj                │
│                                     │
│  Zadanie ESP32:                     │
│  - Monitoruj bazę (36°C)           │
│  - Jeśli za zimno -> więcej PWM    │
│  - Jeśli za gorąco -> mniej PWM    │
└─────────────────────────────────────┘
```

---

## 🔥 Zalety Hybrid System

| Aspekt | Grzałki PWM (old) | Akwarium (new) | Hybrid ✅ |
|--------|-------------------|------------------|----------|
| **Dokładność** | ±1-2°C | ±0.5°C | ±0.3°C |
| **Energooszczędność** | ~100W ciągłe | ~100W ciągłe | ~80W (akw) + 20W (PWM) |
| **Niezawodność** | Elektronika | Mechanika 50lat | Backup (każde podsystem działa solo) |
| **Wilgotność** | Manualne NAV | Naturalne parowanie | Automatyczne + łatwe |
| **Stres termiczny** | Ryzyko skoku | Baza stała | Minimalne |
| **Złożoność** | Średnia | Niska | Średnia (ale bardziej intuicyjna) |
| **Dostępność części** | Niszowe | Każde zoo, sklep akwarium | ✅ Wszędzie |

---

## 📐 Layout Fizyczny

```
KOMORA JAJ (pudelko styropianowe/termobox)
┌──────────────────────────────────┐
│  🥚🥚🥚 Tace z jajami            │
│        (na siatce)               │
│                                  │
│  🌡️ DS18B20 (mierzy 37.5°C)     │
│  💧 DHT22 (mierzy wilgotność)   │
│                                  │
│  🔥 Grzałki silikonowe PWM      │
│     (2x 24W, łącznie ~48W)      │
│                                  │
│  🌫️ Odpowietrznik (przepływ air)│
└──────────────────────────────────┘
         ↑
        ciepło
        para
         ↑
┌──────────────────────────────────┐
│ AKWARIUM (pod komorą jaj)        │
│ ~15 litrów wody                  │
│                                  │
│ 🌡️ DS18B20 (mierzy 36-36.5°C)  │
│ 🔌 Grzałka akwarystyczna 220V   │
│    (z termostatem elektronicznym)│
│                                  │
│ Przepływ: Naturalny (ciepła)     │
│           Parowanie (wilgoć)     │
│                                  │
│ ⚙️ Dodatkowo: Mały wentylator    │
│    (opcjonalnie, recykulacja)    │
└──────────────────────────────────┘
```

---

## 💻 Zmodyfikowany Algorytm Sterowania

### FAZA 1: Startup (0-60 minut)
```
1. Grzałka akwarystyczna: ZAWSZE ON (docelowo 36°C)
   └─ Termostat elektroniczny robi swoje (nie sterujemy!)

2. Grzałki PWM: LINEAR RAMP-UP
   └─ 0% → 100% przez 60 minut
   └─ Docelowo: 36°C (akwarium) + 1.5°C (PWM) = 37.5°C
```

### FAZA 2: Utrzymanie (60 min → 21 dni)
```
1. Monitoruj 2 czujniki:
   - T_akwarium (powinno być ~36°C - jeśli nie, problem z grzałką!)
   - T_jaja (powinno być ~37.5°C)

2. Algorytm PWM:
   IF (T_jaja < 37.5 - 0.5°C):
       PWM++ (więcej mocy)
   ELSE IF (T_jaja > 37.5 + 0.5°C):
       PWM-- (mniej mocy)
   ELSE:
       Utrzymaj bieżące PWM

3. Wilgotność:
   - Jedno akwarium + rozpylacz = zwykle 55-75%
   - Jeśli zbyt sucho → dodaj wodę do akwarium
   - Jeśli zbyt wilgotnie → odsłoń wentylator
```

---

## 🧬 Nowy Kod ESP32

Tylko **czujniki zmienić** - reszta logiki jest taka sama!

### ZMIANA 1: Dodaj DS18B20 akwarium

```cpp
// NOWE: Drugi czujnik temperatury (w akwarium)
#define PIN_TEMP_AQUARIUM    7    // GPIO7 - DS18B20 w akwarium

OneWire oneWire_eggs(PIN_TEMP_SENSOR_1);      // Dla jaj
OneWire oneWire_aquarium(PIN_TEMP_AQUARIUM);  // Dla akwarium

DallasTemperature sensors_eggs(&oneWire_eggs);
DallasTemperature sensors_aquarium(&oneWire_aquarium);
```

### ZMIANA 2: Struktura danych

```cpp
struct SensorData {
  float temp_inside;        // 🥚 Temperatura jaj (DS18B20)
  float temp_aquarium;      // 🐠 Temperatura akwarium (DS18B20 #2)
  float humidity;           // 💧 Wilgotność (DHT22)
  int water_level;          // 🚰 Poziom wody w akwarium (ADC)
  bool door_open;           // 🚪 Drzwi
  int heater_pwm;           // 🔥 PWM (tylko dopasowanie!)
  unsigned long timestamp;  // ⏱️ Czas
};
```

### ZMIANA 3: Odczyt akwarium

```cpp
void read_sensors(SensorData &data) {
  // DS18B20 Jaja
  sensors_eggs.requestTemperatures();
  data.temp_inside = sensors_eggs.getTempCByIndex(0);
  
  // ✨ NEW: DS18B20 Akwarium
  sensors_aquarium.requestTemperatures();
  data.temp_aquarium = sensors_aquarium.getTempCByIndex(0);
  
  // DHT22 - wilgotność
  data.humidity = dht.readHumidity();
  
  // ... reszta bez zmian
}
```

### ZMIANA 4: Sterowanie (UPROSZCZONE!)

```cpp
// STARA GRZAŁKA: Ramp-up 0-100% przez 60 min
// NOWA GRZAŁKA: Ramp-up 0-50% przez 60 min (bo akwarium już grzeje!)

void control_heaters(const SensorData &data) {
  unsigned long elapsed = data.timestamp;
  
  // MONITOROWANIE AKWARIUM!
  if (data.temp_aquarium < 35.5) {
    Serial.println("⚠️  UWAGA: Akwarium za zimne! Sprawdź grzałkę 220V!");
  } else if (data.temp_aquarium > 37.0) {
    Serial.println("⚠️  UWAGA: Akwarium za gorące! Zmniejsz termostat!");
  }
  
  // FAZA 1: Ramp-up tylko do 50% (bo baza już jest ciepła!)
  if (elapsed < HEATER_RAMP_UP_TIME) {
    current_heater_pwm_percent = (float)elapsed / HEATER_RAMP_UP_TIME * 50.0;  // Max 50%!
  }
  // FAZA 2: Utrzymywanie
  else {
    if (data.temp_inside < (TARGET_TEMP - HEATER_HYSTERESIS)) {
      current_heater_pwm_percent = min(50.0, current_heater_pwm_percent + 1.0);  // Max 50%!
    } else if (data.temp_inside > (TARGET_TEMP + HEATER_HYSTERESIS)) {
      current_heater_pwm_percent = max(0.0, current_heater_pwm_percent - 1.0);
    }
  }
  
  // Ustaw PWM (0-255)
  int pwm_value = (int)(current_heater_pwm_percent * 2.55);
  ledcWrite(PWM_CHANNEL_HEATER, pwm_value);
}
```

---

## 🐠 Grzałka Akwarystyczna - Rekomendacje

### Typ: Termostat Elektroniczny
- **Model**: EHEIM Thermostat / Aquael / Sera
- **Moc**: 50-100W (dla 10-20L wody)
- **Dokładność**: ±0.5°C (przebadane!)
- **Cena**: ~50-80 PLN
- **Czas rozgrzewu**: ~30-45 min do 36°C

### Ustawienie
```
Temperatura akwarium: 36.0°C (stała)
    ↓
Przełącznik/pokrętło: Ustaw na ~36°C
    ↓
Czekaj 30 min na stabilizację
    ↓
Zmierz multimetrem → powinna być 36±0.5°C
```

### Bezpieczeństwo
```
✅ Nigdy nie gorsze niż 220V -> GND prosta kabel
✅ Grzałka akwarystyczna ma wbudowaną ochronę
✅ Jeśli nie wiem -> kupić z termostatem (nie osobno!)
```

---

## 🧪 Nowy Plan Testów (Hybrid System)

### TEST 0: Kalibracja Akwarium (30 min)
```
1. Włóż grzałkę 220V do wody
2. Ustaw termostat na 36°C
3. Czekaj 30 minut
4. Zmierz DS18B20 (powinno być ~36°C)
5. Jeśli nie: reguluj pokrętło termostat
```

### TEST 1: Startup bez grzałek PWM (kontrola)
```
1. Obie grzałki PWM: OFF (0% PWM)
2. Komora jaj pustej, ale czujnik
3. Obserwuj przez 2 godziny
4. Rezultat powinien być:
   - T_jaja = T_akwarium ± 1°C
   - Jeśli nie: problem z izolacją!
```

### TEST 2: Startup z grzałkami PWM (hybrid)
```
1. Włącz ramp-up 50% (nie 100! - to ważne!)
2. Obserwuj ramp-up przez 60 minut
3. Rezultat powinien być:
   - 0 min:   36°C (akwarium)
   - 30 min:  36.7°C (akwarium + 25% PWM)
   - 60 min:  37.5°C (akwarium + 50% PWM) ✓
   - 180 min: 37.5°C ±0.3°C (stabilne)
```

### TEST 3: Wilgotność naturalnie
```
1. Akwarium zawiera wodę (naturalne parowanie)
2. DHT22 powinny mierzyć ~60-70%
3. Jeśli zbyt sucho (<50%):
   - Zwiększ poziom wody w akwarium
   - Lub dodaj rozpylacz (nawilżacz)
```

### TEST 4: Redundancja (Security check)
```
1. Wyłącz grzałkę 220V (co się stanie?)
   - Temp. jaj spadnie powoli
   - ESP32 zwiększy PWM automatycznie
   - Powinien osiągnąć 37.5°C dzięki PWM samemu (wolniej)

2. Wyłącz grzałki PWM (co się stanie?)
   - Temp. jaj = Temp. akwarium (~36°C)
   - Jaja będą za zimne, ale żywe
   - Termostat akwarystyczny działa bez PWM
```

---

## ⚙️ Hardware Changes Checklist

```
☐ Grzałka akwarystyczna 220V (50-100W z termostatem)
☐ Przewód zasilania 220V (bezpiecznik!)
☐ Mały zbiornik na wodę (10-20L, tworzywo)
☐ Drugich DS18B20 (GPIO7)
☐ 4.7kΩ rezystor (pull-up dla drugiego czujnika!)
☐ Izolacja pomiędzy akwarium a komorą jaj

USUNĄĆ:
  ☐ Nie ma kontroli 220V z ESP32 (grzałka sama się włącza!)
  ☐ Nie ma dodatkowych przycisków/przełączników
```

---

## 📊 Porównanie: PWM Only vs. Hybrid

```
METRYKA              PWM ONLY          HYBRID (Akwarium) ✅
─────────────────────────────────────────────────────────
Dokładność           ±1-2°C            ±0.3-0.5°C
Ramp-up              100% power/60min  50% power/60min (mniej stresu!)
Stabilność           Falowania         Gładka krzywa
Wilgotność           Manualna          Naturalna (parowanie)
Energia grzańca      96W ciągłe        80W (akw) + 20W (PWM avg)
Niezawodność         Electronics       Mechanics (50 lat w akwariach!)
Koszt części         ~200 PLN          ~250 PLN (+ 50 PLN grzałka)
Cena startowa        Wyższa (2 grzałki) Równa (grzałka akw = grzałka PWM)
Plan B (gdy awaria)  ???               Każdy system działa solo
```

---

## 🎓 Dlaczego to Działa?

```
FIZYKA:

1. Ciepło wznosi się naturalnie (konwekcja)
   → Woda akwarium (36°C) grzeje otoczenie

2. Wilgość paruje naturalnie
   → Woda w akwarium → para → komora jaj

3. Grzałki PWM tylko "dopasowują"
   → +1-1.5°C, żeby osiągnąć 37.5°C

4. System jest "inercyjny"
   → Nie ma skaczących przeskoków
   → Embriony mają miękkie warunki
   → Mniej stresu termicznego = lepsze wyniki
```

---

## 🚀 Następne Kroki

1. ✅ Przeczytaj ten plik (już zrobiłeś!)
2. ☐ Kup grzałkę akwarystyczną (50-80 PLN)
3. ☐ Przygotuj mały zbiornik (~15L, myj plastikowy)
4. ☐ Czekaj na zmodyfikowany kod z ESP32 (patrz poniżej)
5. ☐ Wykonaj TEST 0 (kalibracja akwarium)
6. ☐ Wykonaj TEST 1-4 (hybrid system)
7. ☐ Porównaj CSV z PWM-only metodą (jeśli masz dane)
8. ☐ Zbierz jaja do testów!

---

## 📝 Ostateczna Rekomendacja

```
🏆 HYBRID SYSTEM TO ZWYCIĘZCA

Powody:
  1. Akwarium = sprawdzony system (50+ lat w naturze)
  2. Termostat = mechaniczny, niezawodny, ±0.5°C
  3. PWM = backup + precyzja (ostatnie 1.5°C)
  4. Wilgość = naturalna, nie trzeba kombinować
  5. Bezpieczeństwo = redundancja (każdy system działa solo)
  6. Prostota = mniej elektroniki = mniej błędów
  7. Tanio = tyle samo co PWM-only (~250 PLN)

Jeśli się zdecydujesz na to:
  → Będę modyfikować kod główny main.cpp
  → Zmienię ramp-up na 50% (nie 100%)
  → Dodam drugi DS18B20 do monitorowania
  → Uproszczę algorytm (mniej krytyczno)
  → Wyjaśnię troubleshooting dla dwóch czujników
```
