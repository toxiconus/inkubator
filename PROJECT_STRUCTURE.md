# Projekt Inkubator — struktura katalogów i plików

## 1. Główny katalog (`j:\inkubator`)

- `.git/` — repozytorium Git.
- `.venv/` — virtual environment Python używany przez PlatformIO i narzędzia.
- `.vscode/` — ustawienia edytora i konfiguracje pracy.
- `.pio/` — bufor i pliki tymczasowe PlatformIO.
- `.specstory/` — katalog wsparcia dla SpecStory lub podobnych narzędzi.
- `inkubator-new/` — główny rozwijany projekt.
- `inkubator-new — kopia/` — kopia zapasowa projektu, prawdopodobnie do porównań lub testów.

### Skrypty i pliki pomocnicze w katalogu głównym

- `check_com9_vscode.py` — skrypt diagnostyczny do sprawdzania połączenia COM9.
- `check_mpu_com.cmd` / `check_mpu_com.ps1` — narzędzia do testowania komunikacji z czujnikiem MPU.
- `cleanup-vscode-cache.cmd`, `cleanup-vscode-chat-cache.cmd` — skrypty do czyszczenia pamięci podręcznej VS Code.
- `fix_html_pages.ps1` — skrypt do naprawy plików HTML.
- `inspect_ps1.ps1` — pomocniczy skrypt PowerShell do inspekcji.
- `mpu6050_i2c_scan.ino` — przykładowy szkic Arduino do skanowania I2C czujnika MPU6050.
- `read_com9_once.ps1` — jednorazowy odczyt z portu COM9.
- `repair-index-mojibake.ps1` — naprawa problemów z kodowaniem znaków w pliku `index.html`.
- `rewrite_check_mpu_com.py` — dodatkowy skrypt do testów MPU przez COM.
- `serial_diag.py` — diagnostyka szeregowa urządzenia.
- `serial_mpu_out.txt` — zapis wyjścia diagnostycznego MPU.
- `serial_reset_try.py` — odnawianie/diagnostyka resetu portu szeregowego.
- `stdout` — plik zawierający wyjście z jednej z sesji terminalowych.
- `temp_read_com9.py` — tymczasowy skrypt do odczytu z portu COM9.
- `vscode-storage-report.txt` — raport o pamięci i plikach VS Code.

## 2. Katalog `inkubator-new`

Główne miejsce rozwoju projektu.

- `prompt.md` — notatki, opis celów lub pomysły projektu.
- `steps.md` — instrukcje, etapy pracy lub lista kroków do wykonania.
- `docs/` — dokumentacja projektu.
- `firmware/` — kod i zasoby dla ESP32-S3.
- `web/` — statyczne pliki interfejsu webowego.
- `outputs/` — wyniki kompilacji, wygenerowane pliki lub artefakty.

### `inkubator-new/docs`

Folder dokumentacji, zawiera pliki:

- `INSTRUKCJA_INTEGRACJI.md` — instrukcja integracji projektu.
- `mysli.txt` — zbiór myśli, notatek i pomysłów.
- `POROWNANIE_PROJEKTOW_2026-07-22.md` — porównanie projektów z datą 2026-07-22.
- `QUICK_REFERENCE_TABLE.md` — szybka tabela referencyjna.
- `README.md` — główny opis projektu.
- `SCHEMA_SYGNALOW.md` — schemat sygnałów i połączeń.

### `inkubator-new/firmware`

To główna część kodu urządzenia.

- `.gitignore` — ignorowane pliki Git w folderze firmware.
- `.pio/` — lokalny cache PlatformIO dla tego projektu.
- `.specstory/` — konfiguracja wsparcia SpecStory.
- `.vscode/` — konfiguracja VS Code specyficzna dla firmware.
- `css/` — dodatkowe style CSS (prawdopodobnie używane lokalnie lub przy budowie).
- `data/` — zasoby webowe ładowane do pamięci flash urządzenia.
- `js/` — dodatkowe skrypty JavaScript dla interfejsu.
- `platformio.ini` — główny plik konfiguracyjny PlatformIO.
- `src/` — kod źródłowy C++.
- `temp_dwarf.py` — pomocniczy skrypt Python, być może do konwersji lub budowy zasobów.

#### `inkubator-new/firmware/src`

Główne pliki źródłowe firmware:

- `alarm_manager.cpp` / `alarm_manager.h` — obsługa alarmów i powiadomień.
- `calibration.h` — kalibracja czujników lub wejść.
- `commands.h` — definicje poleceń i protokołu komunikacji.
- `config_manager.cpp` / `config_manager.h` — zarządzanie konfiguracją urządzenia.
- `controller.cpp` / `controller.h` — główna logika sterowania urządzeniem.
- `incubation_profile.cpp` / `incubation_profile.h` — profile inkubacji i logika przebiegów.
- `main.cpp` — punkt wejścia programu na ESP32-S3.
- `species_profiles.cpp` / `species_profiles.h` — profile gatunków i ustawienia specyficzne dla rodzaju inkubowanego zwierzęcia.
- `tray_controller.cpp` / `tray_controller.h` — sterowanie tacą/obracaniem lub mechanizmem wewnątrz inkubatora.

#### `inkubator-new/firmware/data`

Pliki interfejsu webowego, które prawdopodobnie trafiają do pamięci flash urządzenia.

- `ajustes.html` — strona ustawień.
- `alarms.html` — strona alarmów.
- `console.html` — konsola systemowa.
- `humidity.html` — strona wilgotności.
- `incubation.html` — strona inkubacji.
- `index.html` — główna strona interfejsu.
- `index.before-mojibake-fix.html` — wersja przed naprawą problemów z kodowaniem znaków.
- `symulacja_legow.html` / `symulacja_legów.html` — symulacja lęgów / inkubacji.
- `temperature.html` — strona temperatury.
- `turner.html` — strona sterowania obracaniem tac.
- `various.html` — strona różnych parametrów.
- `wifi.html` — strona konfiguracji Wi-Fi.
- `config.json` — dane konfiguracyjne używane przez frontend lub urządzenie.
- `incubation_profiles.json` — profile inkubacyjne w formacie JSON.
- `process_profiles.json` — profile procesów inkubacji.
- `service-worker.js` — obsługa offline i cache w przeglądarce.
- `ws-utils.js` — narzędzia WebSocket.
- `components/` — komponenty HTML/JS dla strony.
- `css/` — styl CSS dla webowego UI.
- `icons/` — ikony używane w UI.
- `js/` — skrypty JavaScript interfejsu.
- `pages/` — dodatkowe strony lub fragmenty generowane przez UI.

#### `inkubator-new/firmware/js`

- `common.js` — wspólne skrypty JavaScript.
- `pages/` — dodatkowe skrypty specyficzne dla poszczególnych stron.

### `inkubator-new/web`

Statyczna wersja strony / wygenerowany web UI o tych samych nazwach:

- `ajustes.html`, `alarms.html`, `console.html`, `humidity.html`, `incubation.html`, `index.html`, `symulacja_legów.html`, `temperature.html`, `turner.html`, `various.html`, `wifi.html`

### `inkubator-new/outputs`

Miejsce na artefakty i wygenerowane pliki wynikowe.

## 3. Kopia projektu (`inkubator-new — kopia`)

Pełna kopia struktury projektu, która odzwierciedla katalogi i pliki z `inkubator-new/`.

- Przydatna jako backup lub alternatywna wersja do porównań.

## 4. Użytkowanie

- `inkubator-new/firmware/` to rdzeń uruchamiany na ESP32-S3.
- `inkubator-new/firmware/data/` zawiera webowy interfejs użytkownika ładowany z urządzenia.
- `inkubator-new/docs/` zawiera dokumentację i opis projektu.
- `inkubator-new/web/` wygląda jak statyczna strona wynikowa do podglądu lub deploymentu.
- Skrypty w katalogu głównym służą do diagnostyki, debugowania i naprawy plików.

---

Plik wygenerowano jako `PROJECT_STRUCTURE.md` w katalogu głównym workspace.