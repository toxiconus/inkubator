# DESIGN_NOTES.md – spójność UI i protokołu

Ten plik opisuje zasady, których trzymamy się w `data/` (frontend) i `src/` (firmware).
Jeśli zmieniasz kolor, odpowiedź WebSocket albo strukturę strony – aktualizuj też tutaj.

## 1. Wygląd (jedna paleta)

- Źródło prawdy: `:root` w `data/css/style.css` (tokeny `--bg`, `--panel`, `--line`,
  `--text`, `--text-2`, `--text-3`, `--amber`, `--ok`, `--err`, `--info`).
- `data/index.html` używa tych samych wartości w swoim `<style>` (ma własny układ pełnoekranowy).
  Zmieniając paletę, zmień ją w obu miejscach.
- Akcent (przyciski główne, aktywna zakładka, ostrzeżenia): bursztyn `#e3a04a`.
- Stany: zielony `#8fbf8f` = OK, czerwony `#e0786b` = błąd/alarm, bursztyn = uwaga,
  niebieski `#7fa6c4` = informacja.
- Bez emoji w interfejsie. Dozwolone symbole tekstowe: ✓ ✗ ✕ ⚠ ▶ ℹ →.
- Liczby i odczyty: czcionka monospace. Etykiety i przyciski: systemowa bezszeryfowa.
- Przełączniki: `input[type=checkbox]` z `appearance: none` (natywny checkbox ignoruje tło).

## 2. Struktura stron

Wspólny szkielet: `header` → `.instrument` → sekcje `form-group` / `btn-group` → `status-bar`.
Lokalny `<style>` na stronie zawiera tylko układ specyficzny dla tej strony.

## 3. Protokół WebSocket (ESP ↔ przeglądarka)

- Komenda: tekst `nazwaKomendy` lub `nazwaKomendy:argumenty`.
- Odpowiedź sukcesu: `ok:nazwaKomendy[:dane]`.
- Odpowiedź błędu: `err:nazwaKomendy:powód`, np. `err:salvarRede:notImplemented`.
- `common.js` pokazuje każdą odpowiedź `err:` w pasku statusu (czerwonym kolorem).
- Zasada: firmware **nie zwraca `ok:` dla operacji, której nie wykonuje**. Zamiast tego
  zwraca `err:...:notImplemented`. Zaszytych danych (np. stałego SSID) nie zwracamy jako stanu.

Stan obecny:
- Zaimplementowane: ustawienia temperatury/wilgotności/obracania, RTC, inkubacja.
- `notImplemented` (zwracane świadomie): zapis i odczyt alarmów `salvarAlarmes`/`carregarAlarmes`,
  Wi‑Fi `salvarRede`/`carregarRede`/`removerRede`.
- `wifiStatus:ap_only` – urządzenie działa tylko jako punkt dostępu (AP), bez trybu stacji.

## 4. Połączenie (common.js)

- Ponawianie wykładnicze: 1 s → max 10 s, bez limitu prób.
- Watchdog co 30 s wywołuje tylko `wsConnect()` (nie zeruje licznika).
- Ręczne wymuszenie: kliknięcie kropki lub napisu statusu.

## 5. Otwarte tematy (kolejność proponowana)

1. Wi‑Fi STA: zapis SSID/hasła w `Preferences`, tryb `WIFI_AP_STA`, status połączenia.
2. Alarmy: 10 flag (włącz/wyłącz) w `config_manager` i obsługa `carregarAlarmes`/`salvarAlarmes`.
3. `ws-utils.js` i `common.js` mają własne funkcje adresu WebSocket – połączyć w jedną.
4. `index.html` ma swoje duplikaty tokenów; po ustaleniu układu można go podpiąć pod `style.css`.
5. Kompilacja i test na sprzęcie: zmiany w `src/` nie były kompilowane w tej sesji (brak PlatformIO).
6. Rozmiar `index.html` (~128 KB): sprawdzić, czy mieści się w partycji LittleFS.
