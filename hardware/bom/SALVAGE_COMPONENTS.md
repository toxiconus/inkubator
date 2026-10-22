# Odzysk komponentów - Kondensatory ze starych płytek (TV, dekodery, itp.)

## 🔧 Gdzie szukać kondensatorów z drugiej ręki?

### Źródła płytek elektronicznych

```
✅ NAJLEPSZE ŹRÓDŁA:
┌─────────────────────────────────────────────────────────┐
│ 1. Stare TV, dekodery, tuner satelitarny (u Ciebie!)  │
│    └─ Zwłaszcza zasilacze (switch mode) - pełno C!     │
│                                                          │
│ 2. Stare monitory LCD/LED                              │
│    └─ Power supply = skarb kondensatorów!              │
│                                                          │
│ 3. Urządzenia audio (wzmacniacze, procesory)           │
│    └─ Dobrze ochraniały kondensatory                   │
│                                                          │
│ 4. Zasilacze komputerowe (stare PSU)                   │
│    └─ Tonowy kondensatorów 10-100µF, różne wartości    │
│                                                          │
│ 5. Stare lodówki, pralki (elektronika)                 │
│    └─ Mniej przydatnych, ale czasem coś się uda       │
│                                                          │
│ 6. Święcimy/Serwisy elektroniki (mogą dać resztki)    │
│    └─ Czasem za darmo lub za kilka zł                 │
└─────────────────────────────────────────────────────────┘
```

---

## 📖 Jak zidentyfikować kondensatory?

### Oznaczenia na obudowie kondensatora

```
┌──────────────────────────────────────────────────────┐
│  KONDENSATOR ELEKTROLIT (cylindryczny)               │
├──────────────────────────────────────────────────────┤
│                                                      │
│     ┌──────────────────────┐                        │
│     │ 100µ                 │  ← Pojemność           │
│     │ 25V                  │  ← Napięcie nominalne  │
│     │ +85°C                │  ← Temp. max           │
│     │ 105h                 │  ← Żywotność (105°C)   │
│     └──────────────────────┘                        │
│      + (dłuższy przewód)                            │
│      - (krótszy przewód)                            │
│                                                      │
│  KODOWANIE POJEMNOŚCI:                              │
│  ┌─────────────────────────────────────┐            │
│  │ 100µ   = 100 mikrofarad (0.0001F)   │            │
│  │ 100n   = 100 nanofarad  (0.0000001F)│            │
│  │ 100p   = 100 pikofarad              │            │
│  │ 1µ     = 1 mikrofarad               │            │
│  │ 10u    = 10 mikrofarad              │            │
│  │ 47u    = 47 mikrofarad              │            │
│  │ 100u   = 100 mikrofarad             │            │
│  │ 220u   = 220 mikrofarad             │            │
│  │ 470u   = 470 mikrofarad             │            │
│  │ 1000u  = 1000 mikrofarad (1 mF)     │            │
│  └─────────────────────────────────────┘            │
└──────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────┐
│  KONDENSATOR FILM (płaski, prostokątny)              │
├──────────────────────────────────────────────────────┤
│                                                      │
│     ┌──────────────────────┐                        │
│     │ 100n 50V             │  ← Pojemność i napięcie│
│     │ MKT                  │  ← Typ folii           │
│     └──────────────────────┘                        │
│                                                      │
│  KODOWANIE (3-cyfrowy kod):                          │
│  ┌─────────────────────────────────────┐            │
│  │ 104 = 10 · 10^4 pF = 100 nF (0.1µ)  │            │
│  │ 105 = 10 · 10^5 pF = 1 µF           │            │
│  │ 224 = 22 · 10^4 pF = 220 nF         │            │
│  │ 225 = 22 · 10^5 pF = 2.2 µF         │            │
│  │ 474 = 47 · 10^4 pF = 470 nF         │            │
│  │ 475 = 47 · 10^5 pF = 4.7 µF         │            │
│  │ 684 = 68 · 10^4 pF = 680 nF         │            │
│  │ 105 = 10 · 10^5 pF = 1 µF           │            │
│  └─────────────────────────────────────┘            │
└──────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────┐
│  KONDENSATOR TANTALOWY (czarny, małe)                │
├──────────────────────────────────────────────────────┤
│                                                      │
│     ┌──────────────────────┐                        │
│     │ 100µ 10V             │  ← Pojemność i napięcie│
│     │ T or K               │  ← Typ polarności      │
│     └──────────────────────┘                        │
│      + (przewód dodatni)                            │
│                                                      │
│  KODOWANIE (na boku):                               │
│  ┌─────────────────────────────────────┐            │
│  │ 106   = 10 µF (napięcie na górze)    │            │
│  │ 476   = 47 µF                       │            │
│  │ 107   = 100 µF                      │            │
│  └─────────────────────────────────────┘            │
└──────────────────────────────────────────────────────┘
```

---

## 📊 KTÓRE KONDENSATORY z każdego urządzenia?

### TV / Monitor LCD
```
┌─────────────────────────────────────────────────────┐
│  ZASILACZ SWITCH-MODE (najważniejszy!)              │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌────────────────────────────────────┐            │
│  │ INPUT (110-240V AC):               │            │
│  │  ├─ C1: 100nF 250V AC (filtr)      │ ✅        │
│  │  ├─ C2: 47µ 450V (główny wejście)  │ ✅        │
│  │  └─ C3: 100µ 50V (dodatkowy)       │ ✅        │
│  │                                    │            │
│  │ SECONDARY (wyjście 5V, 12V):       │            │
│  │  ├─ C10: 100µ 25V (5V output)      │ ✅✅✅   │
│  │  ├─ C11: 47µ 25V (12V output)      │ ✅✅     │
│  │  ├─ C12: 1000µ 16V (duży)          │ ✅       │
│  │  ├─ C20: 10µ film 50V (filtr)      │ ✅       │
│  │  └─ Razem: 6-12 kondensatorów!     │           │
│  │                                    │            │
│  └────────────────────────────────────┘            │
│                                                     │
│  ⚠️ UWAŻAJ:                                        │
│  - Kondensatory na zasilaczu mogą być naładowane!  │
│  - PRZED DEMONTAŻEM sprawdzić multimetrem (V DC)   │
│  - Jeśli > 50V → ROZŁADUJ przez rezystor!          │
│                                                     │
└─────────────────────────────────────────────────────┘

Kondensatory przydatne do Twojego projektu:
  ✅✅✅ 100µ 25V   - NAJLEPSZY (LM2596 wyjście)
  ✅✅   47µ 25V    - OK (można zamiast 100µ)
  ✅    10µ 50V    - Film, do PWM
  ✅    100nF 50V  - Bypass (do czujników)
```

### Dekoder / Tuner satelitarny
```
┌─────────────────────────────────────────────────────┐
│  ELEKTRONIKA (mniej przydatne, ale coś)             │
├─────────────────────────────────────────────────────┤
│                                                     │
│  Główna płytka:                                     │
│  ├─ C1-C10: 100nF film 50V (bypass)  ✅            │
│  ├─ C11: 10µ 25V (zasilanie logiki)  ✅            │
│  ├─ Zasilacz: 47µ 50V, 100µ 25V      ✅✅         │
│                                                     │
│  Przydatne: ~3-5 kondensatorów                     │
└─────────────────────────────────────────────────────┘
```

### Wzmacniacz audio
```
┌─────────────────────────────────────────────────────┐
│  AUDIO AMP (ZŁOTO!)                                 │
├─────────────────────────────────────────────────────┤
│                                                     │
│  Zasilacz:                                          │
│  ├─ C1: 100µ 50V (na wejściu)          ✅✅✅     │
│  ├─ C2: 47µ 50V                         ✅✅      │
│  ├─ C3: 2200µ 50V (DUŻY!)              ✅ (ale ciężki) │
│                                                     │
│  Audio path:                                        │
│  ├─ C10: 10µ film 50V (coupling)       ✅          │
│  ├─ C11: 4.7µ film 50V                 ✅          │
│                                                     │
│  Audio amps: TONOWY 100µ, 47µ kondensatorów!       │
│                                                     │
└─────────────────────────────────────────────────────┘
```

### Zasilacz komputerowy (stare PSU)
```
┌─────────────────────────────────────────────────────┐
│  KOMPUTER PSU (SKARBIEC!)                           │
├─────────────────────────────────────────────────────┤
│                                                     │
│  INPUT (AC):                                        │
│  ├─ C1: 100µ 450V                       ✅          │
│  ├─ C2: 47µ 450V                        ✅          │
│                                                     │
│  +5V output rails (po 2-3 kondensatory):           │
│  ├─ 100µ 25V (x2-3)                     ✅✅✅     │
│  ├─ 47µ 25V (x2)                        ✅✅      │
│  ├─ 10µ film 50V (x2)                   ✅✅      │
│                                                     │
│  +12V output rails:                                 │
│  ├─ 100µ 50V (x2-3)                     ✅✅✅     │
│  ├─ 47µ 50V (x1)                        ✅         │
│                                                     │
│  Razem w PSU: 15-20 przydatnych kondensatorów!     │
│                                                     │
└─────────────────────────────────────────────────────┘
```

---

## 🔨 INSTRUKCJA DEMONTAŻU

### Etap 1: Rozładowanie kondensatorów

```
⚠️ KRYTYCZNE! Niektóre kondensatory mogą być naładowane na 450V!

PROCEDURA ROZŁADOWANIA:
───────────────────────

1. Włączyć multimetr w tryb V DC (Voltage Direct Current)
2. Podłączyć czarny przewód do GND
3. Dotknąć czerwonym przewodem do każdego kondensatora

  Jeśli multimetr pokazuje:
  ├─ 0V    → Bezpieczny do demontażu
  ├─ 10-50V  → Rozładuj przez rezystor 10kΩ
  └─ > 100V → BARDZO NIEBEZPIECZNY! Czekaj lub rozładuj

ROZŁADOWANIE:
  1. Weź rezystor 10kΩ (lub wyższy)
  2. Włóż go między (+) i (-) kondensatora na 5-10 sekund
  3. Czuj czy rezystor się grzeje (to normalne!)
  4. Sprawdź multimetrem - napięcie spadnie do ~0V
  5. Dopiero teraz możesz go zdemontować
```

### Etap 2: Rozlutowywanie

```
METODA 1: Lutownica + ssak (najlepiej)
─────────────────────────────────────

1. Lutownica: temperatura 350-380°C
2. Rozgrzej obydwa przewody kondensatora (~3 sekundy)
3. Gdy lut się rozpuści → użyj ssaka lutowniczego
4. Ostrożnie wciągnij roztopiony lut
5. Poczekaj aż lut stygnie
6. Delikatnie wyciągnij kondensator

METODA 2: Pompka lutownicza
──────────────────────────

1. Tak jak wyżej, ale zamiast ssaka użyj pompki
2. Po roztopieniu lutu: wciśnij gumkę, puść spust
3. Wciągnie roztopiony lut do beczki

METODA 3: Lut wyparowujący (najtrudniej)
──────────────────────────────────────

1. Rozgrzej lut do momentu aż zacznie parować
2. Wyciągnij żródło ciepła → lut fast styśnie
3. Chwyć kondensator pęsetą i wyciągnij

⚠️ OSTROŻNIE: Nie urywaj przewodów!
   Przewody mogą być potrzebne do dalszego montażu
```

### Etap 3: Czyszczenie

```
1. Fluks: może zostać czarny narost wokół
2. Roztwór izopropanolu (91%) - najlepiej
3. Mała szczoteczka do zębów (do wyrzucenia po)
4. Delikatnie pędzel po przewodach
5. Osuszanie: powietrze lub papieru kuchennego

Nowy kondensator:
├─ Przewody czyszczą? ✅
├─ Brak rdzawy? ✅
├─ Czytasz kod na obudowie? ✅
└─ Gotowy do użycia!
```

---

## ✅ SPRAWDZENIE KONDENSATORA PRZED UŻYCIEM

### Test multimetrem (tryb ~ lub Ω)

```
PROCEDURA TESTOWANIA:
──────────────────────

1. Utwórz multimetr na CAPACITANCE (C~) 
   (Jeśli dostępne - najlepsze)
   
2. Jeśli brak CAPACITANCE, użyj CONTINUITY (≈Ω):
   ├─ Dotknij przewodami kondensatora
   ├─ Multimetr powinien pokazać:
   │   - Na początek: 0Ω lub niski opór (ładuje się)
   │   - Po chwili: OPEN (∞) - kondensator sprawny!
   │
   └─ Jeśli stale 0Ω = ZWARTY (do wyrzucenia)

3. Jeśli multimetr ma CAPACITANCE:
   ├─ Powinien pokazać wartość bliską kodowi
   │   (np. kod "100µ" → multimetr 95-105µ)
   │
   ├─ Jeśli pokazuje 0 lub "OUT OF RANGE" → zły
   └─ Jeśli różni się >50% → wątpliwy, nie używaj

REZULTATY:
──────────
✅ DOBRY:      pokazuje wartość bliską kodowi, OPEN w trybie Ω
⚠️ WĄTPLIWY:   wymaga pomiaru kilka razy, niestabilny
❌ ZŁY:        0Ω (zwarty), OUT OF RANGE, bardzo inna wartość
```

### Test wizualny

```
❌ Nie używaj, jeśli:
├─ Obudowa pęknięta lub rozpakowana
├─ Fluks niezystyczony (może być wilgotny)
├─ Przewody zardzewiałe lub złamane
├─ Kondensator "wydyma" się na górze (elektrolit)
├─ Czarna substancja z obudowy (wycieki)
└─ Zapach dziwny (spalony)

✅ OK do użycia, jeśli:
├─ Kod czytelny na obudowie
├─ Przewody nieuszkodzę
├─ Brak wycieku
└─ Wygląda jak nowy (nawet używany)
```

---

## 📋 CHECKLIST: KTÓRE KONKRETNIE SZUKAĆ ZE STARYCH PŁYTEK?

### PRIORYTET 1 - MUSZĄ BYĆ (dla Twojego projektu):

```
┌─────────────────────────────────────────────────────┐
│ ⭐ 100µF 25V (elektrolit) - NAJWAŻNIEJSZY!         │
│   └─ Gdzie: Zasilacz TV/monitora (główny C)        │
│   └─ Ile: Minimum 2-3 szt (potrzebujesz 1-2)       │
│   └─ Kod: "100µ 25V" lub "100 25V"                 │
│                                                      │
│ ⭐ 100nF 50V (film) - DO CZUJNIKÓW!                │
│   └─ Gdzie: Wszędzie na płytce elektroniki        │
│   └─ Ile: Potrzebujesz 6-8, powinna być pełno      │
│   └─ Kod: "104" lub "100n 50V"                     │
│                                                      │
│ ⭐ 10µF film 50V - DO PWM                          │
│   └─ Gdzie: Audio path, zasilanie logiki           │
│   └─ Ile: Potrzebujesz 4-5                         │
│   └─ Kod: "103" lub "10µ 50V"                      │
│                                                      │
└─────────────────────────────────────────────────────┘
```

### PRIORYTET 2 - PRZYDATNE (zamienniki):

```
┌─────────────────────────────────────────────────────┐
│ 47µ 25V (elektrolit)                               │
│   └─ Zamiennik za 100µ (słabszy, ale ok)          │
│   └─ Gdzie: Zasilacze, audio amps                 │
│                                                      │
│ 10µ 50V (elektrolit)                               │
│   └─ OK do większości zastosowań                  │
│   └─ Gdzie: Zasilacz, logika                      │
│                                                      │
│ 1µ-4.7µ film 50V                                   │
│   └─ Do audio, sygnały                             │
│   └─ Gdzie: Audio amps                             │
│                                                      │
└─────────────────────────────────────────────────────┘
```

### PRIORYTET 3 - IGNORUJ (za duże lub za małe):

```
❌ 220µ-2200µ (za duże na Twoją aplikację)
❌ 450V-500V (potrzebujesz max 50V)
❌ 1nF-10nF (za małe na bypass)
❌ 1pF-100pF (za małe, bez sensu)
```

---

## 🎯 PRAKTYCZNY PLAN DZIAŁANIA

### Dzień 1: Zbiór płytek
```
[ ] Znaleźć 2-3 stare TV/monitory (zasilacze!)
[ ] Znaleźć zasilacz komputerowy (jeśli masz)
[ ] Znaleźć wzmacniacz audio (jeśli masz)
[ ] Razem: ~5-10 płytek do analizy
```

### Dzień 2: Analiza i selekcja
```
[ ] Demontaż zasilaczy (BEZPIECZNIE!)
[ ] Identyfikacja kondensatorów:
    ├─ Szukaj: 100µ 25V (najważniejszy)
    ├─ Szukaj: 100nF 50V (6+ szt)
    ├─ Szukaj: 10µ 50V (film, 4+ szt)
    └─ Inne: zapisz kod na kartce
[ ] Wstępny test multimetrem (CONTINUITY)
```

### Dzień 3: Czyszczenie i testowanie
```
[ ] Czyszczenie fluks-u (szczoteczka + izopropanol)
[ ] Osuszanie (papieru lub ciepłe powietrze)
[ ] Dokładny test multimetrem (Capacitance mode)
[ ] Sortowanie na: ✅ DOBRZE, ⚠️ WĄTPLIWE, ❌ ZŁE
```

### Dzień 4: Gotowość do użycia
```
[ ] Odłóż testowane kondensatory w pudełku
[ ] Etykietuj: "100µ 25V (OK)" itd.
[ ] Zapisz listę: ile masz każdej wartości
[ ] Jeśli brakuje → kupić za ~10 PLN online
```

---

## 💡 WSKAZÓWKI PRAKTYCZNE

### Jak zmaksymalizować zbiór?

```
1. TV/Monitor = NAJLEPSZE ŹRÓDŁO
   └─ Zasilacz ma 10-15 przydatnych kondensatorów
   └─ Nie rób z monitorami CRT (wysoka temperatura!)

2. Zasilacz komputerowy PSU
   └─ Możliwe że będzie mieć 20+ kondensatorów
   └─ Ale trudno dostać się do nich

3. Elektronika konsumencka
   └─ Dekodery, tuner, odbiorniki
   └─ Mniej przydatnych, ale coś się zawsze uda

4. Serwis elektroniki
   └─ Czasem mają stos starych płytek
   └─ Poproś o resztki - mogą dać za darmo!
```

### Bezpieczeństwo

```
⚠️ NIEBEZPIECZNE CZĘŚCI:
├─ WYSOKIE NAPIĘCIA (450V) na kondensatorach zasilacza
├─ Transformatory strującego - mogą być namagnesowane
└─ Poprzednie zasilanie może pozostać

✅ ZAWSZE:
├─ Rozładuj kondensatory przed demontażem!
├─ Używaj pęsety, nie palców
├─ Wyłącz zasilanie 2-3 minuty przed demontażem
└─ Pracuj w czystym, suchym otoczeniu
```

---

## 📝 SZABLON - ZAPISZ CO ZNALAZŁEŚ

```
┌─────────────────────────────────────────────┐
│ PŁYTKA: TV Samsung 32" (zasilacz)           │
│ DATA: 16.05.2026                            │
│ STATUS: ✅ Rozmontowana                    │
├─────────────────────────────────────────────┤
│ Znalezione kondensatory:                    │
│ ├─ 100µ 25V     (x3) → ✅ DOBRZE           │
│ ├─ 47µ 25V      (x1) → ✅ DOBRZE           │
│ ├─ 100nF 50V    (x5) → ✅ DOBRZE           │
│ ├─ 10µ film 50V (x2) → ✅ DOBRZE           │
│ └─ 2200µ 50V    (x1) → ⚠️ Duży, może być  │
│                                             │
│ RAZEM PRZYDATNYCH: 12 szt                  │
│                                             │
│ NOTATKA: Zasilacz był czysty, bez wycieku │
└─────────────────────────────────────────────┘
```

---

**PODSUMOWANIE**: Ze starych TV/dekoderów powinieneś wyłowić co najmniej:
- 2-5 x 100µ 25V ✅
- 5-10 x 100nF 50V ✅
- 3-5 x 10µ film 50V ✅

To wystarczy do Twojego projektu + ma rezerwa!

**Data**: 2026-05-16  
**Wersja**: 1.0
