# Inkubator Jaj - ESP32-S3

Automatyczny inkubator jaj oparty na ESP32-S3 N8R2 (docelowo N16R8).
Zasilanie 12V DC, sterowanie grzałkami przez PWM, WiFi AP + WebUI.

Firmware wspiera do 4 kanałów grzałek (CH0-CH3). Aktualny WebUI udostępnia sterowanie ręczne dla 2 kanałów (CH0-CH1), a kanały CH2-CH3 działają w automacie po przypisaniu pinów.

---

## 📋 Wersje WebUI

| Wersja | Status | Opis | Testy |
|--------|--------|------|-------|
| **indexv0.html** | 🟢 Produkcja | Rzeczywisty plik z ESP32-S3 (pobrany fizycznie) | Testowanie rzeczywistego sterowania: 2 grzałki 48W + 1 taca |
| **indexv1.html** | 🟡 Prototyp | Przeprojektowana tarcza termometru (semi-circle gauge, delty na półukach, znacznik ideału) | Dane testowe, brak rzeczywistego sterowania |
| **indexv2.html** | 🟡 Backup v1 | Kopia indexv1.html z nowym designem tarczy | Backup dla ewentualnych przywrócenia |

**Aktualnie testujemy:** indexv1 (nowy design) vs indexv0 (rzeczywista funkcjonalność)

---

## Struktura projektu

```
inkubator/
├── firmware/              # Kod ESP32-S3 (PlatformIO / Arduino)
│   ├── src/               # Pliki źródłowe .cpp
│   ├── include/           # Nagłówki .h
│   ├── data/              # Pliki WebUI i domyślna konfiguracja do LittleFS
│   │   ├── config.json    # Domyślna konfiguracja urządzenia
│   │   ├── index.html     # WebUI serwowany z LittleFS
│   │   ├── kalibracja.html # Wydzielona sekcja kalibracji ładowana dynamicznie przez index.html
│   │   └── documentation_v1_1.html # Lokalna dokumentacja w urządzeniu
│   ├── lib/               # Biblioteki zewnętrzne (lokalne)
│   └── test/              # Testy jednostkowe
├── hardware/
│   ├── schematics/        # Schematy elektryczne (KiCad / PDF)
│   ├── pcb/               # Projekt PCB (opcjonalnie)
│   └── bom/               # Bill of Materials
├── docs/
│   ├── datasheets/        # Karty katalogowe komponentów
│   ├── wiring/            # Diagramy okablowania
│   └── photos/            # Zdjęcia budowy
├── config/                # Pliki konfiguracyjne (np. WiFi credentials template)
├── tools/                 # Skrypty pomocnicze (flash, monitor, itp.)
└── README.md
```

## Firmware / WebUI

- `platformio.ini` ustawia `data_dir = firmware/data` i `board_build.filesystem = littlefs`.
- Aktualny WebUI serwowany z modułu: `firmware/data/index.html`.
- Sekcja kalibracji jest wydzielona do `firmware/data/kalibracja.html` i ładowana dynamicznie przez `index.html`.
- Pliki developerskie WebUI znajdują się w `firmware/data/dev/`:
  - `index.template.html`
  - `styles.css`
  - `app.js`
  - `kalibracja.html`
- Narzędzie do składania i ekstrakcji WebUI: `tools/build_webui.py`.
- Aby wyodrębnić dev assets z aktualnego `index.html`, uruchom:
  - `python tools/build_webui.py --extract`
- Aby złożyć `firmware/data/index.html` z dev sources, uruchom:
  - `python tools/build_webui.py --build`
- Aby złożyć i zminimalizować CSS/JS, dodaj flagę `--minify`.
- Aby wgrać pliki WebUI do LittleFS użyj `platformio run --target uploadfs --environment esp32s3` (lub odpowiednio innego środowiska).

## WebUI / podział plików

### Opcje podziału od najbezpieczniejszej do najbardziej ryzykownej

#### Opcja 1 — Podział po stronie kompilacji, nie serwowania (NAJBEZPIECZNIEJSZA)

Pliki HTML, CSS i JS trzymasz osobno na swoim komputerze jako pliki deweloperskie. Przed wgraniem na ESP32 skrypt (Python lub Node) skleja je automatycznie w jeden plik i minifikuje. ESP32 nadal serwuje jeden plik.

Zysk: czytelność kodu podczas edycji, zero ryzyka dla działania urządzenia.

Narzędzie do tego to np. prosty skrypt Python, który robi `string concatenation`, albo bardziej zaawansowane jak Vite lub esbuild — ale bez frameworka, tylko jako bundler.

To jest podejście, które polecam najbardziej dla Twojego przypadku. Podejście hybrydowe: **podział deweloperski + jednoplikowy deployment**.

W praktyce wygląda to tak: na komputerze masz strukturę katalogów, gdzie CSS jest w osobnym pliku, JavaScript podzielony na logiczne moduły (`core.js`, `tray.js`, `heater.js`, `journal.js`, `kalibracja.js`), a HTML ma miejsca oznaczone komentarzami gdzie wchodzą poszczególne sekcje.

Prosty skrypt `build.py` skleja wszystko w jeden `panel.html`, minifikuje (opcjonalnie) i to jest plik który wgrywasz na ESP32.

Dodatkowa zaleta tego podejścia: możesz otworzyć panel w przeglądarce bezpośrednio z dysku podczas developmentu, bez ESP32, i testować UI. Dopiero gdy chcesz przetestować komunikację z API, serwujesz z ESP32.

---

## Komponenty sprzętowe

| # | Komponent | Model | Parametry | Rola |
|---|-----------|-------|-----------|------|
| 1 | Mikrokontroler | ESP32-S3 N8R2 | 44-pin, Type-C, WiFi 2.4G + BT | Główny MCU |
| 2 | Zasilacz główny | Mean Well LRS-150-12 | 12V / 12.5A (150W) | Zasilanie całego systemu |
| 3 | Przetwornica DC/DC | LM2596-V3 | 12V → 5V (adj.), max ~3A | Zasilanie ESP32 i logiki |
| 4 | Przewód grzewczy | 12V / 24W / 3m | 12V DC | Ogrzewanie komory (x4) |
| 5 | Moduł MOSFET | 2x MOSFET PWM 15A | max 15A, sterowanie PWM | Sterowanie grzałkami |

**Łączna moc grzałek:** 4 × 24W = **96W** @ 12V (prąd ~8A)
**Rezerwa zasilacza:** 150W - 96W = 54W (dla logiki, serw, wentylatorów)

---

## Plan funkcjonalny (scope)

### Faza 1 - MVP (Grzanie)
- [ ] Pomiar temperatury (DS18B20 lub DHT11 / SHT31)
- [ ] Sterowanie PWM grzałkami przez MOSFET (PID lub on/off z histerezą)
- [ ] Wyświetlanie temperatury (OLED I2C lub Serial monitor)
- [ ] Nastawy przez Serial/WiFi

### Faza 2 - Wilgotność
- [ ] Pomiar wilgotności (SHT31 / DHT11)
- [ ] Sterowanie nawilżaczem (przekaźnik lub MOSFET)

### Faza 3 - Obracanie jaj
- [ ] Serwo lub silnik krokowy do obracania tac
- [ ] Harmonogram obracania (co 1-2h)

### Faza 4 - Interfejs i monitoring
- [ ] Web interface (ESP32 jako AP lub STA)
- [ ] Wykresy historii temperatury/wilgotności
- [ ] Alerty (Telegram / MQTT / email)
- [ ] Zegar RTC (DS3231) dla harmonogramu

---

## Pinout ESP32-S3 N8R2 (planowany)

| Pin ESP32-S3 | Funkcja | Komponent |
|--------------|---------|-----------|
| GPIO4 | OneWire / I2C SDA | Czujnik temperatury |
| GPIO5 | I2C SCL | Czujnik temp/wilg |
| GPIO1 | PWM CH0 | Grzałka 1 |
| GPIO2 | PWM CH1 | Grzałka 2 |
| GPIO (dowolny wolny) | PWM CH2 | Grzałka 3 |
| GPIO (dowolny wolny) | PWM CH3 | Grzałka 4 |
| GPIO18 | Relay/MOSFET | Nawilżacz (faza 2) |
| GPIO7 | Relay/PWM | Wentylator |
| GPIO21 | UART / Status LED | Debug |

> Pinout do weryfikacji po wyborze sensorów i wyświetlacza.

---

## Zasilanie - schemat blokowy

```
230V AC
   |
[LRS-150-12]
   |
  12V DC ─────────────────────────────┐
   |                                  |
[LM2596-V3]                    [MOSFET x2 (PWM)]
   |                                  |
  5V DC                        [Grzałka x4 (12V/24W)]
   |
[ESP32-S3]
   |
[Sensory / OLED / RTC]
```

---

## Wymagania środowiskowe inkubatora

| Parametr | Wartość |
|----------|---------|
| Temperatura inkubacji (kurczęta) | 37.5°C ± 0.3°C |
| Wilgotność (dni 1-18) | 45-55% RH |
| Wilgotność (dni 19-21) | 65-75% RH |
| Obroty jaj | co 1-2h, zatrzymane ostatnie 3 dni |

---

## TODO - hardware

- [ ] Wybrać czujnik temperatury/wilgotności (DS18B20 + DHT11 lub SHT31)
- [ ] Wybrać wyświetlacz (OLED 0.96" I2C)
- [ ] Zdecydować o RTC (DS3231)
- [ ] Zaprojektować schemat elektryczny (KiCad)
- [ ] Zmierzyć napięcie wyjściowe LM2596 przed podłączeniem ESP32
- [ ] Sprawdzić czy N8R2 → N16R8 wymagany przed startem

---

## Uwagi

- LM2596 **nastawić na 5V** przed podłączeniem ESP32 (potencjometr na module).
- ESP32-S3 zasilany z 5V (pin 5V/VBUS) lub 3.3V z przetwornicy.
- MOSFET-y sterowane sygnałem PWM z GPIO ESP32 (3.3V logika - sprawdzić próg bramki gate driver).
- Grzałki 12V podłączone przez drain/source MOSFET, masa wspólna z zasilaczem.

---

## Aktualizacja 2026-05-17

### WebUI
- Główny panel działa jako jeden spójny widok z osobnymi zakładkami:
   - Panel
   - Harmonogram
   - Połączenia i ESP
   - Okablowanie i zabezpieczenia
- W zakładce okablowania dodano tabelę elementów ochronnych (rezystory, bezpieczniki, diody) i checklistę serwisową.

### Kondensatory (czujniki i linie sygnałowe)
- DS18B20: 100nF między VDD i GND przy każdym czujniku.
- DHT11: 100nF + 1uF przy zasilaniu czujnika.
- BMP180 (I2C): 100nF między 3V3 i GND przy module.
- Linie sygnałowe dłuższe niż ~30 cm: opcjonalnie 1nF-4.7nF do GND przy wejściu MCU (tylko gdy występują zakłócenia).
- Pull-upy sygnałów czujników prowadzić do 3V3 (nie do 5V), żeby chronić GPIO ESP32-S3.

### Konfiguracja runtime
- Aktualny plik konfiguracyjny urządzenia: `firmware/data/config.json`
- Aktywny WebUI na urządzeniu: `firmware/data/index.html`
- Sekcja kalibracji ładowana dynamicznie: `firmware/data/kalibracja.html`
- Lokalna dokumentacja w urządzeniu: `firmware/data/documentation_v1_1.html`
- `platformio.ini` ustawia `data_dir = firmware/data` oraz `board_build.filesystem = littlefs`.
- `uploadfs` ładuje `firmware/data/` do LittleFS na ESP32-S3.
- Uwaga: podczas startu firmware najpierw ładuje konfigurację z Preferences (NVS), a dopiero potem z LittleFS. Po dużych zmianach struktury konfiguracji wykonaj factory reset z UI/API.
- Obecnie używane kluczowe parametry:
   - config_version: 106
   - tray_count: 2
   - ds18b20_count: 2 (wspólna linia GPIO6)
   - dht11_enabled: true
   - dht11_pin: 8 (w firmware klucz konfiguracyjny pozostaje kompatybilny z legacy dht22_pin)
   - bmp180_enabled: true (SDA GPIO4, SCL GPIO5)
   - heater_pin_1: 38, heater_pin_2: 39
   - heater_pin_3: -1, heater_pin_4: -1 (rezerwa, aktywuj gdy podłączysz kolejne moduły MOSFET)
   - diffuser pin: 21
   - rotation_pwm_speed: surowy zakres 0-255 (np. 102 ≈ 40%), nie 0-100%
