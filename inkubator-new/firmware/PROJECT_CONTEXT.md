# PROJECT_CONTEXT.md

## Opis projektu

Repozytorium `toxiconus/inkubator` zawiera projekt sterownika inkubatora opartego o **ESP32-S3**. Główna aktywna część projektu znajduje się w katalogu `inkubator-new/firmware` i jest przygotowana jako projekt **PlatformIO** dla **VS Code** z frameworkiem **Arduino**. Część zmian była tworzona lub edytowana przy pomocy **GitHub Copilot**.

Projekt łączy w sobie dwie warstwy:
- **firmware C++** uruchamiany na ESP32-S3,
- **wbudowany frontend WWW** przechowywany w systemie plików LittleFS i serwowany bezpośrednio przez urządzenie.

## Główne funkcje

Projekt realizuje lub przygotowuje obsługę następujących funkcji:
- odczyt temperatury z czujników **DS18B20**,
- odczyt wilgotności z czujnika **DHT11**,
- sterowanie grzałkami,
- sterowanie dyfuzorem / nawilżaniem,
- obsługę tac i mechanizmu obracania jaj,
- alarmy temperatury i wilgotności,
- obsługę zegara **RTC DS3231**,
- synchronizację czasu przez **NTP**,
- logowanie danych do plików CSV w **LittleFS**,
- serwer HTTP z panelem konfiguracyjnym,
- komunikację czasu rzeczywistego przez **WebSocket**,
- aktualizację firmware przez **OTA**,
- profile inkubacji zapisane w plikach JSON.

## Środowisko i stack techniczny

- **Edytor / IDE:** VS Code
- **System builda:** PlatformIO
- **Framework:** Arduino
- **Mikrokontroler / płytka:** ESP32-S3 (`esp32-s3-devkitc-1`)
- **System plików:** LittleFS
- **Komunikacja lokalna:** HTTP + WebSocket
- **Aktualizacja:** OTA przez przeglądarkę

### Biblioteki używane w projekcie

Na podstawie `platformio.ini` projekt korzysta m.in. z:
- `adafruit/DHT sensor library`
- `milesburton/DallasTemperature`
- `bblanchon/ArduinoJson`
- `Links2004/WebSockets`
- `adafruit/RTClib`
- `OneWire`
- `rfetick/MPU6050_light`

## Architektura projektu

Projekt jest podzielony na kilka logicznych warstw:

1. **Warstwa sprzętowa**
   - czujniki temperatury i wilgotności,
   - wyjścia sterujące grzałkami,
   - dyfuzory / układ wilgotności,
   - mechanika obracania tac,
   - RTC.

2. **Warstwa firmware**
   - inicjalizacja sprzętu,
   - odczyt czujników,
   - logika sterowania,
   - alarmy,
   - zapis i odczyt konfiguracji,
   - wystawianie API oraz WebSocket,
   - obsługa LittleFS,
   - OTA update.

3. **Warstwa frontendowa**
   - strony HTML panelu użytkownika,
   - wspólny CSS,
   - wspólny i per-strona JavaScript,
   - komunikacja z urządzeniem przez WebSocket i endpointy HTTP.

## Najważniejsze pliki i ich rola

### `inkubator-new/firmware/platformio.ini`
Konfiguracja projektu PlatformIO. Określa docelową płytkę, framework, filesystem oraz zależności bibliotek.

### `inkubator-new/firmware/src/main.cpp`
Główny plik firmware. Zawiera:
- inicjalizację LittleFS,
- ładowanie konfiguracji,
- inicjalizację RTC,
- konfigurację Wi‑Fi w trybie AP,
- konfigurację serwera HTTP i WebSocket,
- odczyt sensorów,
- sterowanie grzałkami i dyfuzorem,
- logowanie do CSV,
- routing API,
- obsługę OTA,
- obsługę komend WebSocket.

### `inkubator-new/firmware/src/config_manager.*`
Obsługa konfiguracji urządzenia: zapis, odczyt, domyślne ustawienia, prawdopodobnie dane kalibracyjne i parametry pracy.

### `inkubator-new/firmware/src/controller.*`
Logika sterowania związana z temperaturą, wilgotnością i wyjściami wykonawczymi.

### `inkubator-new/firmware/src/tray_controller.*`
Obsługa tac, ruchu i harmonogramu obracania.

### `inkubator-new/firmware/src/alarm_manager.*`
Obsługa alarmów przekroczenia parametrów.

### `inkubator-new/firmware/src/incubation_profile.*`
Obsługa profili inkubacji i danych gatunkowych zapisanych w JSON.

## Struktura katalogów

```txt
inkubator/
└── inkubator-new/
    └── firmware/
        ├── .gitignore
        ├── platformio.ini
        ├── temp_dwarf.py
        ├── .specstory/
        ├── .vscode/
        ├── css/
        │   └── style.css
        ├── js/
        │   ├── common.js
        │   └── pages/
        │       ├── alarms.js
        │       ├── humidity.js
        │       ├── incubation.js
        │       ├── temperature.js
        │       ├── turner.js
        │       ├── various.js
        │       └── wifi.js
        ├── data/
        │   ├── ajustes.html
        │   ├── alarms.html
        │   ├── config.json
        │   ├── console.html
        │   ├── humidity.html
        │   ├── incubation.html
        │   ├── incubation_profiles.json
        │   ├── index.html
        │   ├── process_profiles.json
        │   ├── symulacja_legów.html
        │   ├── temperature.html
        │   ├── turner.html
        │   ├── various.html
        │   ├── wifi.html
        │   ├── ws-utils.js
        │   ├── css/
        │   │   └── style.css
        │   └── js/
        │       ├── common.js
        │       └── pages/
        └── src/
            ├── alarm_manager.cpp
            ├── alarm_manager.h
            ├── calibration.h
            ├── commands.h
            ├── config_manager.cpp
            ├── config_manager.h
            ├── controller.cpp
            ├── controller.h
            ├── incubation_profile.cpp
            ├── incubation_profile.h
            ├── main.cpp
            ├── species_profiles.cpp
            ├── species_profiles.h
            ├── tray_controller.cpp
            └── tray_controller.h
```

## Opis katalogów

### `src/`
Główne źródła firmware w C++.

### `data/`
Pliki frontendowe publikowane do LittleFS i serwowane przez ESP32.

Znajdują się tu:
- strony HTML,
- pliki JSON z konfiguracją i profilami,
- wspólny CSS,
- wspólny i częściowo stronicowany JS.

### `data/css/style.css`
**Wspólny arkusz stylów** używany przez wiele stron HTML. Jeśli przekazujesz kod do innego AI, warto zawsze zaznaczać, że jest to CSS współdzielony przez kilka ekranów.

### `data/js/common.js`
Wspólny JavaScript używany przez wiele podstron.

### `data/js/pages/`
Docelowe lub pomocnicze skrypty per-strona.

### `js/` i `css/` poza `data/`
W repo występują również katalogi `firmware/js` i `firmware/css`. Mogą być wersją roboczą, starszą strukturą lub pomocniczym źródłem dla assetów frontendowych. Przy analizie projektu warto rozróżniać te katalogi od plików rzeczywiście serwowanych z `data/`.

## Frontend – widoki i przeznaczenie

Najważniejsze strony w `data/`:
- `index.html` – główny panel urządzenia,
- `temperature.html` – ekran temperatury,
- `humidity.html` – ekran wilgotności,
- `turner.html` – ekran sterowania obracaniem,
- `incubation.html` – ekran ustawień inkubacji,
- `alarms.html` – ekran alarmów,
- `wifi.html` – konfiguracja Wi‑Fi,
- `various.html` – różne ustawienia,
- `console.html` – diagnostyka / konsola,
- `ajustes.html` – ekran ustawień,
- `symulacja_legów.html` – ekran symulacyjny lub testowy.

## Backend / API / komunikacja

Z `main.cpp` wynika, że projekt udostępnia:
- endpointy HTTP, np. status, profile, logi,
- pliki statyczne z LittleFS,
- połączenie WebSocket do wymiany komend i statusów w czasie rzeczywistym,
- OTA update pod ścieżką `/update`.

Przykładowe obszary obsługiwane przez WebSocket:
- konfiguracja temperatury,
- konfiguracja wilgotności,
- konfiguracja obracania tac,
- konfiguracja inkubacji,
- alarmy,
- Wi‑Fi,
- RTC,
- komendy serwisowe i restart,
- kalibracja.

## Jak przekazywać ten projekt do innego AI

Najlepszy sposób:

1. Najpierw podaj kontekst projektu.
2. Następnie podaj strukturę plików.
3. Potem wklej konkretne pliki z pełną ścieżką.
4. Zaznacz, jeśli CSS lub JS są współdzielone między wieloma stronami.
5. Jeśli problem dotyczy frontendu, podaj jednocześnie HTML + powiązany JS + wspólny CSS.
6. Jeśli problem dotyczy logiki urządzenia, podaj też odpowiedni fragment `src/main.cpp` lub właściwego modułu w `src/`.

### Zalecany format

```txt
Projekt: sterownik inkubatora na ESP32-S3
Środowisko: VS Code + PlatformIO + Arduino
Frontend jest serwowany z LittleFS.
Komunikacja odbywa się przez HTTP i WebSocket.

Problem:
[tu opisz problem]

Plik: inkubator-new/firmware/data/temperature.html
```html
[tu kod]
```

Plik: inkubator-new/firmware/data/js/common.js
```js
[tu kod]
```

Plik: inkubator-new/firmware/data/css/style.css
Ten CSS jest wspólny dla wielu stron.
```css
[tu kod]
```

Plik: inkubator-new/firmware/src/main.cpp
```cpp
[tu odpowiedni fragment backendu / WebSocket / API]
```
```

## Wskazówki praktyczne

- Nie wysyłaj do AI samego `style.css` bez informacji, które strony go używają.
- Nie wysyłaj samego `main.cpp`, jeśli problem dotyczy tylko wyglądu frontendu.
- Jeśli problem dotyczy komunikacji, pokaż równocześnie frontendowy JS i firmware obsługujący komendy.
- Przy zmianach CSS warto zaznaczać, że jeden plik stylów może wpływać na wiele widoków.
- Przy zmianach WebSocket warto wskazać dokładny format komend i odpowiedzi.
