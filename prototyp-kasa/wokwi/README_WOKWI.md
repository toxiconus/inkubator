# 🌐 Inkubator na Wokwi.com

Pełna symulacja inkubatora na platformie **[Wokwi.com](https://wokwi.com/)** - bezpośrednio w przeglądarce!

## 🚀 Jak uruchomić

### 📖 WAŻNE: INSTRUKCJA PODŁĄCZEŃ
**Zanim zaczniesz - przeczytaj [INSTRUKCJA_POLACZENIA.md](INSTRUKCJA_POLACZENIA.md)** - Pełny przewodnik jak podłączyć wszystkie komponenty!

### Opcja 1: Upload do Wokwi (najszybciej)

1. Idź na [wokwi.com](https://wokwi.com/)
2. Kliknij **"Create Project"**
3. Wybierz **ESP32**
4. Skopiuj zawartość `sketch.ino` do edytora
5. Skopiuj `diagram.json` do sekcji schemat
6. Kliknij **Play** ▶️

### Opcja 2: Import folderu (GitHub)

Jeśli masz GitHub:
1. Utwórz repozytorium
2. Upload folder `/wokwi`
3. Idź na [Wokwi](https://wokwi.com/) i zaladuj z GitHub

## 📊 Co widzisz w Wokwi

### Komponenty fizyczne (symulowane):
- ✅ **ESP32 Dev Kit V1** - mikrokontroler
- 🔴 **LED1 Czerwona** (Grzałka) - PWM pin 2
- 🔵 **LED2 Niebieska** (Wentylator 1) - PWM pin 19
- 🔵 **LED3 Niebieska** (Wentylator 2) - PWM pin 18
- 🔵 **LED4 Niebieska** (Wentylator 3) - PWM pin 5
- 🔹 **LED5 Cyan** (Nawilżacz) - GPIO pin 12
- 🟡 **LED6 Żółta** (Taca) - GPIO pin 14
- 🟠 **LED7 Pomarańczowa** (Światło) - GPIO pin 13
- 🌡️ **3x DS18B20** (Temperature) - OneWire pin 4
- 💧 **DHT22** (Temperature + Humidity) - GPIO pin 15

### Symulacja:
- ✅ **Rzeczywiste czujniki Wokwi** (DS18B20, DHT22)
- ✅ **PWM dla LED** (pokazuje intensywność)
- ✅ **GPIO relay** (LED ON/OFF)
- ✅ **OneWire z rezystorem pull-up**

## 🔍 Co obserwować

Otwórz **Serial Terminal** (ctrl+shift+M) i obserwuj:

```
┌─────────────────────────────────────────────────┐
│ DZIEŃ: 1/28                                     │
├─────────────────────────────────────────────────┤
│ 🌡️  TEMPERATURA:                                │
│    DS1:25.5°C  DS2:25.5°C  DS3:25.5°C           │
│    Średnia: 25.5°C  Cel: 37.5°C                 │
│ 💧 WILGOTNOŚĆ: 50.0%  Cel: 40%                  │
│ 🔥 GRZAŁKA: 255/255         ← LED PEWNO ŚWIECI! │
│ 💨 WIATRAKI: F1:ON F2:OFF F3:OFF               │
│ 💦 NAWILŻACZ: OFF                               │
│ 🔄 TACA: OBRACA                                 │
│ 💡 ŚWIATŁO: OFF                                 │
│ 🌐 WOKWI SIMULATION                             │
└─────────────────────────────────────────────────┘
```

## ⏱️ Szybkie testowanie

Domyślnie dni trwają po 24 godziny (86400 sekund).

**Aby przyspaczyć do testów**, zmień w `sketch.ino`:

```cpp
// Szybko: 1 dzień = 1 sekunda
int calculatedDay = (elapsedSeconds / 1) + 1;

// Średnio: 1 dzień = 10 sekund
int calculatedDay = (elapsedSeconds / 10) + 1;

// Normalnie: 1 dzień = 24 godziny
int calculatedDay = (elapsedSeconds / 86400) + 1;
```

## 🔷 Scenariusze testowe w Wokwi

### Test 1: Zwrotnica grzałki
1. Zaobserwuj temperaturę początkową (~25°C)
2. 🔴 LED grzałka powinna się zaświecić
3. Temperatura będzie rosnąć (~0.1°C co sekundę w symulacji)
4. Gdy zbliży się do 37,5°C, grzałka zgaśnie

### Test 2: Nawilżacz
1. Zaobserwuj wilgotność (50%)
2. Włącz nawilżacz ręcznie:
   ```cpp
   setHumidifier(true);  // W setup()
   ```
3. 🔹 LED cyan powinna się zaświecić
4. Wilgotność będzie rosnąć

### Test 3: Przejście na dzień 21
1. Ustaw szybkie dni: `elapsedSeconds / 1`
2. Czekaj 21 sekund
3. 🟠 LED światła powinna się zaświecić
4. 🟡 LED tacy powinna się wyłączyć (koniec obrotu)

### Test 4: Obserwuj cykliczny Fan2
1. Otwórz Serial Monitor
2. Fan2 powinien mieć wzór: ON (300ms) → OFF (200ms) → ON...
3. 🔵 LED fan2 powinna pulować

## 🎨 Schemat podłączeń

```
ESP32
├─ GPIO2  ──→ LED Grzałka (Red)
├─ GPIO19 ──→ LED Fan1 (Blue)
├─ GPIO18 ──→ LED Fan2 (Blue)
├─ GPIO5  ──→ LED Fan3 (Blue)
├─ GPIO12 ──→ LED Nawilżacz (Cyan)
├─ GPIO14 ──→ LED Taca (Yellow)
├─ GPIO13 ──→ LED Światło (Orange)
│
├─ GPIO4  ──→ OneWire ──────┬── DS18B20 #1
│               (Pull-up)   ├── DS18B20 #2
│                           └── DS18B20 #3
│
└─ GPIO15 ──→ DHT22 (Data)
```

## 📝 Notatki Wokwi

- **DS18B20**: Rzeczywista symulacja - można zmieniać temperaturę w GUI
- **DHT22**: Rzeczywista symulacja - można zmieniać wilgotność
- **PWM LED**: Jasność LED odpowiada wartości PWM (0-255)
- **GPIO Relay**: LED ON/OFF (8V) na pinach GPIO

## 🔧 Edycja w przeglądarce

W Wokwi możesz:
- ✅ Edytować kod na żywo
- ✅ Uruchamiać ponownie (Reset)
- ✅ Obserwować Serial Monitor
- ✅ Klikać komponenty aby zmienić parametry
- ✅ Dodawać nowe komponenty z biblioteki

## 🌍 Udostępnianie symulatora

Gdy skończysz testowanie, możesz:
1. Kliknij **Share Link** w Wokwi
2. Podeślij link innym - mogą patrzeć na symulację
3. Inne osoby mogą klikać i patrzeć (read-only) lub forknąć

## ⚙️ Zaawansowana symulacja

Możesz:
- Podłączyć szeregowy Wifi sensor do Wokwi
- Dodać wyświetlacz LCD/OLED
- Symulować główny alarm (buzzer)
- Dodać fotorezystor do pomiaru oświetlenia

---

**Całe testowanie bez физического sprzętu! 🌐✨**

