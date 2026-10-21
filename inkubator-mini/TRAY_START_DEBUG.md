Rozumiem, że czujesz, że "kalendarz nie działa" – próbujesz ustawić datę startu, a dzień inkubacji nie zmienia się tak, jak oczekujesz.
Sprawdźmy, co się dzieje i jak to naprawić.

---

## 1. Dlaczego dzień jest liczony tak, a nie inaczej?
Funkcja `setTrayStart()` oblicza dzień w ten sposób:

```javascript
const today = new Date();           // dzisiejsza data i godzina (np. 25.06.2026 10:00)
const start = new Date(date + 'T' + time);  // data i godzina startu (np. 18.06.2026 19:00)
const diffDays = Math.floor((today - start) / (1000 * 60 * 60 * 24)) + 1;
```

- Różnica w milisekundach jest dzielona przez liczbę ms w dobie, zaokrąglana w dół, a potem dodawane jest **+1**.
- Jeśli dzisiaj jest **25.06.2026** o **10:00**, a start to **18.06.2026 o 19:00**, to:

- Czas między 18.06 19:00 a 25.06 10:00 to **6 dni i 15 godzin** → `Math.floor` daje 6, +1 = **7**.
- Dlatego widzisz **Dzień 7** (a nie 8).
Gdyby start był **17.06.2026 o 19:00**, to różnica wynosiłaby 7 dni i 15 godzin → floor 7 + 1 = **8** (czyli 8 dzień inkubacji).
Jeśli chcesz, żeby dzień był inny, musisz zmienić datę startu na właściwą i **kliknąć „Ustaw start”**, aby przeliczyć i wysłać nową wartość do ESP.

---

## 2. Dlaczego „kalendarz nie działa”?
Mogą być dwa powody:

### A. Data nie jest zapisywana w ESP

- Przycisk **„Ustaw start”** wysyła tylko obliczony dzień do endpointu `/api/tray/set-day`.
- **Nie wysyła** daty startu do ESP – data startu jest przechowywana **lokalnie** w przeglądarce (w `localStorage`) i wyświetlana w polach, ale ESP zna tylko **dzień inkubacji**.
- Jeśli połączenie z ESP jest przerywane lub endpoint nie odpowiada, zobaczysz komunikat „✗ Brak połączenia” i dzień się nie zmieni.

### B. Data w polu nie jest poprawnie odczytywana

- Pole `type="date"` wymaga formatu `YYYY-MM-DD`. Jeśli wpisujesz ręcznie np. `18.06.2026`, może być ono niepoprawne (przeglądarka oczekuje formatu `2026-06-18`).
- Użyj wbudowanego kalendarza (kliknij w pole) albo wpisz datę w formacie `2026-06-18`.

---

## 3. Co możesz zrobić, aby działało?

### Opcja A – Ustaw dzień ręcznie (najszybsze)
W sekcji tacy masz pole **„Dzień”** z liczbą i przycisk **„Ustaw”** obok.

- Wpisz tam **8** (lub dowolną liczbę).
- Kliknij **„Ustaw”** – wywoła to funkcję `setTrayDay(i)`, która wyśle dzień bezpośrednio do ESP.
- To jest najprostszy sposób na wymuszenie poprawnego dnia, niezależnie od daty startu.

### Opcja B – Popraw datę i użyj „Ustaw start”

1. Upewnij się, że data jest w formacie **`YYYY-MM-DD`** (np. `2026-06-17`).
2. Ustaw godzinę (np. `19:00`).
3. Kliknij **„Ustaw start”**.
4. Sprawdź, czy w konsoli (F12) nie ma błędów. Jeśli pojawi się „✗ Brak połączenia” – to oznacza, że ESP nie odpowiada na `/api/tray/set-day`. W takim przypadku użyj opcji A.

### Opcja C – Sprawdź, czy data jest zapamiętywana
Po poprawnym ustawieniu startu, data zostanie zapisana w `localStorage`.
Przy następnym załadowaniu strony pola zostaną automatycznie wypełnione.
Jeśli tak się nie dzieje, to znaczy, że zapis w `localStorage` nie działa (np. przeglądarka blokuje localStorage w trybie prywatnym).

---

## 4. Dlaczego po zmianie daty dzień nie aktualizuje się automatycznie?

- Pola **„Data startu”** i **„Godzina startu”** są jedynie **formularzem** – nie są powiązane z ESP.
- Dopiero kliknięcie **„Ustaw start”** oblicza dzień i wysyła go do ESP.
- Po udanym zapisie, `fetchData()` odświeża dane z ESP i aktualizuje wszystkie wskaźniki.
- Jeśli chcesz, żeby dzień aktualizował się na bieżąco bez klikania, możesz dodać automatyczne przeliczanie przy zmianie daty, ale to wymagałoby zmiany kodu.

---

## Podsumowanie

- **Aktualny dzień** jest obliczany poprawnie na podstawie daty startu i aktualnej godziny.
- Jeśli chcesz ustawić konkretny dzień, użyj przycisku **„Ustaw”** obok pola `Dzień`.
- Jeśli data startu nie jest zapamiętywana lub nie działa kalendarz, sprawdź, czy używasz formatu `YYYY-MM-DD` i czy przeglądarka nie blokuje localStorage.
- W razie dalszych problemów – otwórz konsolę (F12), kliknij „Ustaw start” i zobacz, czy pojawia się błąd. Jeśli tak, wklej go tutaj – pomogę go rozwiązać.
