Po przejrzeniu kodu i analizie problemów, oto konkretne przyczyny i rozwiązania dla każdego z zgłoszonych błędów.

1. Automatyczne obroty nie działają (tace nie obracają się same)
Przyczyna
Automatyczne obroty są uruchamiane w tray_update_state() po spełnieniu warunków:

st.active == true

st.should_rotate == true (wynika z harmonogramu gatunku, że dany dzień wymaga obrotu i nie ma lockdownu)

st.currently_rotating == false

now_millis >= st.next_rotation_ms

Kluczowy problem:
W tray_init() ustawiasz st.next_rotation_ms = (unsigned long long)init_interval_ms; (np. 17 280 000 ms).
Następnie w tray_update_state() porównujesz now_millis (czas inkubacji) z st.next_rotation_ms.
To działa tylko dla pierwszego obrotu, ale po zakończeniu pierwszego obrotu ustawiasz st.next_rotation_ms = now_millis + safe_interval_ms; – to już jest bezwzględny czas w osi inkubacji.

Problem: przy starcie next_rotation_ms to interwał, a nie bezwzględny czas. Dla pierwszego obrotu warunek now_millis >= next_rotation_ms jest spełniony, ale po restarcie (np. po resecie) next_rotation_ms zostaje nadpisane interwałem, a nie bezwzględnym czasem, więc system czeka od nowa cały interwał.
Dodatkowo, jeśli taca jest w stanie manual_move_until_ms > 0 (ręczny ruch), to automatyczne obroty są blokowane.

Rozwiązanie
Popraw inicjalizację next_rotation_ms – ustaw ją na bezwzględny czas od startu inkubacji, czyli na 0 lub na init_interval_ms (ale to i tak będzie 0, bo czas startu to 0).
Lepiej ustawić st.next_rotation_ms = 0; w tray_init(), a w tray_update_state() przy pierwszym uruchomieniu (gdy next_rotation_ms == 0) ustawić ją na now_millis + safe_interval_ms.

Poprawka w tray_controller.cpp – tray_init():

cpp
// zamiast:
st.next_rotation_ms = (unsigned long long)init_interval_ms;
// ustaw:
st.next_rotation_ms = 0; // wymusi natychmiastowe obliczenie w pierwszej pętli
Poprawka w tray_update_state() – przed sprawdzeniem warunku:

cpp
// Jeśli next_rotation_ms == 0, ustaw pierwszy obrót na teraz + interwał
if (st.next_rotation_ms == 0) {
    st.next_rotation_ms = now_millis + (unsigned long long)safe_interval_ms;
}
Dzięki temu pierwszy obrót nastąpi po upływie pełnego interwału od startu, a po każdym kolejnym obrocie next_rotation_ms będzie aktualizowane o interwał od bieżącego czasu.

2. Ostatni obrót nie jest zapisywany w panelu status (wyświetla się "brak")
Przyczyna
W tray_status_json() pole last_rotation_unix jest obliczane na podstawie:

cfg.start_unix (jeśli RTC działa i start ustawiony)

lub incubation_start_unix (globalny start inkubacji)
Jeśli RTC nie działa (rtc_ok == false) lub incubation_start_unix == 0, to last_rotation_unix pozostaje 0, a UI wyświetla "brak".
Dodatkowo, jeśli st.has_last_rotation == false (np. nie było jeszcze żadnego obrotu), też pokazuje "brak".

Rozwiązanie
Zapewnij fallback – jeśli RTC nie działa, użyj czasu systemowego (millis) do wyliczenia przybliżonego czasu obrotu.

Poprawka w tray_status_json():

cpp
if (st.has_last_rotation) {
    unsigned long long unix_rotation = 0;
    if (cfg.start_unix > 0ULL && rtc_ok) {
        unix_rotation = cfg.start_unix + (st.last_rotation_elapsed_ms / 1000ULL);
    } else if (incubation_start_unix > 0ULL) {
        unix_rotation = incubation_start_unix + (st.last_rotation_elapsed_ms / 1000ULL);
    } else {
        // Fallback: użyj bieżącego czasu systemowego
        // Uwaga: to tylko przybliżenie, ale pokaże dzień/godzinę
        unsigned long long now_sec = millis() / 1000;
        // Załóżmy, że start inkubacji był w momencie uruchomienia (lub pobierz z EEPROM)
        // Lepiej byłoby zapisywać start_unix do EEPROM, ale na szybko:
        unix_rotation = now_sec - (get_incubation_elapsed_ms() / 1000) + (st.last_rotation_elapsed_ms / 1000);
    }
    t["last_rotation_unix"] = unix_rotation;
}
Dodatkowo upewnij się, że st.has_last_rotation jest ustawiane na true przy każdym obrocie (już jest).

3. Żyroskop (MPU6050) nie działa – mimo migania diod
Przyczyna
MPU6050 wymaga odpowiednich pinów I2C i rezystorów pull-up (4,7kΩ).

W kodzie piny I2C są ustawione na stałe: Wire.begin(4,5); w setup().

Jeśli podłączyłeś MPU6050 do innych pinów, nie zostanie wykryty.

Dodatkowo, w api_tray_angle() sprawdzane jest configMgr.cfg.trays[tray].has_angle_sensor – jeśli to false, to nawet wykryty czujnik nie będzie używany. W domyślnej konfiguracji has_angle_sensor dla tacy 0 jest true, dla tacy 1 false.

Rozwiązanie
Sprawdź, czy I2C działa – w setup() jest skaner I2C, który wypisuje znalezione adresy. Sprawdź w monitorze szeregowym, czy pojawia się adres 0x68 lub 0x69.

Jeśli nie ma – zmień piny I2C na inne, np. GPIO4 (SDA) i GPIO5 (SCL) – na Twojej podstawce to są faktyczne linie I2C z RTC/EEPROM/BMP180.

Zmień w setup(): Wire.begin(4, 5);

Aby zmiana była trwała, możesz dodać konfigurację pinów I2C do config.json (np. i2c_sda_pin, i2c_scl_pin) i odczytywać z configu.

Upewnij się, że has_angle_sensor jest true dla tacy, którą testujesz. W config.json dla tacy 0 masz "has_angle_sensor": true – to jest OK.

Pull-up – MPU6050 wymaga rezystorów 4,7kΩ na liniach SDA i SCL do 3,3V. Jeśli ich brak, dodaj.

Zasilanie – MPU6050 pracuje na 3,3V, sprawdź, czy jest zasilany.

Jeśli chcesz użyć innych pinów, możesz dodać je do konfiguracji:

cpp
// w config_manager.h dodać pola:
int i2c_sda_pin;
int i2c_scl_pin;
// w default_config() ustawić: cfg.i2c_sda_pin = 21; cfg.i2c_scl_pin = 22;
// w setup() użyć: Wire.begin(configMgr.cfg.i2c_sda_pin, configMgr.cfg.i2c_scl_pin);
4. Dwie identyczne tace, takie samo PWM, ale jedna chodzi szybciej
Przyczyna
Różnica w prędkości może wynikać z odwróconej logiki sygnału (signal_active_low).

Jeśli dla jednej tacy signal_active_low == true, a dla drugiej false, to wartość PWM jest odwrócona (np. 102 daje 102 dla false, ale dla true daje 255-102 = 153), co daje inną moc.

Sprawdź w config.json dla każdej tacy pole "signal_active_low". Domyślnie dla obu jest false. Jeśli przypadkiem masz różne wartości, to wyjaśnia różnicę.

Rozwiązanie
Ustaw signal_active_low na taką samą wartość dla obu tac (najlepiej false, chyba że Twój driver silnika wymaga aktywnego stanu LOW).

Sprawdź też, czy obie tace mają takie same parametry mechaniczne (przełożenie, napięcie).

Możesz też dodać kalibrację – w panelu kalibracji możesz ręcznie ustawić PWM i zmierzyć czas obrotu.

Weryfikacja – w tray_controller.cpp funkcja tray_pwm_duty_from_speed:

cpp
return tray_signal_active_low(st) ? (uint8_t)(255 - speed) : (uint8_t)speed;
Jeśli signal_active_low różni się, to dla tej samej prędkości duty będzie odwrócone.

Podsumowanie poprawek
Problem	Przyczyna	Rozwiązanie
Auto-obroty nie działają	next_rotation_ms inicjalizowane interwałem, a nie bezwzględnym czasem; brak synchronizacji	Ustaw st.next_rotation_ms = 0; w tray_init() i zainicjuj w tray_update_state() jako now_millis + interval
Ostatni obrót nie wyświetlany	Brak RTC lub incubation_start_unix; last_rotation_unix = 0	Dodaj fallback do czasu systemowego w tray_status_json()
MPU6050 nie działa	Złe piny I2C, brak pull-up, has_angle_sensor false	Sprawdź skaner I2C; zmień piny na 21/22; dodaj pull-up; upewnij się, że has_angle_sensor = true
Różna prędkość tac	Odwrócona logika signal_active_low	Ustaw jednakowe signal_active_low dla obu tac (np. false)
Jeśli potrzebujesz konkretnych fragmentów kodu do wklejenia (np. całą funkcję tray_update_state po poprawkach), daj znać – przygotuję gotowe poprawki.

