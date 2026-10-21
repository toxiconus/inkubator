# Bill of Materials (BOM) - Inkubator ESP32-S3

## Wyjaśnienie: Dlaczego każdy komponent?

### 🔋 ZASILANIE

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 1 | Zasilacz Mean Well | LRS-150-12 (12V/12.5A, 150W) | 1 | ~200-250 PLN | **Stabilne zasilanie 12V na 96W grzałek** - Must-have. Zasilacze tańsze mogą się przegrzewać lub mieć szumy |
| 2 | Przetwornica DC/DC | LM2596-V3 (12V→5V, adjustable) | 1 | ~20-30 PLN | **Obniża 12V do 5V dla ESP32 i logiki** - Alternatywa: AMS1117 (ale robi więcej ciepła na tak dużym obciążeniu) |
| 3 | Bezpiecznik główny | 15A (oprawiacz) | 1 | ~3 PLN | **Chroni cały system przed zwarciem** - Jeśli 12V zwrze, bez tego spali się całą elektronikę |

---

### 🔲 MIKROKONTROLER

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 4 | Mikrokontroler | ESP32-S3 N8R2 (44-pin Type-C) | 1 | ~80-120 PLN | **Mózg systemu** - WiFi + Bluetooth, dużo GPIO, ADC, PWM, OneWire support |
| 5 | Kabel USB-C | USB 2.0 (do programowania) | 1 | ~10-20 PLN | **Programowanie i debugging** - Alternatywa: UART adapter, ale USB-C wygodniej |

---

### 🔥 STEROWANIE GRZAŁKAMI

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 6 | Moduł MOSFET | 2-kanał PWM 15A (IRF540N lub podobny) | 1-2 | ~40-80 PLN | **Przełączanie 12V na grzałkach** - MOSFET lepszy niż relay (brak trzasku, PWM działa). 2 kanały = 4 wyjścia (szeregowo) |
| 7 | Rezystor Pull-down | 10kΩ, 1/4W | 4 | ~0.5 PLN | **Bezpieczeństwo MOSFET** - Jeśli ESP32 utraci zasilanie, MOSFET domyślnie wyłączony (nie pali grzałkami) |
| 8 | Kondensator | 10µF film 50V | 4 | ~1 PLN | **Filtracja szumów PWM** - Zmniejsza piki napięcia przy szybkich zmianach (mniej zaburzeń analogowych) |

---

### 💨 STEROWANIE WENTYLATORAMI

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 9 | Moduł Relay | 1-kanałowy 5V (SRD-05VDC-SL-C) | 1 | ~15-25 PLN | **Przełącznik do wentylator 12V** - Relay wymagany bo wentylator indukcyjny (mogą przechodzić wolne napięcia). Lepsze niż tranzystor |
| 10 | Wiatrak 12V | 12V DC fan, ~0.5A | 1 | ~30-50 PLN | **Wentylacja i chłodzenie** - Potrzebny czy system pracuje zawsze (tlenowanie dla embrionów) |
| 11 | Rezystor | 10kΩ, 1/4W | 1 | ~0.1 PLN | **Pull-down do Relay** - Opcjonalnie, dla pewności że relay wyłączy się przy starcie |

---

### 💧 CZUJNIKI WILGOCI

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 12 | Czujnik DHT22 | Temperatura + Wilgość (-40 do +80°C) | 1 | ~25-40 PLN | **Pomiar temperatury i wilgotności** - DHT22 dokładniejszy niż DHT11. OneWire mieć oddzielnie |
| 13 | Rezystor Pull-up | 10kΩ, 1/4W | 1 | ~0.1 PLN | **Protokół DHT wymaga** - Dane przez jeden pin, pull-up musi być |
| 14 | Kondensator | 100nF (0.1µF) film 50V | 1 | ~0.3 PLN | **Filtracja szumów DHT22** - Między VCC i GND DHT, zmniejsza anomalie odczytu |

---

### 🌡️ CZUJNIKI TEMPERATURY

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 15 | Czujnik DS18B20 | OneWire temperatura (-55 do +125°C) | 2 | ~15-25 PLN (2 szt) | **Pomiar w różnych punktach komory** - OneWire lepszy niż DHT (dokładniejszy, bardziej niezawodny). Montaż: górny i dolny punkt |
| 16 | Rezystor Pull-up | 4.7kΩ, 1/4W | 1 | ~0.1 PLN | **OneWire wymaga pull-up** - Standard protokołu. Każdy DS podłączony na tym samym pinie |
| 17 | Kondensator | 100nF (0.1µF) film 50V | 2 | ~0.6 PLN | **Filtracja szumów linii OneWire** - Zmniejsza błędy odczytu |

---

### 💦 CZUJNIK POZIOMU WODY

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 18 | Czujnik poziomu | Capacitive (3-wire 5V) | 1 | ~20-30 PLN | **Pomiar poziomu płynu bez kontaktu** - Capacitive nie żrdłoży się od wody (kontak jest w komorze) |
| 19 | Rezystor Pull-down | 10kΩ, 1/4W | 1 | ~0.1 PLN | **Wstabilizowanie ADC** - Jeśli czujnik zwisa bez obciążenia |
| 20 | Kondensator | 100nF (0.1µF) film 50V | 1 | ~0.3 PLN | **Filtracja analogowa** - ADC odczytuje bardziej stabilnie |

---

### 🚪 CZUJNIK DRZWI

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 21 | Przełącznik | Reed switch + magnes | 1 | ~5-10 PLN | **Detekacja otwarcia drzwi** - Bezprzewodowy (magnes) - nie da się przypadkowo zepsuć |
| 22 | Rezystor Pull-up | 10kΩ, 1/4W | 1 | ~0.1 PLN | **Pull-up wewnętrzny ESP32, ale zewnętrzny lepszy** - ESP32 może mieć niska impedancja |
| 23 | Kondensator | 100nF (0.1µF) film 50V | 1 | ~0.3 PLN | **Debouncing** - Przełącznik może się trzaść, kondensator filtruje szum |

---

### 💦 NAWILRZACZE ULTRASONICZNE

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 24 | Nawilrzacz ultradźwiękowy | 5V, 2W | 1-2 | ~30-50 PLN/szt | **Парообразование bez grzania** - Ultrasoniczne lepsze niż parowe (nie zmiękczają embrion) |
| 25 | MOSFET/Tranzystor | N-channel (2N7000 lub IRF520N) | 2 | ~3-5 PLN | **Przełączanie 5V na nawilrzaczach** - PWM sterowanie - mogą pracować na niżej niż 100% |
| 26 | Rezystor Pull-down | 10kΩ, 1/4W | 2 | ~0.2 PLN | **Bezpieczeństwo MOSFET** - Gdy ESP32 wyłączony = nawilrzacz wyłączony |

---

### 🔌 ZASILANIE I POŁĄCZENIA

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 27 | Przewód zasilający | 12V: min 1.5mm² (mogą być grubiutkie jak grzałki biora ~8A) | ~5m | ~15-20 PLN | **Transport prądu bez strat** - Cienkie przewody = spadki napięcia i ciepło |
| 28 | Przewód sygnałowy | 0.5-0.75mm² (GPIO, czujniki) | ~10m | ~10-15 PLN | **Sygnały logiczne** - Tańsze niż grube, wystarczające dla milliampów |
| 29 | GND szynka | Aluminiowa lub miedź | 1 | ~10-15 PLN | **Wspólna masa dla wszystkiego** - Ogranicza szumy, pętle gruntu |
| 30 | Złączki | Zaciski śrubowe (KF301, KF302) | 5-10 | ~5-10 PLN | **Łatwe podłączenie/rozpinanie** - Alternatywa: drut wsklepiony (mniej wygodny) |

---

### 📊 ZASILANIE LOGIKI (5V, LM2596)

| Lp. | Komponent | Model/Parametry | Ilość | Cena estymowana | Dlaczego? |
|-----|-----------|-----------------|-------|-----------------|-----------|
| 31 | Kondensator wejściowy | 100µF elektrolit, 25V | 1 | ~1 PLN | **Wygładzenie napięcia 12V przed LM2596** - Chroni przed spadkami napięcia |
| 32 | Kondensator wyjściowy | 100µF tantał, 10V | 1 | ~2-3 PLN | **Wygładzenie napięcia 5V** - Tantał szybciej reaguje niż elektrolit |
| 33 | Kondensator | 10µF film, 50V | 1 | ~0.5 PLN | **Filtracja szumów RMS** - Dodatkowo, zaraz za LM2596 |
| 34 | Dioda Schottky'ego | 1N5817 (20V, 1A) | 1 | ~0.5 PLN | **Ochrona przed zwrotem napięcia** - Jeśli battery się rozładuje przez LM2596 |

---

## 📋 LISTA REZYSTORÓW I KONDENSATORÓW - KTÓRE DO CZEGO

### ✅ REZYSTORY (możesz użyć różne wartości, jeśli masz)

```
10kΩ:  Pull-up/down dla GPIO, Relay, MOSFET (NAJWAŻNIEJSZE)
  └─ Ilość: 10-15 szt (Pull-down do każdego MOSFET, Pull-up do czujników)
  └─ Dlaczego: Standard dla logiki 5V, wiele aplikacji

4.7kΩ: Pull-up dla OneWire (DS18B20)
  └─ Ilość: 1-2 szt
  └─ Dlaczego: OneWire wymaga 4.7kΩ, 10kΩ mogą być też ok ale 4.7kΩ zalecanego

1kΩ:  Rezystor seriowy do LED-ów (do debugowania)
  └─ Ilość: 2-3 szt
  └─ Dlaczego: Ogranicza prąd LED, zapobiega spaleniu się diody

```

**TL;DR Rezystory**: Jeśli masz 10kΩ - wystarczą! Mogę obejść się bez 4.7kΩ.

---

### ⚠️ KONDENSATORY (TU JEST PROBLEM - POTRZEBUJESZ!)

```
100µF elektrolit, 25V (x1)
  └─ Gdzie: Wejście LM2596 (12V strona)
  └─ Dlaczego: Wygładzenie napięcia z zasilacza, chroni przed SpikAMI
  └─ Zamiennik: 47µF, 10µF elektrolit (mniej efektywny, ale zadziała)

100µF tantał, 10V (x1)
  └─ Gdzie: Wyjście LM2596 (5V strona)
  └─ Dlaczego: SZYBKA filtracja dla ESP32 (tangalumowe mają BARDZO niski ESR)
  └─ Zamiennik: 100µF elektrolit (wolniejszy, ale zadziała)

10µF film, 50V (x4-5)
  └─ Gdzie: Przy sygnałach PWM (grzałki, nawilrzacze, wentylator)
  └─ Dlaczego: Filtracja szumów PWM, zmniejsza EMI
  └─ Zamiennik: 22µF, 4.7µF film (trochę inne charakterystyki, ale ok)

100nF (0.1µF) film, 50V (x6-8)
  └─ Gdzie: Zaraz obok każdego IC (DHT22, DS18B20, czujnik poziomu)
  └─ Dlaczego: Standard "bypass" - chroni przed szumami, BARDZO ważne dla czujników!
  └─ Zamiennik: 10nF, 1µF film (działają, ale 100nF jest "złotym standardem")
```

**TL;DR Kondensatory**: 
- **KRYTYCZNE**: 100µF elektrolit (wejście LM2596) - bez tego będą problemy z stabilnością
- **WAŻNE**: 100nF film x6 (bypass - każdy czujnik!) - bez tego czujniki mogą "szaleć"
- **PRZYDATNE**: 100µF tantał (wyjście LM2596), 10µF film (PWM)

---

## 🛒 PODSUMOWANIE ZAKUPÓW

### Bez kondensatorów (jeśli masz rezystory):
```
[ ] ESP32-S3 N8R2 + USB-C              ~100 PLN
[ ] Zasilacz Mean Well LRS-150-12      ~250 PLN
[ ] Moduł MOSFET 2-kanałowy            ~60 PLN
[ ] Moduł Relay 5V                     ~20 PLN
[ ] Wiatrak 12V 0.5A                   ~40 PLN
[ ] DHT22                              ~30 PLN
[ ] 2x DS18B20 (OneWire)               ~20 PLN
[ ] Czujnik poziomu wody               ~25 PLN
[ ] Reed switch + magnes               ~8 PLN
[ ] 1-2x Nawilrzacz 5V                 ~50 PLN
[ ] Bezpiecznik 15A                    ~3 PLN
[ ] Przewody (12V grube, sygnałowe)    ~20 PLN
[ ] Zaciski/złączki                    ~10 PLN
─────────────────────────────────────────────
SUMA BEZ KONDENSATORÓW                 ~636 PLN
```

### KONDENSATORY (MUSZĄ BYĆ!):
```
[ ] 100µF elektrolit 25V (x1)          ~2 PLN
[ ] 100µF tantał 10V (x1)              ~3 PLN
[ ] 10µF film 50V (x5)                 ~2 PLN
[ ] 100nF film 50V (x8)                ~3 PLN
─────────────────────────────────────────────
SUMA KONDENSATORY                      ~10 PLN
```

### REZYSTORY (masz już):
```
[ ] 10kΩ 1/4W (x15)                    ~1 PLN
[ ] 4.7kΩ 1/4W (x2) - opcjonalnie     ~0.5 PLN
─────────────────────────────────────────────
SUMA REZYSTORY                         ~1.5 PLN
```

---

## 🎯 PLAN DZIAŁANIA

1. **Gdzie szukać kondensatorów?**
   - Allegro, eBay (tanie chińskie, dobre)
   - Lokalny sklep elektroniki (Botland, Electro24, Conrad)
   - AliExpress (najtatańsze, 2-4 tygodnie dostawy)

2. **Kupić priorytetowo:**
   - `100µF elektrolit 25V` ← **TO JEST NAJWAŻNIEJSZE**
   - `100nF film 50V` (x8) ← **Czujniki nie będą pracować bez tego!**

3. **Rezystory:**
   - Jeśli masz 10kΩ - świetnie! Użyj do wszystkiego
   - Jeśli masz inne wartości - spróbujemy zaadaptować

---

**Data**: 2026-05-16  
**Wersja**: 1.0 (z objaśnieniami dlaczego każdy komponent)
