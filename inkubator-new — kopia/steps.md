# Inkubator New — Development Steps

## Cel
Stworzyć spójny plan prac dla nowego projektu `inkubator-new`, oparty na aktualnym pliku `inkubator-new/docs/mysli.txt` oraz istniejących plikach frontendowych i firmware.

## Stan obecny
- `inkubator-new/docs/mysli.txt` zawiera nową wersję dashboardu i logikę UI.
- W `inkubator-new/web/` oraz `inkubator-new/firmware/data/` są produkcyjne pliki HTML, które muszą zostać uporządkowane i zsynchronizowane.
- `inkubator-new/firmware/src/main.cpp` obsługuje API, WebSocket i serwowanie plików z LittleFS.
- W projekcie znajdują się osobne strony ustawień i symulacja lęgów.

## Najważniejsze obszary pracy
1. Przegląd i weryfikacja `inkubator-new/docs/mysli.txt`
   - sprawdź wszystkie sekcje zakładek: STATUS, LĘG, CHARTS, SCHEDULE, JOURNAL, ESP, LOGS, CALIBRATE, SETTINGS
   - zweryfikuj nowy link `🐣 Symulacja lęgów` i powiązane strony.
2. Porównanie z `inkubator-new/web/index.html` i `inkubator-new/firmware/data/index.html`
   - zidentyfikuj różnice w HTML/CSS/JS
   - określ, które elementy `mysli.txt` są nowe lub lepsze
3. Weryfikacja backendu firmware
   - potwierdź obsługę istniejących API: `/api/status`, `/api/trays`, `/api/profile`, `/api/profiles`
   - sprawdź mechanizm WebSocket w `main.cpp`
   - upewnij się, że wszystkie potrzebne pliki frontendu mogą być serwowane z LittleFS
4. Synchronizacja plików statycznych
   - `index.html` w `web/` i `firmware/data/`
   - `symulacja_legów.html`
   - wszystkie strony ustawień: temperature.html, humidity.html, turner.html, incubation.html, alarms.html, wifi.html, various.html, console.html, ajustes.html
5. Dane i profile inkubacji
   - sprawdź `incubation_profiles.json`
   - upewnij się, że profile gatunków pasują do logiki z zakładki LĘG
6. Testy i walidacja
   - test UI w przeglądarce
   - test API i WebSocket z backendem ESP32
   - sprawdź zachowanie trybu symulacji, gdy urządzenie jest offline

## Priorytetowe zadania
- [ ] Potwierdzić, że `mysli.txt` to aktualny wzorzec UI dla nowego projektu
- [ ] Wstawić brakujący link `Symulacja lęgów` we wszystkich odpowiednich plikach
- [ ] Zidentyfikować elementy HTML/JS w `mysli.txt`, które muszą zostać scalone do produkcyjnego `index.html`
- [ ] Sprawdzić, czy firmware wysyła dane potrzebne do zakładek LĘG i CALIBRATE
- [ ] Upewnić się, że konfiguracja `symulacja_legów.html` działa zarówno w `web/`, jak i `firmware/data/`

## Sugerowany przebieg pracy
1. Przeczytać cały plik `inkubator-new/docs/mysli.txt` i wyciągnąć kluczowe sekcje.
2. Porównać go linia po linii z `inkubator-new/web/index.html` i `inkubator-new/firmware/data/index.html`.
3. Oznaczyć różnice i przygotować listę "do przeniesienia".
4. Przeanalizować backend w `inkubator-new/firmware/src/main.cpp` pod kątem zgodności API z frontem.
5. Uporządkować pliki frontendowe i zbudować pierwszą wersję synchronizacji.
6. Przetestować działanie w przeglądarce oraz komunikację z ESP32.

## Notatki dla przyszłych iteracji
- Nie zaczynaj od `main.cpp`, dopóki nie wystąpi jasna potrzeba nowego endpointu lub format danych.
- Najpierw traktuj `inkubator-new/docs/mysli.txt` jako docelowy wzorzec UI i dopasuj do niego pozostałe warstwy.
- Zachowaj dane; w razie wątpliwości rób porównania delta/merge zamiast nadpisywać pełne pliki.
- Utrzymuj krok dokumentacji: każda większa zmiana UI powinna dostać aktualizację `inkubator-new/steps.md`.

## Kolejne kroki
- użytkownik powinien przekazać pełne wersje plików, które chce zintegrować
- po otrzymaniu kompletnych plików zbudować wersję roboczą i sprawdzić integrację
- następnie uzupełnić ten `steps.md` o konkretne zadania implementacyjne
