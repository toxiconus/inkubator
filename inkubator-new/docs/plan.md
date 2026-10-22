# Projekt Inkubator - Plan migracji do docelowej struktury

Data: 2026-08-02
Status: W trakcie realizacji
Platforma: ESP32-S3 + Web HMI

## Cel
Przywrócenie planu migracji i uporzadkowanie struktury dokumentacji oraz frontendu.

## Zakres
- Przywrocenie `inkubator-new/docs/plan.md`
- Weryfikacja `firmware/data/index.html` (kolejnosc skryptow, zakladka LEG, view-hatch)
- Weryfikacja `firmware/data/ws-utils.js` (reconnect, event bus, parser)
- Rozszerzenie `firmware/data/js/common.js` o `Inkubator.*`
- Utworzenie lub weryfikacja katalogow `scripts/`, `backups/`, `web/`
- Usuniecie plikow tymczasowych

## Status
- Plan migracji uszkodzony, naprawa priorytetowa.
- Dokumentacja w `inkubator-new/docs` do zachowania.
- Frontend w `inkubator-new/firmware/data` do weryfikacji.

## Kroki
1. Odbudowac plan w `inkubator-new/docs/plan.md`.
2. Sprawdzic kolejnosc ladowania `ws-utils.js` przed `js/common.js`.
3. Sprawdzic obecnosci zakladki LEG i widoku `view-hatch`.
4. Sprawdzic `ws-utils.js` z reconnect i event bus.
5. Sprawdzic `common.js` moduy `Inkubator.Events`, `Inkubator.State`, `Inkubator.UI`, `Inkubator.Storage`, `Inkubator.Utils`, `Inkubator.Config`.
6. Utworzyc katalogi `scripts/` i `backups/`.
7. Zsynchronizowac `inkubator-new/web/` z `inkubator-new/firmware/data/`.
8. Usunac pliki tymczasowe: serial_mpu_out.txt, stdout, vscode-storage-report.txt.

## Kryteria zakonczenia
- `plan.md` jest czytelny i kompletny.
- Kolejnosc skryptow w `index.html` jest poprawna.
- `ws-utils.js` obsluguje reconnect i event bus.
- `common.js` ma zdefiniowane moduy.
- Katalogi `scripts/`, `backups/`, `web/` sa zorganizowane.
- Pliki tymczasowe zostaly usuniete.
