# Ready - kroki implementacyjne

Poniżej znajduje się gotowa lista kroków implementacyjnych do wdrożenia w UI i harmonogramie. Sekcja ta jest oddzielona od przeglądu, aby zachować czytelność oceny kodu.

## Stan wykonania

- [x] Surowy PWM tac 0-225 i walidacja `PWM 0-225`.
- [x] Ujednolicone wejścia tac w `Status` i `Panel`.
- [x] Uproszczony oscylator temperatury w zakładce `Status`.
- [x] Przebudowane `firmware/data/index.html` z `firmware/data/dev/index.template.html`.
- [x] Konflikty harmonogramu i profile inkubacji (podstawowa detekcja konfliktów gotowa).
- [x] Obsługa kanałów CH2/CH3 w UI i backendzie.
- [x] Zaawansowana wizualizacja dyfuzorów.

## Status
- Rozszerzono sterowanie z 2 do 4 kanałów grzewczych CH0–CH3 w UI.
- Backend API teraz przyjmuje surowe PWM 0-225 dla CH0–CH3 i zwraca `heater_pwm_ch2/3`.
- Utrzymano oddzielne ustawienia impulsów mocy dla każdego kanału.
- Gotowe podstawowe detekcje konfliktów w harmonogramie.

## 1. Scalenie Status + Panel

- [x] Stwórz jedną zakładkę `Status`, która łączy kluczowe elementy ze starego `Status` i `Panel`.
- [x] Zmień nazwę zakładki `Panel` na `Wykresy`.
- [x] W górnej sekcji umieść cztery zegary:
   - dodaj cienki zielony marker celu temperatury na tarczy,
   - dla wilgotności pokaż dwa markery (min/max) i wypełniony zielony łuk między nimi,
   - DS18B20 #1/#2 wyświetl w jednym module z dwoma wartościami i jedną główną wskazówką.
- [x] Zachowaj sekcję ΔT/ΔH/Moc/Równowaga bez większych zmian.
- [x] W widoku `Status` pokaż uproszczony „oscylator” zamiast pełnego wykresu historycznego:
   - czarna linia aktualnej wartości,
   - zielony pas zakresu,
   - biała linia celu.
- [x] Zastąp obecne switche grzałek kartami zawierającymi:
   - dużą wartość PWM 0-225,
   - suwak ręcznej regulacji,
   - toggle AUTO/RĘCZNY,
   - diodę statusu: szara=OFF, zielona=pulsująca AUTO, żółta=ręczny,
   - dodatkowy przełącznik ON/OFF.
- [x] Przestaw wszystkie suwaki tac na surowy zakres PWM 0-225 zamiast 0-100%.
- [x] Usuń procentową notację z wyświetlanych wartości PWM tac i statusu.
- [x] Zaimplementuj walidację PWM 0-225 i spójne komunikaty ruchu tacek.
- [x] Odbuduj `firmware/data/index.html` z `firmware/data/dev/index.template.html` po zmianach.
- [x] Użyj `grid` z `align-items: start` dla kart tac, aby uniknąć rozjeżdżania się wysokości.
- [x] W widoku `Status` pokaż uproszczony „oscylator” zamiast pełnego wykresu historycznego:
   - czarna linia aktualnej wartości,
   - zielony pas zakresu,
   - biała linia celu.
- [x] Pełne wykresy historyczne przenieś do osobnej zakładki `Wykresy`.

## 2. Harmonogram i konflikty

1. Dodaj mechanizm porównywania parametrów obu tac na każdy dzień.
2. Oblicz overlapping zakresów temperatury i wilgotności.
3. Ustal trzy poziomy sygnalizacji:
   - zielony = zakresy się pokrywają,
   - żółty = kompromis możliwy,
   - czerwony = poważny konflikt.
4. Dodaj kolumnę `Konflikt` do tabeli harmonogramu, widoczną gdy aktywne są różne profile.
5. Wdróż system profili inkubacji zamiast sztywnych gatunków:
   - `Jaja ptasie`, `Jaja gadów`, `Grzyby`, `Nasiona i kiełki`, `Fermentacja`, `Ogólny/Custom`.
6. Profil powinien zawierać:
   - nazwę,
   - zakres temperatur,
   - zakres wilgotności,
   - czas trwania,
   - obroty TAK/NIE,
   - fazy i zmiany parametrów.
7. Dodaj obsługę opcjonalnych kanałów CH2/CH3 jako dodatkowe grzałki per profil.

## 3. Wizualizacja Status w stylu Pip-Boy

1. Zmień grzałki na aktywne wskaźniki VU-meter:
   - słupek rosnący od zielonego do czerwonego,
   - dioda stanu obok słupka.
2. Tace oznacz animowanym spinnerem podczas ruchu.
3. Dyfuzory reprezentuj ikoną pary z animacją unoszenia.
4. Zorganizuj panel `Kontrola Mocy` jako jeden spójny panel boczny:
   - grzałki,
   - dyfuzory,
   - tace,
   - tabela mocy nominalnej.
5. Zmień timer inkubacji na „Dzień X / Y” + pasek postępu z kolorami.

## 4. Historia i storage w przeglądarce

1. Przechowuj dane tylko w przeglądarce, gdy strona jest aktywna i połączona.
2. Nie zapisuj pełnej historii do flasha.
3. Zamiast tego:
   - używaj `localStorage` do krótkotrwałej persystencji UI,
   - zapamiętuj tylko agregaty lub stan sesji,
   - egzekwuj to tylko dla połączonej przeglądarki.
4. Krytyczne zdarzenia zapisuj osobno:
   - alarmy krytyczne,
   - utrata sieci,
   - awarie grzałki / temperatura poza zakresem.
5. Przy starcie odczytuj z `localStorage` ostatni stan i przywróć tylko krytyczne alarmy oraz istotne metadane.
6. Jeśli chcesz więcej trwałości, lepszym rozwiązaniem jest API z ESP32 zamiast zapisu do flash po stronie przeglądarki.

## 5. Priorytety

1. Naprawić natychmiast:
   - zduplikowane ID grzałek,
   - niespójny panel `Kontrola Mocy`,
   - brakujące `setHeaterManual`.
2. Wdrożyć następnie:
   - scalenie `Status + Panel`,
   - nowe karty grzałek,
   - live oscylator w `Status`.
3. Potem:
   - konflikty harmonogramu,
   - profile inkubacji,
   - dodatkowe kanały grzewcze.
4. Docelowo:
   - historia po stronie ESP32,
   - lepsze zarządzanie persystencją danych,
   - osobna zakładka `Wykresy`.
