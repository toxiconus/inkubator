# QUICK REFERENCE - Szybka ściąga do szukania kondensatorów

## 🔍 CO KONKRETNIE SZUKAĆ? (NAJWAŻNIEJSZE!)

```
SZUKAJ:                          GDZIE:                    ILOŚĆ POTRZEBNA:
─────────────────────────────────────────────────────────────────────────

⭐⭐⭐ 100µ 25V (cylindryczny)   Zasilacz TV/Monitor      Minimum 2 szt
                                 + Audio amp              (do LM2596)
                                                          
⭐⭐⭐ 100nF 50V (prostokątny)    Wszędzie na płytce       Minimum 6 szt
       kod "104"                                          (bypass czujniki)

⭐⭐  10µ 50V (film)             Audio path               Minimum 4 szt
       kod "103"                Zasilanie logiki         (PWM filtracja)

⭐   47µ 25V (elektrolit)        Zasilacz, Audio         Minimum 1 szt
                                                          (zamiennik za 100µ)
```

---

## 🎯 SZYBKI PLAN

```
ETAP 1 (5 minut):
┌─────────────────────────────────┐
│ [ ] Weź multimetr               │
│ [ ] Ustaw na V DC (sprawdzanie)  │
│ [ ] Rozładuj KAŻDY kondensator!  │
│     (dotknij - powinno być 0V)   │
└─────────────────────────────────┘

ETAP 2 (20 minut - demontaż):
┌─────────────────────────────────┐
│ [ ] Lutownica 350°C             │
│ [ ] Ssak/pompka lutownicza       │
│ [ ] Pęseta                       │
│ [ ] Wyciągaj ostrożnie (2-3 sek) │
│ [ ] Każdy kondensator w pudełko  │
└─────────────────────────────────┘

ETAP 3 (10 minut - czyszczenie):
┌─────────────────────────────────┐
│ [ ] Izopropanol (91%)           │
│ [ ] Szczoteczka do zębów        │
│ [ ] Wycieraj przepisy fluks-u    │
│ [ ] Osusz (papier, ciepło)       │
└─────────────────────────────────┘

ETAP 4 (10 minut - test):
┌─────────────────────────────────┐
│ [ ] Multimetr na CAPACITANCE     │
│ [ ] Dotknij obydwa przewody      │
│ [ ] Powinna być liczba bliska    │
│       kodowi na obudowie         │
│ [ ] OK? → Do pudełka DOBRZE      │
│ [ ] Wątpliwe? → Pudełko WĄTPLIWE │
│ [ ] Zwarty? → Wyrzuć             │
└─────────────────────────────────┘
```

---

## 📋 IDENTYFIKACJA KODU (30 SEKUND)

### ELEKTROLIT (cylindryczny)
```
Szukasz:  "100µ 25V" lub "100u 25" lub "100µ"
          └─ Pierwszy numer = pojemność
             Drugi numer (25V, 50V) = max napięcie

Przykłady czytane:
│ 100µ │  = 100 mikrofarad
│ 50V  │  = do 50 woltów
│ +85  │  = temp. max 85°C
│ 105h │  = żywotność

Dla Ciebie: 25V, 50V → OK
            100V+ → za duże
```

### FILM (prostokątny, pachnący azotkiem?)
```
Szukasz:  "104" (= 100nF) lub "103" (= 10µ) lub "100n 50V"

Kod 3-cyfrowy:
│ 1 0 4 │
├─────┘
│ Pierwsze 2 cyfry  (10)
│ × 10^(ostatnia)   (10^4 = 10000)
│ = 10 × 10000 pF = 100000 pF = 100 nF ✅

Kodów które szukasz:
│ 104  →  100 nF  ✅ (100 n F - najczęściej)
│ 103  →  10 µF   ✅ (film - rzadszy)
│ 105  →  1 µF    ⚠️  (małowartościowy)
│ 224  →  220 nF  ✅ (ok, zamiennik za 100nF)
```

### TANTALOWY (czarny, mały - premium)
```
Szukasz: "100µ 10V" (tantalowy jest drogi, ale dobry!)

Dla Ciebie:
│ 100µ 10V  → ✅✅✅ IDEAŁ do LM2596 wyjścia
│ 47µ 10V   → ✅ Zamiennik
│ 10µ 10V   → ⚠️ Mały ale ok
```

---

## ❌ CO IGNOROWAĆ (Szybka selekcja)

```
IGNORUJ:                          DLACZEGO:
──────────────────────────────────────────────────────
220µ, 470µ, 1000µ                Za duże na 5V logikę
(bardzo duże cylindry)            
                                  
450V, 500V kondensatory           Zbyt wysokie napięcie
(na zasilaczu AC)                 (potrzebujesz max 50V)

Kondensatory ze śladami rdzy      Mogą być wewnętrznie
lub zmalinami                     uszkodzane
                                  
1nF, 10nF, 100pF                  Za małe (wymagaj >= 100nF)
(bardzo małe wartości)
```

---

## 📊 GDZIE ZNALEŹĆ KAŻDY TYP?

```
TV / Monitor LCD
├─ Zasilacz (power supply)
│  ├─ 100µ 25V x2-3 (GŁÓWNE!)
│  ├─ 47µ 25V x1-2
│  ├─ 100nF 50V x3-5
│  └─ 10µ film x1-2
│
└─ Elektronika (płytka główna)
   ├─ 100nF 50V x5-10 (bypass)
   └─ 10µ 25V x1-2

Audio amplifier
├─ Zasilacz:
│  ├─ 100µ 50V x2-3 (ZŁOTO!)
│  ├─ 47µ 50V x1-2
│  └─ 2200µ 50V (ale ciężki)
│
└─ Audio path:
   ├─ 100nF 50V x5-10
   ├─ 10µ film 50V x2-3
   └─ 4.7µ film 50V x2

Zasilacz komputerowy (PSU)
├─ INPUT (AC):
│  └─ 100µ 450V, 47µ 450V
│
├─ +5V output:
│  ├─ 100µ 25V x2-3 ✅
│  ├─ 47µ 25V x1-2
│  └─ 10µ film 50V x1-2
│
└─ +12V output:
   ├─ 100µ 50V x2
   ├─ 47µ 50V x1
   └─ 10µ film 50V x1-2

RAZEM Z JEDNEGO PSU: ~15-20 przydatnych! 🎯
```

---

## 🛠️ NARZĘDZIA (MUSZĄ BYĆ!)

```
OBOWIĄZKOWE:
[ ] Lutownica (25-40W) lub stacja lutownicza
[ ] Ssak lutowniczy (lub pompka) - DO ODSYSANIA LUTU
[ ] Pęseta (metalowa, nie plastikowa)
[ ] Multimetr (tanie 20-50 PLN wystarczą)

PRZYDATNE:
[ ] Drut lutowniczy bez fluks-u
[ ] Fluks w pióro
[ ] Izopropanol (91%) - do czyszczenia
[ ] Szczoteczka do zębów (nowa, do wyrzucenia)
[ ] Papier kuchenny
[ ] Rezystor 10kΩ do rozładowywania
[ ] Pudełka/etykiety do sortowania

ZAREZERWOWANA:
[ ] Okulary powiększające
[ ] Lampka robocza
[ ] Mata na biurko (izolacyjna, anty-ESD)
```

---

## 🚨 ZAGROŻENIA - CO MOŻE PÓJŚĆ ŹLE

```
ZAGROŻENIE              SYMPTOM                   ROZWIĄZANIE
────────────────────────────────────────────────────────────────
Naładowany             Zaboli, bo 450V!          Rozładuj rezystorem!
kondensator            Błyskawica                (czekaj 5 sec)
                       
Błędy demontażu        Spadł się przewód         Użyj pęsety, nie palców
                       Spadł się kondensator
                       
Zła identyfikacja      Montaż tego co trzeba     Sprawdzaj kod 3x!
                       Urządzenie nie działa     Czytaj KAŻDY bit
                       
Zwarty kondensator     Zasilanie się zwiera      TEST multimetrem
                       Całe urządzenie się spala Przed montażem!
                       
Kontakt + rezydua      Korozja połączeń          Czyszcz izopropanolem
fluks-u                Utleniana złota           + szczotka
```

---

## ✅ CHECKLIST PRZED UŻYCIEM

```
KONDENSATOR GOTOWY DO UŻYCIA?

[ ] Kod na obudowie czytany (np. "100µ 25V")
[ ] Przewody nieuszkodzenie
[ ] Multimetr pokazuje wartość bliską kodowi
[ ] Brak zapyszczenia ani wycieku
[ ] Czysty, bez fluks-u
[ ] Nie jest zwartych (multimetr nie pokazuje 0Ω)
[ ] Wartość mieści się w specyfikacji (25V lub 50V)

WSZYSTKO OK? → Do użycia! ✅
WĄTPLIWOŚĆ? → Nie używaj! (lepiej kupić nowy za 50 groszy)
```

---

## 📞 QUICK MATH (Szybka konwersja kodów)

```
Kod 3-cyfrowy:  AB(C)
  A × 10^B = wartość w pF

104 = 1 × 10^4 pF   = 10 nF    (= 0.01 µF)   ❌ Mały
105 = 1 × 10^5 pF   = 100 nF   (= 0.1 µF)    ⚠️ Średni
106 = 1 × 10^6 pF   = 1 µF     (= 1 µF)      ⚠️ Średni

224 = 22 × 10^4 pF  = 220 nF   (= 0.22 µF)   ✅ OK
225 = 22 × 10^5 pF  = 2.2 µF   (= 2.2 µF)    ✅ OK

104 = 10 × 10^4 pF  = 100 nF   (= 0.1 µF)    ✅✅✅ NAJLEPSZY
474 = 47 × 10^4 pF  = 470 nF   (= 0.47 µF)   ✅ OK
684 = 68 × 10^4 pF  = 680 nF   (= 0.68 µF)   ✅ OK

Dla Twojego projektu szukaj:
├─ 104 (100nF) - najczęściej
├─ 103 (10nF) lub 224 (220nF) - rzadsze
└─ Wszystko w ±20% to zadziała
```

---

## 🎯 MOJA RADA - PRIORITETY

```
DZISIAJ (szybko):
1. Zbierz stare TV/monitor/zasilacz
2. Demontaż zasilacza (najważniejszy!!)
3. Szukaj kodu "100µ" na kondensatorach
   
JUTRO:
4. Czyszczenie i testowanie
5. Policzy ile masz każdej wartości

PONAD TYDZIEŃ:
6. Jeśli brakuje → kupić za 10 PLN online
   (bo już będziesz mieć 80% tego co trzeba!)
```

---

**Data**: 2026-05-16  
**Wersja**: 1.0 - QUICK REFERENCE
**Status**: Drukuj to i weź do ręki gdy demontaż! 📄
