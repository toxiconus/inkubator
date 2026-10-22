# 🔄 TEST vs PRODUCTION Mode - Konfiguracja Interwałów

## 🎯 Dlaczego Dwa Tryby?

```
TESTOWANIE:
  - Chcesz widzieć co się dzieje (co 10 sek odczyt)
  - Szybka analiza problemu
  - Dużo danych = łatwiej debugować

PRODUKCJA (21 dni z jajkami):
  - Chcesz oszczędzić pamięć SPIFFS
  - Chcesz oszczędzić energię (mniej pracy CPU)
  - Co 30 sek odczyt wystarczy (jaja się wolno zmieniają!)
```

---

## 📊 Porównanie Trybów

```
PARAMETR                    TEST MODE    PRODUCTION MODE
────────────────────────────────────────────────────────
Odczyt czujników            10 sekund    30 sekund
Logging do CSV              60 sekund    600 sekund (10 min)
Serial Monitor output       60 sekund    300 sekund (5 min)
Liczba logów w 21 dni       30,240       2,520 (12x mniej!)
Użycie SPIFFS (KB/dzień)    ~20 KB       ~1.7 KB
CPU obciążenie              Wyższe       Niższe
Przydatność                 Debugowanie  Rzeczywista inkubacja
```

---

## 🔧 Jak Przełączać?

### Otwórz main.cpp i znajdź (linia ~12-13):

```cpp
// #define MODE_TEST           // Odkomentuj dla TESTÓW (częste odczyty, fast logging)
#define MODE_PRODUCTION      // Odkomentuj dla PRAWDZIWEJ INKUBACJI (rzadkie odczyty, mała pamięć)
```

### Dla TESTÓW:
```cpp
#define MODE_TEST           // ← Odkomentuj (usuń //)
// #define MODE_PRODUCTION   // ← Zakomentuj
```

### Dla PRODUKCJI (21 dni z jajkami):
```cpp
// #define MODE_TEST           // ← Zakomentuj
#define MODE_PRODUCTION      // ← Odkomentuj (usuń //)
```

Potem: **Ctrl+U** w PlatformIO (upload to board)

---

## 📈 Co Się Zmienia?

### MODE_TEST (Testowanie)
```
HEATER_UPDATE_INTERVAL: 10 sek
  └─ Czujniki odczytywane co 10 sekund
  └─ Szybka odpowiedź na zmiany (algorytm PWM odpowiada szybko)

LOG_INTERVAL: 60 sek
  └─ CSV zapisywany co 60 sekund
  └─ Dużo danych, ale wystarczy do analizy

PRINT_INTERVAL: 60 sek
  └─ Serial Monitor aktualizuje co 60 sekund
  └─ Widzisz na żywo co się dzieje

REZULTAT:
  ├─ ~60 logów na dzień (1 log co minuta)
  ├─ ~20 KB SPIFFS na dzień (mało)
  ├─ Dobrze do debugowania i optymalizacji
  └─ Idealnie dla TESTÓW (kilka godzin)
```

### MODE_PRODUCTION (Produkcja)
```
HEATER_UPDATE_INTERVAL: 30 sek
  └─ Czujniki odczytywane co 30 sekund
  └─ Wolniejsza odpowiedź (ale dla jaj OK - nie zmieniają się szybko)
  └─ Mniej pracy CPU (oszczędź energię)

LOG_INTERVAL: 600 sek (10 minut)
  └─ CSV zapisywany co 10 minut
  └─ Mało danych, ale wystarczy (embrion zmienia się stopniowo)

PRINT_INTERVAL: 300 sek (5 minut)
  └─ Serial Monitor aktualizuje co 5 minut
  └─ Możesz zostawić terminal aktywny cały czas

REZULTAT:
  ├─ ~144 logów na dzień (1 log co 10 minut)
  ├─ ~1.7 KB SPIFFS na dzień (prawie nic!)
  ├─ 21 dni = ~2500 logów = ~35 KB (mieści się!)
  └─ Idealnie dla PRODUKCJI (21 dni ciągłego monitorowania)
```

---

## 🧪 Scenariusze Użycia

### SCENARIUSZ 1: Testowanie Systemu (1-2 dni)
```
1. Wgraj kod z MODE_TEST
2. Obserwuj output (co minutę)
3. Zbierz CSV przez kilka godzin
4. Analizuj dane w Excelu
5. Zmień parametry jeśli trzeba
6. Powtórz do zadowolenia

CSV z MODE_TEST:
  - Pełna informacja (co minutę)
  - Łatwo widzieć anomalie
  - Idealny do debugowania
```

### SCENARIUSZ 2: Pierwsza Próba z Jajkami (21 dni)
```
1. Wgraj kod z MODE_PRODUCTION
2. Włóż jajka do inkubatora
3. Obserwuj output (co 5 minut)
4. Nie rób nic przez 18 dni
5. Po 18 dni: "lockdown" - ostatnie 3 dni
6. Dzień 21: Piskliaki!

CSV z MODE_PRODUCTION:
  - 2500 logów na całe 21 dni
  - Mało pamięci = duża niezawodność
  - Zawsze możesz sprawdzić co było wczoraj
```

### SCENARIUSZ 3: Szybka Kalibracja (1 godzina)
```
1. Wgraj kod z MODE_TEST (odczyty co 10 sek)
2. Włącz system
3. Obserwuj przez 1 godzinę
4. Sprawdź czy parametry OK
5. Gotowe do produkcji!
```

---

## ⚙️ Szczegóły Techniczne

### Jak Działa Logika Interwałów?

```cpp
// Przykład MODE_TEST:
if (now - last_sensor_read > HEATER_UPDATE_INTERVAL) {  // 10 sek
  // Odczyt czujników i sterowanie
  
  if (now - last_data_log > LOG_INTERVAL) {  // 60 sek
    // Zapis do CSV (ale co 60 sek, a nie co 10 sek)
  }
  
  if (now - last_print_status > PRINT_INTERVAL) {  // 60 sek
    // Print do Serial Monitor (ale co 60 sek)
  }
}

// Rezultat:
// - Sterowanie: co 10 sek (szybka odpowiedź PWM)
// - CSV: co 60 sek (6 odczytów = średnia)
// - Serial: co 60 sek (widzisz średnią)
```

### SPIFFS Memory Calculation

```
MODE_TEST (21 dni):
  ├─ 1 log = ~80 bajtów
  ├─ 1440 logów/dzień (co minuta)
  ├─ 21 dni = 30,240 logów
  ├─ = ~2.4 MB (ESP32 ma 8 MB, ALE...)
  ├─ Może się zapełnić!
  └─ DLATEGO: MODE_TEST tylko dla testów!

MODE_PRODUCTION (21 dni):
  ├─ 1 log = ~80 bajtów
  ├─ 144 logów/dzień (co 10 minut)
  ├─ 21 dni = 2,520 logów
  ├─ = ~200 KB (czysty!)
  └─ DLATEGO: MODE_PRODUCTION dla prawdziwej inkubacji!
```

---

## 🚀 Rekomendacje

### Dla TESTÓW (1-2 dni)
```
✅ MODE_TEST
   - Odczyty: 10 sekund
   - Logging: 60 sekund
   - CSV: Ogromny, ale detailowy
   - Energia: Wyższa (ale krótki czas)
   - Analiza: Łatwa (dużo danych)
```

### Dla PRODUKCJI (21 dni)
```
✅ MODE_PRODUCTION
   - Odczyty: 30 sekund
   - Logging: 600 sekund (10 minut)
   - CSV: Mały (zaoszczędzisz 95% pamięci!)
   - Energia: Niższa (oszczędzisz baterię jeśli na baterii)
   - Analiza: Wystarczająca (4x na godzinę = OK)
```

---

## 📝 Workflow

### DZIEŃ 1-2: TESTOWANIE
```
CODE:  #define MODE_TEST
TESTY: Kilka godzin (TEST 0-2)
CSV:   ~20 KB/dzień
OUTPUT: Co minutę
CEL:   Zrozumieć system, debugować
```

### DZIEŃ 3-23: PRODUKCJA (21 dni z jajkami!)
```
CODE:  #define MODE_PRODUCTION
TESTY: 21 dni rzeczywista inkubacja
CSV:   ~1.7 KB/dzień (razem ~35 KB)
OUTPUT: Co 5 minut
CEL:   Inkubacja piskląt
```

---

## ❓ FAQ

### Q: Czy mogę zmienić interwały na własne?
**A**: TAK! W main.cpp, zmień te linie:
```cpp
#ifdef MODE_TEST
  #define HEATER_UPDATE_INTERVAL 10000   // <- zmień
  #define LOG_INTERVAL 60000            // <- zmień
  #define PRINT_INTERVAL 60000          // <- zmień
#endif
```

### Q: Ile zajmie RAM?
**A**: ~500 bajtów (struct + zmienne). Brak problemu!

### Q: Co jeśli zapełni się SPIFFS?
**A**: 
- MODE_TEST: ~2.4 MB na 21 dni = zbyt dużo!
- MODE_PRODUCTION: ~200 KB na 21 dni = OK

### Q: Czy mogę zmienić tryb w trakcie?
**A**: TAK, ale musisz resetować ESP32 (przycisk RST)

### Q: Dla piskląt powinienem używać MODE_PRODUCTION?
**A**: TAK! 21 dni to za długo dla MODE_TEST

### Q: Czy mogę mieć 3 tryby?
**A**: TAK! Dodaj `#define MODE_CUSTOM` i ustaw własne interwały

---

## 🔍 Jak Interpretować Interwały?

```
HEATER_UPDATE_INTERVAL (co ile czujniki):
  - 10 sek:   Szybka kontrola (TEST)
  - 30 sek:   Normalna kontrola (PRODUCTION)
  - 60 sek:   Powolna kontrola (rzadko używane)
  - 1 min:    Zbyt powolnie! (embrion może zamarznąć)

LOG_INTERVAL (co ile CSV):
  - 60 sek:   Dużo danych (TEST, szybka analiza)
  - 600 sek:  Mało danych (PRODUCTION, zaoszczędzisz RAM)
  - 3600 sek: Bardzo mało danych (co godzinę - extreme production)

PRINT_INTERVAL (co ile Serial Monitor):
  - 60 sek:   Widzisz co się dzieje (TEST)
  - 300 sek:  Mniej printów (PRODUCTION, mniej scroll)
  - 3600 sek: Co godzinę (extreme production, prawie nic nie widać)
```

---

## ✅ Checklist

- [ ] Wiesz jaki tryb użyć (TEST vs PRODUCTION)
- [ ] Umiesz przełączać między trybami (#define)
- [ ] Rozumiesz co się zmienia (interwały)
- [ ] Widzisz korzyści (oszczędzisz RAM!)
- [ ] Gotów do wgrania kodu

---

**Wersja**: 1.0  
**Data**: 2026-05-16  
**Status**: Gotowy do użycia
