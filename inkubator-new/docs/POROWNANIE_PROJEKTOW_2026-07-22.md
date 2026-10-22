# Porownanie projektow: `J:\inkubator\inkubator-new` vs `J:\stare inkubatory\inkubator-new`

Data analizy: 2026-07-22

## 1) Zakres i metoda
- Porownano komplet struktur plikow obu katalogow.
- Dla plikow wspolnych (poza `firmware/.pio`) policzono hash SHA-256 i wykryto realne roznice tresci.
- Zweryfikowano osobno rdzen firmware (`firmware/src`, `platformio.ini`) i dokumentacje (`docs`, `prompt.md`, `steps.md`).

## 2) Szybkie podsumowanie
- Kopia w `J:\inkubator\inkubator-new` (dalej: **A**) jest lepsza jako baza rozwojowa (czytelniejsza, modularna warstwa UI, oddzielone JS/CSS).
- Kopia w `J:\stare inkubatory\inkubator-new` (dalej: **B**) ma wartosc glownie archiwalna / diagnostyczna, bo zawiera gotowe artefakty builda (`firmware/.pio/build/**`).
- Rdzen firmware i konfiguracja kompilacji sa praktycznie takie same merytorycznie (brak roznic w `firmware/src`, identyczne `platformio.ini`).

## 3) Twarde dane
- Liczba plikow: A = 483, B = 697
- Pliki tylko w A: 22
- Pliki tylko w B: 236
- Wspolne pliki (bez `firmware/.pio`): 61
- Rozna tresc wsrod wspolnych (bez `firmware/.pio`): 13

### 3.1 Tylko w A (najwazniejsze)
- `firmware/css/style.css`
- `firmware/data/css/style.css`
- `firmware/data/js/common.js`
- `firmware/data/js/pages/*.js` (oddzielne skrypty per strona)
- `firmware/js/common.js`
- `firmware/js/pages/*.js`

Wniosek: A ma rozdzielona warstwe prezentacji i logiki (duzo lepsze utrzymanie i dalszy rozwoj).

### 3.2 Tylko w B (najwazniejsze)
- `firmware/.pio/build/**` (setki plikow .o/.d/.a + `firmware.bin`, `firmware.elf`, `firmware.map`, `bootloader.bin`)

Wniosek: B przechowuje lokalny stan kompilacji i artefakty, ale to nie jest przewaga kodowa produktu.

## 4) Co jest identyczne
- `firmware/src` -> 15/15 plikow, roznice tresci: 0
- `firmware/platformio.ini` -> identyczny
- `docs` -> 5/5 plikow, roznice tresci: 0
- `prompt.md` -> identyczny
- `steps.md` -> identyczny

Wniosek: logika firmware C++ i glowna dokumentacja sa zgodne w obu kopiach.

## 5) Co sie realnie rozni (merytorycznie)
Roznice tresci (poza buildem) dotycza glownie:
- `firmware/data/*.html` (11 stron)
- `firmware/.specstory/history/...`
- `firmware/.specstory/statistics.json`

### 5.1 Charakter roznic w HTML
W A strony sa rozbudowane i modularne:
- odwolania do zewnetrznych zasobow (`/css/style.css`, `/js/common.js`, `/js/pages/...`)
- wieksza czytelnosc i mozliwosc ponownego uzycia
- mniej duplikacji

W B strony sa krotsze / bardziej monolityczne:
- zwykle brak zewnetrznych `script/link` (logika i styl bardziej „w srodku” lub uproszczone widoki)
- mniejsza elastycznosc przy rozwoju

Przyklad (`various.html`):
- A: pelny ekran ustawien (RTC/NTP, jezyk, OTA, zarzadzanie), modularne skrypty.
- B: uproszczony panel RTC/system bez pelnego rozdzialu zasobow.

## 6) Co lepsze w A
- Lepsza architektura frontendu (separacja HTML/CSS/JS).
- Lepsza skalowalnosc (oddzielne pliki JS na strony).
- Latwiejsze debugowanie i maintenance.
- Mniej „szumu” buildowego w drzewie porownania.

## 7) Co lepsze w B
- Zawiera artefakty kompilacji i linkowania przydatne czasem do:
  - analizy mapy pamieci (`firmware.map`),
  - porownan binarnych,
  - szybkiej inspekcji stanu builda z danego momentu.

## 8) Rekomendacja praktyczna
1. Za glowna baze przyjac **A** (`J:\inkubator\inkubator-new`).
2. Z **B** ewentualnie przeniesc tylko to, co diagnostyczne (np. `firmware.map`), jesli naprawde potrzebne.
3. Nie mieszac stale `firmware/.pio/build/**` do kodu roboczego.
4. Jezeli chcesz trwalego porzadku: ustawic ignorowanie builda i traktowac B jako archiwum.

## 9) Krotkie uzasadnienie biznesowe
- Poniewaz firmware C++ jest zgodny, przewaga A wynika z lepszej warstwy UI i utrzymywalnosci.
- B daje korzysc glownie historyczno-diagnostyczna, nie rozwojowa.

---

### Dodatkowa obserwacja
W poprzednich notatkach repo pojawia sie zasada „single source” dla UI (unikanie rozjazdu kopii). Wynik porownania potwierdza, ze kierunek A jest zgodny z ta praktyka.

## 10) Metryki stron HTML (A vs B)
Interpretacja:
- `extRefs`: liczba odniesien do zewnetrznych `script/link`.
- `inlineScript`: liczba skryptow inline.
- `styleTags`: liczba sekcji `<style>`.

| Plik | Rozmiar A/B (B) | extRefs A/B | inlineScript A/B | styleTags A/B |
|---|---:|---:|---:|---:|
| ajustes.html | 2688 / 819 | 3 / 0 | 0 / 0 | 0 / 0 |
| alarms.html | 4764 / 754 | 3 / 0 | 0 / 0 | 1 / 0 |
| console.html | 3770 / 1117 | 3 / 0 | 0 / 0 | 1 / 0 |
| humidity.html | 6296 / 1611 | 3 / 0 | 0 / 0 | 1 / 0 |
| incubation.html | 1949 / 1537 | 3 / 0 | 0 / 0 | 0 / 0 |
| index.html | 124230 / 124188 | 1 / 0 | 1 / 1 | 1 / 1 |
| symulacja_legow.html* | 14547 / 51028 | 3 / 1 | 0 / 1 | 1 / 1 |
| temperature.html | 6449 / 1263 | 3 / 0 | 0 / 0 | 1 / 0 |
| turner.html | 9408 / 1672 | 3 / 0 | 0 / 0 | 1 / 0 |
| various.html | 6898 / 1384 | 3 / 0 | 0 / 0 | 1 / 0 |
| wifi.html | 2929 / 1322 | 3 / 0 | 0 / 0 | 0 / 0 |

\* W drzewie plikow wystepuje nazwa z polskim znakiem (`symulacja_legów.html`).
