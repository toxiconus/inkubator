# Projekt Inkubator – Plan migracji do docelowej struktury

**Data:** 2026-08-02  
**Status:** W trakcie realizacji  
**Platforma:** ESP32-S3 + Web HMI

## Cel
Przywrócić i usprawnić plan migracji dla frontendowej części projektu, zachowując strukturę dokumentacji i robiąc porządek w katalogach.

## Najważniejsze obszary do naprawy

1. `inkubator-new/docs/plan.md` musi zostać przywrócony do czytelnej postaci.
2. `inkubator-new/firmware/data/index.html` musi mieć poprawną kolejność skryptów i zakładkę `LĘG`.
3. `inkubator-new/firmware/data/ws-utils.js` powinien obsługiwać WebSocket z reconnectem.
4. `inkubator-new/firmware/data/js/common.js` powinien zawierać moduły `Inkubator.Events`, `Inkubator.State`, `Inkubator.UI`, `Inkubator.Storage`, `Inkubator.Utils`, `Inkubator.Config`.
5. Zrobić porządek katalogów: `scripts/`, `backups/`, `web/` i usunąć pliki tymczasowe.

## Obecna sytuacja
- Projekt działa jako `inkubator-new/` z podkatalogiem `firmware/`.
- Frontend znajduje się w `inkubator-new/firmware/data/`.
- Dokument planu został uszkodzony i wymaga pełnej rekonstruacji.
- Istnieje kopia `inkubator-new — kopia/`, którą warto przenieść do backupów.

## Zadania do wykonania

### A. Naprawa dokumentu planu
- Zastąpić zawartość `inkubator-new/docs/plan.md` poprawnym planem.
- Utrzymać listę plików i katalogów do weryfikacji.
- Dodać proste kryteria zakończenia.

### B. Weryfikacja frontendu
- Sprawdzić, że w `inkubator-new/firmware/data/index.html`:
  - `ws-utils.js` ładuje się przed `js/common.js`
  - w nawigacji jest zakładka `LĘG`
  - istnieje sekcja widoku `view-hatch`
- Zweryfikować, że `ws-utils.js` obsługuje:
  - reconnect
  - parser JSON/KEY=VALUE
  - event bus
- Upewnić się, że `common.js` zawiera rozszerzone moduły stanu i UI.

### C. Porządkowanie struktury katalogów
- Utworzyć katalog `scripts/` i przenieść tam narzędzia diagnostyczne.
- Utworzyć katalog `backups/` i przenieść tam `inkubator-new — kopia/`.
- Zsynchronizować `inkubator-new/web/` z `inkubator-new/firmware/data/`.
- Usunąć pliki tymczasowe: `serial_mpu_out.txt`, `stdout`, `vscode-storage-report.txt`.

## Lista najważniejszych plików
- `inkubator-new/docs/plan.md`
- `inkubator-new/firmware/data/index.html`
- `inkubator-new/firmware/data/ws-utils.js`
- `inkubator-new/firmware/data/js/common.js`
- `inkubator-new/firmware/data/js/hatch.js`
- `inkubator-new/firmware/data/css/style.css`
- `inkubator-new/firmware/data/css/responsive.css`
- `inkubator-new/firmware/data/js/dashboard.js`

## Kryteria zakończenia
- `inkubator-new/docs/plan.md` ma czytelny, poprawny plan.
- `index.html` i skrypty ładowane są w poprawnej kolejności.
- `ws-utils.js` ma reconnect i event bus.
- `common.js` ma zdefiniowane moduły `Inkubator.*`.
- `scripts/`, `backups/` i `web/` są porządnie zorganizowane.
- Pliki tymczasowe zostały usunięte.
