# PROJECT_CONTEXT_SHORT.md

## Krótki opis projektu

To jest projekt sterownika inkubatora na **ESP32-S3**.

Środowisko:
- **VS Code**
- **PlatformIO**
- **Arduino framework**
- część zmian była robiona z pomocą **GitHub Copilot**

Projekt składa się z:
- **firmware w C++** (`src/`),
- **frontendu WWW** (`data/`) serwowanego z **LittleFS**,
- komunikacji przez **HTTP** i **WebSocket**.

## Główne funkcje

- odczyt temperatury i wilgotności,
- sterowanie grzałkami,
- sterowanie nawilżaniem / dyfuzorem,
- obsługa obracania tac,
- alarmy,
- RTC,
- logowanie CSV,
- OTA update,
- profile inkubacji w JSON.

## Najważniejsze pliki

- `inkubator-new/firmware/platformio.ini`
- `inkubator-new/firmware/src/main.cpp`
- `inkubator-new/firmware/src/config_manager.*`
- `inkubator-new/firmware/src/controller.*`
- `inkubator-new/firmware/src/tray_controller.*`
- `inkubator-new/firmware/data/index.html`
- `inkubator-new/firmware/data/css/style.css`
- `inkubator-new/firmware/data/js/common.js`

## Ważna uwaga

Plik `inkubator-new/firmware/data/css/style.css` jest **wspólnym CSS** dla wielu stron HTML, więc zmiany w nim mogą wpływać na kilka widoków jednocześnie.

## Jak przekazywać kod do innego AI

Podawaj zawsze:
1. krótki opis projektu,
2. ścieżkę pliku,
3. HTML + powiązany JS + wspólny CSS,
4. przy problemach komunikacyjnych także odpowiedni fragment firmware z `src/`.
