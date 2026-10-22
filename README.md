# Inkubator (ESP32-S3)

Sterownik inkubatora z panelem WWW. Jedno repozytorium zamiast wielu kopii na dysku.

## Układ repozytorium

| Folder | Zawartość |
| --- | --- |
| `inkubator-new/firmware/` | Aktualne firmware (PlatformIO): `src/`, `platformio.ini` |
| `inkubator-new/firmware/data/` | Panel WWW wgrywany do LittleFS (`index.html`, podstrony, `css/`, `js/`) |
| `inkubator-new/docs/` | Dokumentacja bieżąca |
| `inkubator-new/docs/archiwum/` | Starsza dokumentacja projektowa i firmware (zachowana dla kontekstu) |
| `inkubator-new/tools/` | Narzędzia, np. `build_webui.py` |
| `hardware/` | BOM, lista elementów z odzysku, schematy, projekt KiCad `inkubator_v1`, instrukcje okablowania |
| `narzedzia/` | Skrypty pomocnicze: diagnostyka portów COM, test MPU6050, czyszczenie cache VS Code |

## Panel WWW

Strony w `inkubator-new/firmware/data/` są samodzielne i działają na telefonie oraz na komputerze.
Firmware serwuje je z LittleFS z poprawnymi typami MIME (CSS, JS, JSON, SVG).
Nazwy plików są w ASCII (np. `symulacja_legow.html`), bo ESP32 nie obsługuje dobrze nazw z polskimi znakami.

## Uwagi

- Dane w repozytorium (hasła AP, konfiguracja Wi-Fi) są testowe.
- Zrzuty flash (`*.bin`) i kopie zapasowe nie są w repozytorium.
