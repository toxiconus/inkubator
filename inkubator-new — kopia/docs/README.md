🥚 Inkubator Inteligentny — System Sterowania Inkubacją
Kompletny system zarządzania inkubacją z automatyczną termoregulacją, obrotem jaj, monitoringiem w czasie rzeczywistym i inteligentnymi alarmami opartymi na profilach gatunków.

📦 Spis treści
Opis systemu

Struktura projektu

Pliki źródłowe (ESP32)

Pliki interfejsu webowego

Pliki danych (JSON)

Instalacja i wdrożenie

Komunikacja — API i WebSocket

Konfiguracja gatunków

Zakładki interfejsu

Troubleshooting

🎯 Opis systemu
System przeznaczony jest do profesjonalnej inkubacji jaj w warunkach domowych i hodowlanych. Obsługuje dwie niezależne tace, każda z własnym gatunkiem i harmonogramem inkubacji.

Główne funkcje:
✅ Termoregulacja — regulator temperatury (On/Off lub PID) z histerezą i alarmami

✅ Kontrola wilgotności — setpoint, histereza, czas pracy pompy

✅ Obrót jaj — dwa niezależne kanały PWM z programowalnym interwałem i czasem

✅ Profile gatunków — Kura, Kaczka, Gęś, Przepiórka, Perliczka, Bażant

✅ Harmonogram inkubacji — dzień po dniu: temperatura powłoki, wilgotność, obrót, lockdown

✅ Fazy wrażliwości — CRITICAL / HIGH / MEDIUM z automatycznymi ostrzeżeniami

✅ Alarmy — temperatury, wilgotności, awaria grzałki/pompy, poziom wody, Wi-Fi, NTP

✅ Dziennik inkubacji — zapis lęgów, wpisy dzienne, ovoskop, podsumowania

✅ Kalibracja — ręczne sterowanie PWM, stoper, test sekwencji

✅ Diagnostyka — RAM, temperatura ESP, pinout, logi zdarzeń

✅ Aktualizacje OTA — przez przeglądarkę

✅ Wielojęzyczność — polski, angielski, portugalski, francuski (w przygotowaniu)

📁 Struktura projektu
text
inkubator/
├── firmware/                          # Kod źródłowy ESP32 (PlatformIO)
│   ├── src/
│   │   ├── main.cpp                   # Główny plik — setup(), loop(), serwery
│   │   ├── config_manager.cpp         # Konfiguracja (LittleFS + Preferences)
│   │   ├── config_manager.h
│   │   ├── controller.cpp             # Sterowanie: grzałki, dyfuzory, alarmy, PID
│   │   ├── controller.h
│   │   ├── tray_controller.cpp        # Sterowanie tacami: obrót, dzień, gatunek
│   │   ├── tray_controller.h
│   │   ├── species_profiles.cpp       # Profile gatunków (stara wersja)
│   │   ├── species_profiles.h
│   │   ├── incubation_profile.cpp     # Profile inkubacji v2 (shell_temp, sensitivity)
│   │   └── incubation_profile.h
│   ├── data/                          # Pliki wgrywane do LittleFS
│   │   ├── index.html                 # Główny dashboard
│   │   ├── temperature.html           # Ustawienia temperatury
│   │   ├── humidity.html              # Ustawienia wilgotności
│   │   ├── turner.html                # Ustawienia obrotu
│   │   ├── incubation.html            # Ustawienia inkubacji
│   │   ├── alarms.html                # Ustawienia alarmów
│   │   ├── wifi.html                  # Ustawienia Wi-Fi
│   │   ├── various.html               # Ustawienia różne (NTP, OTA, język)
│   │   ├── console.html               # Konsola terminala
│   │   ├── ajustes.html               # Spis ustawień (hub)
│   │   ├── incubation_profiles.json   # Profile gatunków (v2)
│   │   └── process_profiles.json      # Alternatywne profile (opcjonalnie)
│   ├── platformio.ini                 # Konfiguracja PlatformIO
│   └── ...
│
├── web/                               # Frontend — pliki do edycji
│   ├── index.html
│   ├── temperature.html
│   ├── humidity.html
│   ├── turner.html
│   ├── incubation.html
│   ├── alarms.html
│   ├── wifi.html
│   ├── various.html
│   ├── console.html
│   ├── ajustes.html
│   └── incubation_profiles.json
│
├── docs/                              # Dokumentacja
│   ├── README.md                      # Ten plik
│   ├── INSTRUKCJA_INTEGRACJI.md       # Instrukcja integracji
│   ├── QUICK_REFERENCE_TABLE.md       # Szybka tabela referencyjna
│   └── SCHEMA_SYGNALOW.md             # Schemat sygnałów i flow danych
│
└── outputs/                           # Pliki wygenerowane (do archiwum)
    ├── incubation_profiles.json
    ├── incubator_alarm_logic.cpp
    ├── IncubatorDashboard.jsx
    └── ...
🔧 Pliki źródłowe (ESP32)
Plik	Ścieżka	Opis
main.cpp	firmware/src/	Główna pętla, WebServer, WebSocket, odczyt sensorów, RTC, EEPROM
config_manager.cpp	firmware/src/	Zapis/odczyt konfiguracji w LittleFS + Preferences z CRC
controller.cpp	firmware/src/	Sterowanie grzałkami (PWM), dyfuzorami, alarmy, PID, learning
tray_controller.cpp	firmware/src/	Sterowanie tacami (obrót, dzień, gatunek, manual move)
species_profiles.cpp	firmware/src/	Wczytywanie profili gatunków z JSON (stara wersja)
incubation_profile.cpp	firmware/src/	Wczytywanie profili inkubacji v2 (shell_temp, sensitivity, notes)
🌐 Pliki interfejsu webowego
Plik	Zakładka / Funkcja	Opis
index.html	STATUS, LĘG, CHARTS, SCHEDULE, JOURNAL, ESP, LOGS, CALIBRATE	Główny dashboard z elipsą termoregulacji, trendem, liniałami dni, alarmami
temperature.html	Ustawienia temperatury	Setpoint, histereza, alarmy, kalibracja, tryb PID/OnOff, opóźnienie wentylatora
humidity.html	Ustawienia wilgotności	Setpoint, histereza, alarmy, kalibracja, czas pompy, interwał wody
turner.html	Ustawienia obrotu	Tryb (ciągły/cykl), czas pracy, czas przerwy, PWM, test manualny
incubation.html	Zarządzanie inkubacją	Gatunek, dzień, harmonogram, podgląd ustawień dla dwóch tac
alarms.html	Ustawienia alarmów	Włączanie/wyłączanie poszczególnych alarmów (checkboxy)
wifi.html	Konfiguracja Wi-Fi	SSID, hasło, status połączenia
various.html	Ustawienia różne	NTP, język, OTA, reset fabryczny, reboot
console.html	Konsola terminala	Wysyłanie komend tekstowych do ESP (debug)
ajustes.html	Spis ustawień (hub)	Linki do wszystkich podstron konfiguracyjnych
📊 Pliki danych (JSON)
Plik	Lokalizacja	Opis
incubation_profiles.json	data/	Profile gatunków z danymi dzień po dniu: shell_temp, air_temp_target, humidity_set, sensitivity, notes
process_profiles.json	data/	Alternatywne profile (opcjonalnie, do rozszerzenia)
Struktura incubation_profiles.json:

json
{
  "species": {
    "chicken": {
      "common_name": "Kura",
      "incubation_days": 21,
      "turning_frequency_per_day": 4,
      "critical_phases": { "early": [1,2,3], "late": [16,17,18], "lockdown": [19,20,21] },
      "day_by_day": [
        { "day": 1, "shell_temp": 37.8, "air_temp_target": 37.5, "humidity_set": 50, "humidity_range": [40,60], "turning": true, "sensitivity": "CRITICAL", "notes": "..." }
      ]
    }
  }
}
🚀 Instalacja i wdrożenie
Wymagania
ESP32-S3 (lub ESP32 z wystarczającą ilością RAM)

PlatformIO (zalecane) lub Arduino IDE

LittleFS (system plików dla danych)

Krok 1: Przygotowanie plików
bash
# Skopiuj pliki HTML do katalogu data/
cp web/*.html firmware/data/
cp web/*.json firmware/data/
Krok 2: Wgranie LittleFS
bash
cd firmware
pio run -t buildfs
pio run -t uploadfs
Krok 3: Kompilacja i wgranie firmware
bash
pio run -t upload
Krok 4: Połączenie i test
ESP32 uruchomi Access Point o SSID INKUBATOR (hasło: 5465INKUBATOR5465).

Połącz się z siecią Wi-Fi INKUBATOR.

Otwórz przeglądarkę i przejdź do: http://192.168.5.1/

Powinien pojawić się dashboard.

🔌 Komunikacja — API i WebSocket
REST API (port 80)
Endpoint	Metoda	Opis
/api/status	GET	Aktualne dane: temperatura, wilgotność, alarmy, PWM
/api/trays	GET	Dane tac: dzień, gatunek, obrót, lockdown
/api/config	GET	Pełna konfiguracja (JSON)
/api/config	POST	Zapis konfiguracji
/api/profile?species=CHICKEN	GET	Harmonogram dla gatunku
/api/history	GET	Historia temperatur/wilgotności
/api/reset	POST	Reset inkubacji
/api/reboot	POST	Restart ESP
WebSocket (port 81)
Używany do komunikacji w czasie rzeczywistym z podstronami ustawień.

Przykładowe komendy:

Komenda	Opis
setpoint:37.5	Ustawienie temperatury
histerese:0.5	Ustawienie histerezy
alarmMax:40.0	Ustawienie alarmu max
alarmMin:36.0	Ustawienie alarmu min
calibracao:0.0	Kalibracja czujnika
mode:0	Tryb sterowania (0=On/Off, 1=PID)
salvarVentilador:5	Opóźnienie wentylatora
limparMinMax	Reset ekstremów
carregarConfiguracao	Pobranie konfiguracji
🐣 Konfiguracja gatunków
System obsługuje następujące gatunki:

Gatunek	ID w JSON	Dni inkubacji	Lockdown	Uwagi
Kura	chicken	21	19	Standardowy profil
Kaczka Biegus	duck_indian_runner	28	25	Wyższa wilgotność
Gęś	goose	30	27	Dłuższy cykl
Przepiórka	quail	17	15	Krótki cykl
Perliczka	guinea_fowl	26	23	Średni cykl
Bażant	pheasant	24	21	Średni cykl
Każdy gatunek ma zdefiniowane:

shell_temp — temperatura powłoki jaja (docelowa)

air_temp_target — docelowa temperatura powietrza

humidity_set — docelowa wilgotność

humidity_range — zakres wilgotności

turning — czy obracać jaja

sensitivity — CRITICAL / HIGH / MEDIUM

notes — porady dla użytkownika

📱 Zakładki interfejsu
STATUS
Elipsa termoregulacji (punkt pracy)

Trend odchyleń ΔT i ΔRH

Wskaźniki mocy grzałek (G1, G2)

Dioda dyfuzora

Liniały dni inkubacji dla T1 i T2

Lista alarmów

LĘG
Wybór gatunku, dnia i tacy

Harmonogram dzień po dniu z kolorami faz wrażliwości

Podgląd ustawień dla wybranego dnia

CHARTS
Wykresy temperatury i wilgotności (zakresy: 1h, 6h, 24h, 7d)

Eksport CSV

SCHEDULE
Porównanie harmonogramów dla dwóch tac

JOURNAL
Dziennik inkubacji

Dodawanie wpisów startowych i dziennych

Ovoskop, wagi, notatki

Podsumowanie po wylęgu

ESP
Diagnostyka ESP (RAM, temperatura, CPU)

Pinout i okablowanie

LOGS
Logi zdarzeń (alarmy, komendy, błędy)

CALIBRATE
Kalibracja tacy: PWM, faza, stoper, sekwencje

Test manualny obrotu

🐛 Troubleshooting
Problem: "Brak połączenia z ESP"
Sprawdź:

Czy jesteś podłączony do sieci Wi-Fi INKUBATOR?

Czy ESP jest zasilany i działa (dioda LED)?

Czy w przeglądarce otwierasz http://192.168.5.1/?

Problem: "WebSocket nie łączy się (port 81)"
Sprawdź:

Czy w firmware WebSocket jest uruchomiony (webSocket.begin())?

Czy port 81 nie jest blokowany przez zaporę sieciową?

Spróbuj w konsoli przeglądarki: new WebSocket('ws://192.168.5.1:81')

Problem: "Brak danych w dashboardzie"
Sprawdź:

Czy LittleFS jest poprawnie wgrany (pio run -t uploadfs)?

Czy plik incubation_profiles.json istnieje w data/?

Czy API /api/status zwraca JSON? (otwórz w przeglądarce)

Problem: "Nie można zapisać ustawień"
Sprawdź:

Czy WebSocket jest połączony (zielona kropka w prawym górnym rogu)?

Czy wartości są w dozwolonym zakresie?

Czy ESP ma wystarczająco dużo wolnej pamięci?

📚 Dokumentacja dodatkowa
INSTRUKCJA_INTEGRACJI.md — szczegółowy przewodnik integracji

QUICK_REFERENCE_TABLE.md — tabela wartości dzień po dniu

SCHEMA_SYGNALOW.md — schemat sygnałów i flow danych

📝 Licencja
Projekt jest udostępniany na licencji MIT — możesz go dowolnie wykorzystywać, modyfikować i rozpowszechniać.

🙏 Podziękowania
Projekt portugalski (virador, incubacao, temperatura, umidade, wifi, diversos, alarmes, console, ajustes) — za inspirację i logikę WebSocket

ukkz/esp32-incubator — za logikę PID

molo-7/esp32-egg-incubator — za fail-safe i tryb offline

Wersja: 2.0
Data: 2025-07-06
Autor: Rafał's Homestead, Kraśnik, Lubelskie

Powodzenia z lęgami! 🥚🐤🦆