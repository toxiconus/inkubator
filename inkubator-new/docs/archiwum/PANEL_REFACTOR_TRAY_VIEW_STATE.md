# Centralny model stanu widoków `Status` / `Panel`

## Cel

Opisać, jak działa aktualne zjednoczenie stanu sterowania tacami i co jeszcze trzeba dopracować, aby widoki `Status` i `Panel` korzystały z jednej wspólnej reprezentacji stanu.

## Co jest zrobione

- Wprowadzono `trayControlState` jako wspólny model dla ustawień tac (`species`, `day`, `spd`, `dur`).
- Dodano funkcje:
  - `getTrayControlState(i)` — zwraca centralny stan komend dla tacy `i`.
  - `applyTrayControlState(i)` — aplikuje ten stan do obu paneli (`main-*` i `status-*`).
  - `readTrayControlInputs(i)` — odczytuje wartości z pól formularza, normalizuje je i zapisuje do `trayControlState`.
  - `setTrayControlState(i, updates)` — aktualizuje model i odświeża widok.
  - `installTrayControlSync()` — nasłuchuje zmian w obu panelach i utrzymuje stan w synchronizacji.
- `syncMainControls(s, trays)` synchronizuje obie sekcje paneli z backendowym stanem tac i aktualizuje centralny model `trayControlState`.
- `updateTrayUI(i, tr)` zapisuje stan z backendu do:
  - `setTrayState(i,{day,species})` — centralny magazyn stanu tacy,
  - `setTrayControlState(i,{day,species})` — stan formularza.
- Dodano też `getTrayState(i)` / `setTrayState(i, updates)` jako jednolitą warstwę dostępu do `state.tray` i `deviceState.trays`.
- `getTrayDay(i,tray)` i `getTraySpecies(i,tray)` korzystają z tego centralnego stanu jako fallbacku, gdy backend nie dostarczy danych.

## Co już działa dobrze

- Widoki `main` i `status` używają wspólnej reprezentacji `trayControlState` dla ustawień komend.
- `setTrayDayStatus(i)` i `moveTrayStatus(i)` już korzystają z `trayControlState` i `readTrayControlInputs(i)`.
- Zmiany gatunku lub dnia w panelu głównym aktualizują też panel statusowy.
- Po odebraniu statusu z backendu oba panele są synchronizowane z centralnym widokiem.
- `state.tray` i `deviceState.trays` są utrzymywane spójnie przez `setTrayState()`.

## Co jest jeszcze do zrobienia

1. Uporządkować logikę `Status` / `Panel` tak, aby wszystkie akcje komend czytały tylko z `trayControlState` i nie kopiowały wartości bezpośrednio z jednego DOM-u do drugiego.
   - `setTrayDayStatus(i)` i `moveTrayStatus(i)` już używają `readTrayControlInputs(i)` i `trayControlState` zamiast kopiować wartości z `status-*` do `main-*`.
2. Dodać wyraźną separację między:
   - `trayControlState` — stan bieżących opcji do wysłania jako komenda,
   - `state.tray` / `deviceState.trays` — ostatnio pobrany status tacy,
   - `state.timers.tray` — lokalny timer hatcha/anchor.
3. Przejrzeć funkcje wykonujące komendy (`setTrayDay()`, `moveTray()`, `setTraySpecies()`) i zastąpić bezpośrednie odczyty z `document.getElementById('status-*')` lub `document.getElementById('main-*')` odczytami z centralnego modelu.
4. Jeśli chcemy pełnego centralnego store, warto też:
   - zmniejszyć powiązanie `applyTrayControlState()` z bezpośrednim manipulowaniem DOM-u,
   - ewentualnie dodać `getTrayCommandState(i)` jako readonly API dla akcji.

## Co znalazłem

- Kod ma już rozpoznane dwa poziomy stanu:
  - `trayControlState` dla formularzy,
  - `state.tray` / `deviceState.trays` dla stanu urządzenia.
- Brakuje jednej, czystej ścieżki przepływu dla akcji z panelu `status` do wykonania komendy.
- `trayControlState` jest dobrym fundamentem, ale obecna implementacja wciąż używa DOM jako drugorzędnego źródła prawdy.

## Wnioski

Ten fragment jest już częściowo zaimplementowany. Aby uznać go za skończony, trzeba:

- usunąć bezpośrednie przeklejanie wartości między polami `status` i `main`,
- wszystkie działania komend opierać na `trayControlState`,
- doprecyzować, że `state.tray` to tylko zapis ostatniego znanego statusu tacy.
