# Plan refaktoryzacji panelu sterowania inkubatorem

## Cel
Skrócić i uporządkować plan refaktoryzacji, żeby skupić się na stanie, fetchach, walidacji, kalibracji i wydajności.

## Co jest zrobione
- [x] Oddzielono polling statusu od komend sterujących
- [x] Dodano timeout-aware helpery `fetchJsonWithTimeout()` / `fetchJson()`
- [x] Przeniesiono manualne komendy do timeoutowego helpera
- [x] `loadSchedule(trayIdx)` używa helpera i lokalnego fallbacku
- [x] Wykorzystano `getTrayTimer()` w funkcjach wizualnych
- [x] Dodano walidację formularzy + offline/retry dla komend
- [x] Poprawiono logikę auto-LAP w kalibracji
- [x] Widoczny komunikat błędu przy uszkodzonym `journalLoad()` / `localStorage`
- [x] Wprowadzono wspólny model stanu dla PWM / gatunków / dni
- [x] Centralny store dla widoków `Status` / `Panel`

## Najbliższe kroki
1. Ujednolicić źródło prawdy dla tacy
   - `state.tray`, `state.timers.tray`, `deviceState.trays`, `trayMotion`
2. [x] Ograniczyć `abortCtrl` do pollingu statusu
3. [x] Wspólny model stanu dla PWM / gatunków / dni
4. [x] Poprawić reset anchorów w `updateTrayUI` tak, by nie restartował timera przy normalnym pollingu
5. [x] Usunąć globalny fallback `state.trayConfig[i]` z `computeTurnProgress()` i wymagać jawnego `opts.config`
6. [x] Zweryfikować działanie `setTraySpecies` i obsługę `/api/tray/set-species`
7. [x] Upewnić się, że `installTrayControlSync()` działa przed interakcją użytkownika (np. `DOMContentLoaded`)
8. [x] Sprawdzić cache dla `drawFalloutGauge` przy ukrytym widoku i naprawić klucz rysowania
9. [x] Rozdzielić reset stopera kalibracji od czyszczenia `measurements`
10. [x] Dodać retry/logowanie dla `fetchTrayConfig` i poprawić obsługę błędów
11. Doprecyzować dokumentację zmian i changelog

## Priorytety
- [x] Oddzielić status od komend
- [x] Timeout/abort dla fetchy
- [x] Walidacja i feedback dla komend
- [x] Błąd dziennika przy uszkodzonym `localStorage`
- [x] Centralny model stanu dla widoków

## Uwagi
- Najpierw uporządkuj logikę w jednym pliku, potem wydzielaj moduły.
- Trzymaj plan zwięzły i w kolejności działań.
