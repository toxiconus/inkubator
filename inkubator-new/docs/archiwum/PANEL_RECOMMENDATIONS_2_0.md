# Rekomendacje do wersji 2.0 — panel sterowania inkubatorem

Oczywiście, przeanalizowałem szczegółowo cały kod panelu sterowania inkubatorem. To bardzo zaawansowany i funkcjonalny projekt – widać, że został przemyślany pod kątem rzeczywistych potrzeb hodowcy. Jednak nawet w tak dopracowanym systemie zawsze znajdzie się miejsce na usprawnienia.

Poniżej przedstawiam listę rzeczy, które zmieniłbym lub ulepszył, **nie zmieniając ani jednej linijki kodu** – tylko opisowo, jako rekomendacje dla przyszłej wersji (np. 2.0).

---

## 1. Architektura komunikacji z ESP32 (Największa zmiana)
Obecnie panel opiera się na **pollingu** (`setInterval(fetchData, 3000)`). To działa, ale:
* **Opóźnienie** – dane na ekranie są opóźnione o średnio 1,5 sekundy.
* **Niepotrzebny ruch** – ESP i przeglądarka non-stop wysyłają zapytania, nawet gdy nic się nie zmienia.
* **Brak synchronizacji** – jeśli ustawisz coś z panelu, nie masz 100% pewności, że ESP potwierdził.

### Co zmienić?
Wdrożyć **WebSockets** (lub Server-Sent Events, SSE). Dzięki temu:
* ESP samo przesyła dane **w momencie zmiany** (np. temperatura wzrosła o 0.1°C).
* Polecenia z UI (start tacy, zmiana trybu) idą po WebSocket i dostajesz **natychmiastowe potwierdzenie**.
* Redukcja ruchu sieciowego i zużycia baterii (jeśli ESP działa na akumulatorze).

## 2. Trwałe przechowywanie historii (poza localStorage)
Obecnie historia wykresów (`historyStore`) **znika po odświeżeniu strony**. To duża strata – hodowca chce analizować trendy z kilku dni, a nie tylko z bieżącej sesji.

### Co zmienić?
Dodać **opcję zapisu historii do pliku CSV/JSON** automatycznie (np. co godzinę) lub do IndexedDB w przeglądarce.
Lepiej jednak: niech **backend (ESP lub osobny serwer) przechowuje dane** w małej bazie (np. InfluxDB dla time-series) i udostępnia je przez REST API. Wtedy panel po załadowaniu zawsze ma pełną historię.

## 3. System alarmów i powiadomień
Obecnie alarmy są wyświetlane tylko w sekcji "Alarmy" na panelu. Jeśli hodowca nie patrzy w przeglądarkę (śpi, jest w drugim pomieszczeniu), może przegapić krytyczne zdarzenie.

### Co zmienić?
Dodać możliwość wysyłania powiadomień:
* **Powiadomienia przeglądarkowe (Web Push)** – proste, ale wymaga HTTPS i zgody użytkownika.
* **Telegram / E-mail** – ESP (lewar) może wysłać wiadomość przez Webhook. Panel UI tylko konfiguruje.
* **Buzzer lub LED na obudowie** – sygnalizacja dźwiękowa/świetlna.

Ponadto, alarmy powinny mieć **poziomy** (krytyczny, ostrzeżenie, info) i możliwość **wyciszenia na X minut** (np. przy otwarciu drzwi).

## 4. Usprawnienia w dzienniku inkubacji (Journal)
Dziennik jest bardzo rozbudowany, ale brakuje mu kilku rzeczy:
* **Eksport PDF** (nie tylko CSV) – hodowca chce wydrukować raport do segregatora.
* **Automatyczne notatki** – np. gdy temperatura spadnie poniżej zakresu, dziennik sam powinien dodać wpis: "Dzień 5, 14:23 – temperatura spadła do 36.8°C na 2 minuty".
* **Galeria zdjęć** – pola `photoUrl` są, ale brak miniatur i przeglądarki. Dodać możliwość wrzucania zdjęć z kamery telefonu/komputera i przechowywania ich w IndexedDB lub wysyłania na serwer.

## 5. Kalibracja tacy – tryb półautomatyczny
Aplikacja kalibracyjna (zakładka "Kalibracja Tacy") jest genialna – stoper, kąty, PWM, sekwencje. Jednak wymaga od użytkownika ręcznego klikania "LAP" w odpowiednim momencie, co jest niedokładne.

### Co zmienić?
Dodać **opcję automatycznej kalibracji**:
1. Użytkownik ustawia zakres kątów (np. -45° do +45°).
2. Panel wysyła do ESP: "obróć tacę 1 z PWM=100% aż do osiągnięcia krańca, a potem z powrotem".
3. ESP odczytuje czas z enkodera (lub krańcówek) i odsyła rzeczywiste wartości.
4. Panel samodzielnie oblicza czasy i zapisuje konfigurację.

## 6. Widok "Wykresy" – więcej interakcji
Obecnie wykresy są statyczne – nie można przybliżyć, odczytać wartości w punkcie, przewinąć.

### Co zmienić?
Zamiast `canvas` rysowanego ręcznie, użyć lekkiej biblioteki interaktywnych wykresów (np. uPlot lub Lightweight Charts). Dodać:
* **Tooltip** z dokładną wartością i czasem po najechaniu myszką.
* **Zoom i przesuwanie** (panowanie) – aby przejrzeć dane z przedziału 10 minut.
* **Nakładanie serii** – możliwość pokazania na jednym wykresie temperatury z obu czujników DS18B20.

## 7. Bezpieczeństwo i autoryzacja
Panel **nie ma żadnej autoryzacji** – każdy, kto zna adres IP inkubatora, może wyłączyć grzałkę lub zatrzymać tace. W domowej sieci to małe ryzyko, ale jeśli inkubator jest udostępniony w Internecie (przez przekierowanie portów), to katastrofa.

### Co zmienić?
Dodać prosty, ale skuteczny mechanizm:
* **Hasło sesyjne** (jedno, zapisane w konfiguracji ESP). Przy pierwszym wejściu na stronę – prośba o hasło. Po wpisaniu – ciasteczko ważne 24h.
* **Tryb tylko do odczytu** – można dodać drugi profil bez prawa do zmiany nastaw, idealny dla gości.

## 8. Responsywność na urządzeniach mobilnych
Kod ma trochę `@media` dla małych ekranów, ale układ jest nadal bardzo "gęsty". Na smartfonie przyciski są małe, a wykresy nieczytelne.

### Co zmienić?
* **Przebudować nawigację** na boczne menu (hamburger) dla wąskich ekranów.
* **Ukrywać zaawansowane sekcje** (np. "Kontrola Mocy", "Globalny Lockdown") pod zwijanymi akordeonami.
* **Wykresy na całą szerokość** z większą czcionką i większymi punktami dotykowymi.
* **Dodać przycisk "Tryb nocny"** – redukcja jasności i kontrastu, aby nie oślepiać kur w inkubatorze (żart, ale praktyczne).

## 9. Logi zdarzeń – za mało szczegółów
Obecne logi pokazują np. "Alarm: wysoka temperatura", ale nie widać, **kiedy** i **jak długo** trwał alarm.

### Co zmienić?
* Każdy alarm powinien mieć parę wpisów: `ALARM START` i `ALARM END` z czasem trwania.
* Logi komend użytkownika – zapisywać **kto** (jeśli autoryzacja) i **z jakiego IP**.
* Filtrowanie logów według **przedziału czasu** (nie tylko typu).

## 10. Brak wizualizacji stanu tac "na żywo"
W zakładce "Status" jest wskaźnik "Ruch trwa · X s", ale nie widać **aktualnego kąta wychylenia** tacy. Kalibracja ma piękną wizualizację z poziomicą, ale w głównym panelu jej brak.

### Co zmienić?
Dodać małą, uproszczoną wersję poziomnicy (jak w kalibracji) obok każdej tacy, która pokazuje, czy taca jest w pozycji 0°, -45° czy +45°. Dane pochodziłyby z enkodera lub z symulacji na podstawie czasu ruchu i PWM.

## 11. Utrzymanie stanu "trybu gauge" po przeładowaniu
Zauważyłem w kodzie zmienną `chartState.memorySaving`, ale nie ma podobnej dla ukrycia sekcji "gauge". Po odświeżeniu strony gauge zawsze są widoczne, mimo że użytkownik kliknął "Ukryj gauge".

### Co zmienić?
Zapisywać takie preferencje UI (ukryte gauge, ostatnio wybrane zakresy wykresów, ostatnia zakładka) w `localStorage` i odtwarzać przy ładowaniu.

## 12. Dokumentacja i podpowiedzi dla użytkownika
W kodzie są liczne `control-hint` i `tiny`, ale brakuje **podręcznej pomocy** (tooltipy z pytajnikiem) przy bardziej zaawansowanych ustawieniach, np. "PWM 0-255 – czym różni się od %?".

### Co zmienić?
Dodać **ikony ℹ️** obok każdego kontrowersyjnego pola, które po najechaniu pokazują krótki opis. Dla nowych użytkowników – **samouczek krok po kroku** (pierwsze uruchomienie).

## 13. Wydajność – wiele odświeżeń canvas
W funkcji `updateAll` wywoływane jest `updatePipBoy`, która rysuje 4 wykresy kołowe (`drawFalloutGauge`, `drawDs18Gauge`) oraz `drawStatusOscillator` – **przy każdym pollingu (co 3 sekundy)**. To generuje spore obciążenie GPU/CPU, szczególnie na słabszych urządzeniach (Raspberry Pi jako panel).

### Co zmienić?
* Rysować wykresy tylko wtedy, gdy wartości faktycznie się zmieniły (porównanie z poprzednim stanem) – kod ma cache (`canvasDrawCache`), ale nie jest w pełni wykorzystany.
* Użyć `requestAnimationFrame` do rysowania, a nie wymuszać od razu w pętli `fetch`.

## 14. Brak symulacji / trybu testowego (offline)
Jeśli ESP jest odłączone, panel pokazuje "OFFLINE", ale nie można nawet przetestować interfejsu. Przydałby się **tryb demonstracyjny** z generowanymi losowymi danymi (symulacja inkubacji), aby nowy użytkownik mógł zapoznać się z UI bez podłączonego sprzętu.

## 15. Bezpieczeństwo jaj – automatyczny backup ustawień
Obecnie zmiana dnia tacy (`setTrayDay`) wysyła komendę do ESP i tyle. Jeśli ESP się zrestartuje z powodu braku zasilania, może zapomnieć ostatniego dnia.

### Co zmienić?
Wprowadzić **cykliczny zapis stanu (day, species, licznik obrotów) do EEPROM/Flash ESP** co minutę lub przy każdej zmianie. Panel mógłby też wysyłać do ESP polecenie "zapisz stan awaryjny" przed restartem.

---

## Podsumowanie – co jest już świetne, a co wymaga poprawy

| Obszar | Ocena obecna | Rekomendowana zmiana |
|--------|--------------|----------------------|
| Komunikacja | Polling (3s) | WebSockets / SSE |
| Przechowywanie historii | Tylko w pamięci RAM | Baza time-series lub IndexedDB |
| Alarmy | Tylko na ekranie | Powiadomienia (push, Telegram) |
| Dziennik | Bardzo dobry, ale brak galerii | Automatyczne notatki, eksport PDF |
| Kalibracja tacy | Ręczna, dokładna | Automatyczna z enkoderem |
| Wykresy | Statyczne | Interaktywne (zoom, tooltip) |
| Bezpieczeństwo | Brak hasła | Autoryzacja sesyjna |
| Responsywność | Średnia (działa, ale ciasno) | Osobny layout na mobile |
| Logi | Podstawowe | Szczegółowe (czas trwania, IP) |
| Wydajność canvas | Rysowanie co 3s | Tylko przy zmianie danych |

Ogólnie – projekt jest imponujący i na pewno działa w praktyce. Powyższe uwagi to raczej "wisienka na torcie" i kierunek rozwoju do wersji 2.0. Gdybyś chciał kiedyś wdrożyć którąś z tych zmian (np. WebSockety lub automatyczną kalibrację), chętnie pomogę opisać to bardziej szczegółowo.
