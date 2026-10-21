# Inkubator New — Prompt / Change Journal

## Cel
Ten plik służy jako centralny prompt dla AI i repozytorium `inkubator-new`.
Będzie tu zapisywana logika zmian, wymagania, stan plików oraz wszystkie przyszłe instrukcje do dalszej pracy.

## Jak używać
- Każda nowa prośba użytkownika powinna być zapisana tutaj jako kolejny punkt albo sekcja.
- Jeśli trzeba, wkleja się tu fragmenty plików, które mają zostać zmodyfikowane, albo pełne pliki.
- Po każdym kroku AI może aktualizować ten dokument, aby utrzymać kontekst i historię zmian.
- Zawsze zapisuj tutaj:
  - nazwy plików, które wymagają zmiany,
  - krótki opis zakresu modyfikacji,
  - konkretną treść lub fragmenty, jeśli użytkownik je poda.
- Jeśli nowy wpis jest prosty, możesz od razu wprowadzić poprawki w plikach.
- Jednocześnie czekaj na nowsze wersje plików: nie nadpisuj całościowych plików, jeśli użytkownik zapowiada dalsze uzupełnienia.
- W kolejnych uzupełnieniach traktuj ten plik jako żywy prompt/środek do zapisu wymagań i decyzji.

## Pliki do śledzenia w `inkubator-new`
- `inkubator-new/docs/mysli.txt`
- `inkubator-new/prompt.md`
- `inkubator-new/steps.md`
- `inkubator-new/firmware/src/main.cpp`
- `inkubator-new/firmware/src/config_manager.h`
- `inkubator-new/firmware/src/controller.cpp`
- `inkubator-new/firmware/src/tray_controller.cpp`
- `inkubator-new/firmware/src/alarm_manager.cpp` (proponowany)
- `inkubator-new/web/index.html`
- `inkubator-new/firmware/data/index.html`

## Struktura pliku
1. Cel i zakres zmian
2. Lista plików do edycji
3. Notatki i obserwacje
4. Stan zadań / TODO
5. Zawartość plików (opcjonalnie)

## Aktualny stan
- Projekt: `inkubator-new`
- Główny plik referencyjny UI: `inkubator-new/docs/mysli.txt`
- Istniejący plan: `inkubator-new/steps.md`
- Cel użytkownika: mieć plik promptu, który służy jako punkt startowy dla kolejnych zmian i uzupełnień.

## Zadania
- [x] Utworzyć centralny plik prompt w `inkubator-new/prompt.md`
- [x] Przeczytać `inkubator-new/docs/mysli.txt` i wyciągnąć z niego aktualne potrzeby
- [x] Dodać endpoint `/api/profiles` w `inkubator-new/firmware/src/main.cpp`
- [x] Zaktualizować `inkubator-new/web/incubation.html` do dynamicznego ładowania profili
- [x] Dodać obsługę `start_unix` i `resetDay` w `tray_controller.cpp` i `main.cpp`
- [ ] Przejrzeć `alarm_manager.cpp` i poprawić brakujące deklaracje oraz integrację PWM
- [x] Naprawić `salvarVentilador` tak, aby zapisywał `fan_delay_sec`
- [x] Sprawdzić `carregarConfiguracao*` w WebSocket, by wysyłały aktualne dane konfiguracji
- [ ] Ujednolicić język w frontendzie/backendzie oraz usunąć mieszankę polsko-portugalsko-angielską

## Bieżący kontekst z `inkubator-new/docs/mysli.txt`
Wnioski z nowych danych:
- Najważniejszy problem to trwałe przechowywanie dnia inkubacji po restarcie. Rozwiązanie: `start_unix` + RTC.
- `TrayConfig.start_unix` istnieje w `config_manager.h` i jest już serializowany w `config_manager.cpp`.
- Implementowano `tray_update_days_from_rtc()` oraz WebSocketowe komendy `setStartUnix:tray:unix`, `resetDay:tray`, `getStartUnix:tray`.
- `incubation.html` otrzymało pola daty startu dla obu tac i przyciski Ustaw/Reset.
- `main.cpp` wysyła teraz także `tray0_start_unix` i `tray1_start_unix` w broadcastzie statusu.
- `mysli.txt` zawiera teraz kompletny przykład `main.cpp` z pełnym WebSocket/RTC i `webServer.on("/api/profiles")`.
- `mysli.txt` zawiera także przykład `tray_controller.cpp` z funkcjami `tray_update_days_from_rtc`, `tray_set_start_unix`, `tray_reset_day`, `tray_get_start_unix`.
- `alarm_manager.cpp` jest uwzględniony w `mysli.txt` jako moduł sprawdzający alarmy temperatury, wilgotności i historię PWM.
- `mysli.txt` zawiera pełną listę nowych, spójnych komend WebSocket oraz ich aliasy do starych poleceń.
- Nowy front-end powinien używać `setStartDate:` zamiast `setStartUnix:`, ale oba są obsługiwane jako aliasy.
- Endpoint `/api/profiles` jest już dostępny i front-end ładuje profile z JSON.
- `mysli.txt` dodatkowo potwierdza, że komunikacja między ESP a przeglądarką jest teraz czytelna i łatwiejsza w utrzymaniu.
- Wciąż wymaga weryfikacji: alarmy, sterowanie wentylatorem/diffuserem i konsystencja komunikatów WebSocket.

## Nowe komendy WebSocket
Tabela w `mysli.txt` definiuje nowe, spójne nazwy komend oraz istniejące aliasy:

| Grupa | Nowa komenda | Stara komenda (alias) |
| --- | --- | --- |
| Temperatura | `getTempConfig` | `carregarConfiguracao` |
| Temperatura | `setTemp:` | `setpoint:` |
| Temperatura | `setHysteresis:` | `histerese:` |
| Temperatura | `setAlarmMaxTemp:` | `alarmMax:` |
| Temperatura | `setAlarmMinTemp:` | `alarmMin:` |
| Temperatura | `setTempCalibration:` | `calibracao:` |
| Temperatura | `setControlMode:` | `mode:` |
| Temperatura | `setFanDelay:` | `salvarVentilador:` |
| Temperatura | `resetMinMax` | `limparMinMax` |
| Wilgotność | `getHumidityConfig` | `carregarConfiguracaoUmidade` |
| Wilgotność | `setHumidity:` | `setpointUmidade:` |
| Wilgotność | `setHumidityHysteresis:` | `histereseUmidade:` |
| Wilgotność | `setHumidityAlarmMax:` | `alarmMaxUmidade:` |
| Wilgotność | `setHumidityAlarmMin:` | `alarmMinUmidade:` |
| Wilgotność | `setHumidityCalibration:` | `calibracaoUmidade:` |
| Wilgotność | `setPumpTime:` | `tempoBomba:` |
| Wilgotność | `setWaterCheckInterval:` | `intervaloVerificacaoAgua:` |
| Obrót | `getTurnerConfig` | `carregarConfiguracaoVirador` |
| Obrót | `setTurnerMode0:` | `modoVirador0:` |
| Obrót | `setTurnerWorkTime0:` | `tempoTrabalho0:` |
| Obrót | `setTurnerRestTime0:` | `tempoRepouso0:` |
| Obrót | `setTurnerPwm0:` | `pwm0:` |
| Obrót | `setTurnerMode1:` | `modoVirador1:` |
| Obrót | `setTurnerWorkTime1:` | `tempoTrabalho1:` |
| Obrót | `setTurnerRestTime1:` | `tempoRepouso1:` |
| Obrót | `setTurnerPwm1:` | `pwm1:` |
| Obrót | `turnerStart:` | `virador:start` |
| Obrót | `turnerStop:` | `virador:stop` |
| Inkubacja | `getIncubationConfig` | `carregarConfiguracaoIncubacao` |
| Inkubacja | `setStartDate:` | (nowa) |
| Inkubacja | `resetDay:` | (nowa) |
| Inkubacja | `getStartDate:` | (nowa) |
| Alarmy | `getAlarms` | `carregarAlarmes` |
| Alarmy | `setAlarms:` | `salvarAlarmes:` |
| Wi-Fi | `getWifiConfig` | `carregarRede` |
| Wi-Fi | `getWifiStatus` | `status` |
| Wi-Fi | `setWifi:` | `salvarRede:` |
| Wi-Fi | `removeWifi` | `removerRede` |
| RTC | `getRtcTime` | `carregarRTC` |
| RTC | `syncNtp` | `sincronizarNTP` |
| RTC | `setRtcTime:` | `rtcSet:` |
| RTC | `getRtcStatus` | `rtcStatus` |
| Inne | `getFirmwareVersion` | `versaoFirmware` |
| Inne | `setLanguage:` | `idioma:` |
| Inne | `getLanguage` | `carregarIdioma` |
| Inne | `factoryReset` | `redefinir` |
| Inne | `reboot` | `reiniciar` |

## Potrzebne elementy do następnego promptu
1. Wyraźne zadania do AI oparte na informacjach z `mysli.txt`:
   - Co dokładnie poprawić w `main.cpp` i `config_manager.h`
   - Jak uporządkować parser WebSocket
   - Które profile inkubacyjne należy sprawdzić i zaktualizować
   - Jak poprawić UI sygnalizację połączenia
   - Jakie zabezpieczenia dodać dla odczytów sensorów
2. Utrzymać zasadę: na razie nie kodować innych plików, tylko zapisać wymagania i plan zmian
3. Jeśli użytkownik poda nowe dane, dopisać je tutaj jako następny wpis

## Kroki bez testów
- Spisać dokładne poprawki dla `alarm_manager.cpp` i w `main.cpp` bez wdrażania ich od razu.
- Zidentyfikować brakujące lub niespójne komendy WebSocket, bazując na `mysli.txt`.
- Uporządkować listę plików i danych, które trzeba jeszcze przejrzeć.
- Zanotować w promptcie, co już zostało zrobione, a co trzeba dalej zaplanować.

## Aktualny prompt
Przeczytaj `inkubator-new/docs/mysli.txt` i przygotuj listę zmian oraz potrzebnych uzupełnień dla projektu `inkubator-new`.
Zachowaj wyraźne odniesienia do plików w `inkubator-new`, zwłaszcza:
- `inkubator-new/docs/mysli.txt`
- `inkubator-new/steps.md`
- `inkubator-new/firmware/src/main.cpp`
- `inkubator-new/firmware/src/config_manager.h`
- `inkubator-new/firmware/src/tray_controller.cpp`
- `inkubator-new/web/index.html`
- `inkubator-new/firmware/data/index.html`

## Przykład wpisu
### Wpis #1
- Data: 2026-07-07
- Żądanie: "utwórz nowy plik który bedzie dla ciebie potem promptem do zmian"
- Działanie: stworzono `inkubator-new/prompt.md`
- Kolejny krok: czekać na uzupełnienie promptu nowymi poleceniami

## Miejsce na zawartość plików
### Plik: inkubator-new/docs/mysli.txt
<tu można wklejać fragmenty lub pełną zawartość w razie potrzeby>

### Plik: inkubator-new/web/index.html

### Plik: inkubator-new/firmware/src/main.cpp

## Historia zmian
- 2026-07-07: utworzono plik promptu dla przyszłych zmian i notatek.
- 2026-07-07: dodano nowe dane z `inkubator-new/docs/mysli.txt` i zaktualizowano kontekst promptu o szczegóły dotyczące WebSocket, konfiguracji i brakujących komend.
- 2026-07-07: uzupełniono prompt o nowe dane dotyczące `heaters_control` i propozycję utworzenia `alarm_manager.cpp`.

## Plan migracji frontendu
- Najpierw zaktualizować `inkubator-new/web/temperature.html` do nowych komend temperatury:
  - `getTempConfig`, `setTemp:`, `setHysteresis:`, `setAlarmMaxTemp:`, `setAlarmMinTemp:`, `setTempCalibration:`, `setControlMode:`, `setFanDelay:`, `resetMinMax`
- Następnie zaktualizować `inkubator-new/web/humidity.html` do nowych komend wilgotności:
  - `getHumidityConfig`, `setHumidity:`, `setHumidityHysteresis:`, `setHumidityAlarmMax:`, `setHumidityAlarmMin:`, `setHumidityCalibration:`, `setPumpTime:`, `setWaterCheckInterval:`
- Potem zaktualizować `inkubator-new/web/turner.html` (lub odpowiedni plik obsługi obrotu) do nowych komend obrotu:
  - `getTurnerConfig`, `setTurnerMode0:`, `setTurnerWorkTime0:`, `setTurnerRestTime0:`, `setTurnerPwm0:`, `setTurnerMode1:`, `setTurnerWorkTime1:`, `setTurnerRestTime1:`, `setTurnerPwm1:`, `turnerStart:`, `turnerStop:`
- Zaktualizować `inkubator-new/web/incubation.html` do `setStartDate:`, `resetDay:` i `getStartDate:` przy zachowaniu aliasu `setStartUnix:`.
- Zachować starsze aliasy jako kompatybilne przejściowo, ale zaznaczyć w kodzie, że są przestarzałe.
- Na koniec przesłonić logikę WebSocket w `main.cpp`, aby nowa lista komend była źródłem prawdy, a stare aliasy działały tylko jako fallback.
- W frontendzie zaktualizować `ws.onmessage` i `saveConfig()` do używania nowych nazw komend jako priorytetowych w `temperature.html`, `humidity.html` i `incubation.html`.
- Dodać bezpośrednie mapowanie w `ws.onmessage`:
  - `setTemp:` / `setpoint:`
  - `setHysteresis:` / `histerese:`
  - `setAlarmMaxTemp:` / `alarmMax:`
  - `setAlarmMinTemp:` / `alarmMin:`
  - `setTempCalibration:` / `calibracao:`
  - `setControlMode:` / `mode:`
  - `setFanDelay:` / `salvarVentilador:`
  - `resetMinMax` / `limparMinMax`
  - `setHumidity:` / `setpointUmidade:`
  - `setHumidityHysteresis:` / `histereseUmidade:`
  - `setHumidityAlarmMax:` / `alarmMaxUmidade:`
  - `setHumidityAlarmMin:` / `alarmMinUmidade:`
  - `setHumidityCalibration:` / `calibracaoUmidade:`
  - `setPumpTime:` / `tempoBomba:`
  - `setWaterCheckInterval:` / `intervaloVerificacaoAgua:`
  - `setStartDate:` / `setStartUnix:`
  - `resetDay:` / (alias: `resetDay:`)
  - `getStartDate:` / `getStartUnix:`
