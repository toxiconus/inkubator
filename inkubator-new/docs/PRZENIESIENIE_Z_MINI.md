# Przeniesienie funkcji z `inkubator-mini` do `inkubator-new`

Bazą jest `inkubator-new` (modularny kod, panel, GitHub `master`). Folder `inkubator-mini` został usunięty z repo
po przeniesieniu wartościowych funkcji. Dawny kod `mini` nadal jest w historii gita
(commit `b108a93`, ścieżka `inkubator-mini/`) oraz w archiwum `archiwum_inkubator_2026-10-21.zip`
(razem ze zrzutami flash `.bin`).

**Stan weryfikacji:** kod został zbudowany (PlatformIO, ESP32-S3, Flash 69,6%, RAM 15,7%), ale nie był wgrywany
na urządzenie ani testowany na sprzęcie. Nowe tryby sterowania są domyślnie wyłączone.

## 1. Co zostało przeniesione

| Funkcja | Gdzie | Uwagi |
| --- | --- | --- |
| Regulator adaptacyjny z uczeniem (moc nauczona, adaptacja do różnicy dwóch czujników, uczenie cyklu dyfuzora) | `src/controller.cpp`, tryb 1 | Dostosowany do `new`: zakres wilgotności z konfiguracji, górny czujnik tylko gdy skonfigurowany i nie jest czujnikiem akwarium |
| Nagrywanie próbek do uczenia | `handle_learning_record()` w pętli głównej | W `new` te funkcje były pustymi stubami |
| Sterowanie tacami przez HTTP | `POST /api/tray/set-day`, `set-start`, `set-species`, `set-active`, `move`, `stop` | Wrappery na istniejące `tray_*`; `set-species` sprawdza, czy profil istnieje |
| Konfiguracja przez HTTP | `GET/POST /api/config` | `POST` z `{"factory_reset":true}` przywraca ustawienia fabryczne; po zapisie potrzebny restart |
| Restart | `POST /api/reboot` | |
| Uczenie regulatora | `GET/POST /api/heater/learning`, `POST /api/heater/learning/reset` | Tryby `on`, `off`, `start_record`, `stop_record` |
| Wybór regulatora | `GET/POST /api/heater/mode` (`mode=0\|1\|2`) | Także komenda WebSocket `setControlMode` |

Endpointy zmieniające stan są tylko `POST` i, tak jak WebSocket, nie mają uwierzytelniania. Zabezpiecza je hasło sieci Wi-Fi.

## 2. Problem ze skalą PWM

W kodzie były trzy różne skale mocy:

- sprzęt (LEDC, 25 kHz): **0..1023** (10 bitów),
- panel dzielił surową wartość przez **4095**, więc przy pełnej mocy pokazywał tylko ok. 25%,
- silniki obrotu tac: **0..255**.

Skutek: wskaźnik „G1/G2” w panelu pokazywał za niską moc grzałki, więc mogło wyglądać, że jest zapas mocy,
choć wyjście było bliskie 100%.

Zmiany:

- w całym kodzie sterowania moc jest w **procentach** (float 0..100), a skalę sprzętu zna tylko `heater_write_pct()`,
- `heater_write_pct()` rozkłada ułamkową część wypełnienia w czasie (dithering), więc efektywna rozdzielczość
  jest dużo lepsza niż 1/1023,
- status (`/api/status` i WebSocket) zawiera `heater_pct` (procenty) i `pwm_max`, a stare pola `heater_pwm_ch0/1`
  zostały dla zgodności (poprawiony błąd obcinania `(int)heater_pwm_pct * 10.23`),
- panel liczy moc z `heater_pct`, a bez niego dzieli surową wartość przez `pwm_max`.

Silniki obrotu tac zostają w skali 0..255 (to osobne wyjście, nie grzałka).

## 3. Dlaczego temperatura „wisiała” poniżej celu

Tryb 0 (prosty regulator) działa tak: moc rośnie o 5% na cykl tylko wtedy, gdy temperatura jest niższa niż
`cel − histereza` (domyślnie 0,5 °C), a w paśmie histerezy moc powoli maleje. Temperatura krąży więc w okolicy dolnej
krawędzi pasma, czyli ok. 37,2 °C przy celu 37,7 °C, mimo wolnych zasobów mocy. To cecha algorytmu, a nie błąd czujnika.

Tryb 2 (PI) usuwa ten efekt:

- człon całkujący dobiera moc ustaloną, więc nie ma stałego uchybu,
- ograniczenie przeciwdziałające nawijaniu całki (całka rośnie tylko, gdy wyjście nie jest nasycone),
- jeśli uczenie ma zapisaną moc dla danej nastawy, całka startuje od niej,
- zabezpieczenia: odcięcie powyżej `cel + 0,4 °C` i powyżej `alarm_max_temp`, odcięcie przy błędnym odczycie, łagodny start po włączeniu.

Parametry (`PI_KP = 15 %/°C`, `PI_KI = 0,05 %/(°C·s)`, czas zdwojenia ok. 5 minut) są ostrożne i wymagają dostrojenia na sprzęcie.
Włączenie: `POST /api/heater/mode` z `mode=2` albo komenda `setControlMode:2`. Powrót: `mode=0`.

## 4. Zalecenia dla silikonowych grzałek 12 V (mała bezwładność)

- **Szybszy pomiar.** Dziś czujniki są odczytywane co 5 s, a sterowanie wykonuje się co 2 s na tym samym odczycie.
  Przy małej bezwładności grzałki to za wolno i sprzyja oscylacjom. Warto czytać DS18B20 co ok. 1 s w trybie
  nieblokującym (konwersja 12-bitowa trwa ok. 750 ms) i sterować w tym samym rytmie.
- **Rozdzielczość.** Przy 25 kHz LEDC ma najwyżej 10–11 bitów. Dithering daje dokładniejszą średnią moc bez zmiany częstotliwości.
  Gdyby 25 kHz nie było potrzebne dla grzałek, obniżenie częstotliwości daje więcej natywnych bitów
  (np. 13 bitów przy 9 kHz, 14 bitów przy 4,8 kHz), ale może być słyszalne.
- **Dobór mocy.** Zbyt duża grzałka pracuje przy kilku procentach mocy, więc drobny krok PWM zmienia dużo. Przy dużym zapasie
  mocy warto ograniczyć maksymalne wypełnienie lub użyć mocy dobranej do potrzeb.
- **Położenie czujnika.** Czujnik blisko grzałki reaguje szybko, ale pokazuje temperaturę grzałki, a nie komory.
  Do regulacji lepszy jest czujnik w komorze, na wysokości jaj.

## 5. Czego nie przeniesiono

Zostało w historii gita (`inkubator-mini/` w commicie `b108a93`):

- historia w pamięci RAM i eksport `/api/history/csv` (w `new` jest log CSV w `GET /api/log`),
- dziennik (`/api/journal*`) zapisywany w LittleFS,
- strona wgrywania plików do LittleFS (`/upload`); w `new` jest aktualizacja OTA pod `/update`,
- profile dyfuzora i profil grzałki z `mini` (w `mini` parametry rozmyte nie były używane przez regulator),
- odczyt kąta tacy z MPU6050 (`/api/tray/angle`),
- stare wersje panelu (`index.html` 322 KB, `start.html`, `temperature.html`).

## 6. Znane ograniczenia

- Nazwa gatunku w konfiguracji tacy ma 11 znaków. Identyfikator `duck_indian_runner` się nie mieści, więc
  `set-species` zwróci błąd dla tego gatunku. Zmiana rozmiaru pola wymaga migracji zapisanej konfiguracji.
- Funkcje `tray_get_chamber_temp()` i `tray_get_chamber_humidity()` zwracają stałe wartości. Dlatego regulator
  bierze nastawy z konfiguracji (`target_temp`, `target_humidity`), a nie z profilu gatunku.
- Zapisana wcześniej konfiguracja z `control_mode = 1` po aktualizacji uruchomi tryb adaptacyjny.
  Sprawdź `GET /api/heater/mode` przed pierwszym uruchomieniem na urządzeniu.
