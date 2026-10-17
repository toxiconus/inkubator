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

## 6. Gatunki i fazy (jedno źródło)

- Jedyne źródło danych o gatunkach: `data/incubation_profiles.json`. Czyta je firmware
  i panel (`index.html` buduje z niego tablicę `SPECIES` przy starcie).
- Klucze gatunków są małymi literami (`chicken`, `duck_indian_runner`, `goose`, `quail`,
  `guinea_fowl`, `pheasant`). Panel dopuszcza też `id` profilu (np. `gallus_gallus_domesticus`).
- Faza: `critical_phases` – `early` (nie ruszać, drzwi zamknięte), `late` (ostrożnie,
  ostatnie obroty), `lockdown` (obracanie wyłączone, wilgotność w górę, zamknięte drzwi).
  Fazy nie mogą się nakładać – `late` kończy się dzień przed `lockdown`.
- Obracanie: `turning_frequency_per_day` (4) i flaga `turning` dla każdego dnia.
- Dni świec (candling): `candling_days` – na nich obracanie jest pomijane.
- Kolory alarmów: tylko z palety (zielony = normalnie, bursztyn = uwaga, czerwony = krytycznie).
- Pole gatunku w konfiguracji ma 24 znaki (klucz `duck_indian_runner` mieści się).

Do dodania nowego gatunku: wpis w `species` z `day_by_day` dla wszystkich dni,
`critical_phases`, `turning_frequency_per_day`, `candling_days`. Żadnych zmian w kodzie.

## 7. Tace, komora i grupy wylęgu

Stan obecny (sprawdzony w kodzie):
- Regulator (`controller.cpp`) steruje grzałką i nawilżaniem według globalnych wartości
  `target_temp` / `target_humidity` z konfiguracji. Profile dnia (`tray_controller`) liczą
  cele dla każdej tacy, ale nikt ich nie odczytuje. Dopóki to nie zostanie połączone,
  komora trzyma stałe nastawy, a nie warunki z profilu.
- Temperatura celu z profilu to `air_temp_target` (powietrze), a nie `shell_temp` (skorupa).
  Poprawione w `tray_controller.cpp`.
- Jedna komora = jedna temperatura i jedna wilgotność dla wszystkich tac. Tace mogą się różnić
  gatunkiem, dniem i obracaniem, ale nie klimatem.

Zalecany model: **taca = grupa wylęgu**. Gatunki w jednej komorze wyrównujemy po dniu wylęgu,
a nie po dniu startu (krótszy gatunek startuje później o różnicę dni inkubacji).
Wyniki z profili (wyrównanie po wylęgu):
- kura + kaczka: 0 dni rozjazdu obracania, max różnica wilgotności 8%
- kura + przepiórka: 0 dni, 0%
- kura + bażant: 0 dni, 5%
- kaczka + przepiórka: 0 dni, 8%
- kaczka + bażant: 0 dni, 3%
- kura + perliczka, gęś + perliczka: 2–3 dni rozjazdu, nie zalecane w jednej komorze.

Zasada dla wspólnej komory: temperatura 37,7 °C dla wszystkich; wilgotność celu = średnia
z aktywnych tac, a w lockdown: najwyższa z nich. Gdy rozjazd obracania > 0, panel ostrzega.

Wartości zweryfikowane z NCSU (Egg Temp Chart): temperatura 100 °F (37,8 °C) cały czas,
wilgotność wylęgu 45–55%, lockdown 55–65%, stop obracania: kura 18, kaczka 25, perliczka 25,
przepiórka 14 (dla Coturnix). Nasze profile: wilgotność lockdown 65–75% (z innych źródeł) –
do decyzji, czy zejść do 55–65%.
