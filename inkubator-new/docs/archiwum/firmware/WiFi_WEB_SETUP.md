# 📱 WiFi + Web Dashboard - Poradnik Setup

## ✨ Co Nowego?

Twój inkubator ma teraz **3 wielkie ullepszenia**:

```
✅ EEPROM Storage    - Zapamiętuje dzień nawet po wyłączeniu zasilania!
✅ WiFi Connectivity - Łączy się z Twoją domową siecią WiFi
✅ Web Dashboard     - Wyświetl dane na telefonie/komputerze w przeglądarce
```

---

## 🔧 KONFIGURACJA - 3 Kroki

### Krok 1: Ustaw SSID i Hasło WiFi

W **main.cpp** (linia ~17-18), zmień:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"           // 👈 ZMIEŃ NA TWÓJ SSID
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"   // 👈 ZMIEŃ NA TWOJE HASŁO
```

Na **Twoje rzeczywiste dane**:

```cpp
#define WIFI_SSID "MojaSeć"              // Nazwa sieci (SSID z routera)
#define WIFI_PASSWORD "Moje1234Hasło"     // Hasło WiFi
```

**Gdzie znaleźć SSID/Hasło?**
- SSID = nazwa sieci WiFi (widać w ustawieniach WiFi)
- Hasło = hasło dostępu do WiFi (na dnie/obudowie routera ALBO w ustawieniach)

### Krok 2: Upload do ESP32

```bash
Ctrl+U  # Upload firmware do ESP32
```

### Krok 3: Sprawdź Serial Monitor

Po starcie zobaczysz:

```
📶 Łączenie z WiFi...
.......................
✅ WiFi Connected!
   SSID: MojaSeć
   IP Address: http://192.168.1.123  ← 👀 ЗАПИШЬ TEN ADRES!
   Sygnał: -45 dBm

✅ WebServer started na porcie 80
   Otwórz: http://192.168.1.123 na telefonie/komputerze
```

---

## 📊 Jak Używać Web Dashboard?

### Na Telefonie:

1. Upewnij się, że **jesteś podłączony do tej samej sieci WiFi** co ESP32 (MojaSeć)
2. Otwórz przeglądarkę
3. Wpisze w pasku adresu: `http://192.168.1.123` (lub Twój IP adres)
4. **ENTER** - Dashboard się załaduje! 🎉

### Co Zobaczysz?

```
═══════════════════════════════════════════════════════════════
           🐔 Inkubator Monitor
           Kura | Dzień 5/21
═══════════════════════════════════════════════════════════════

🥚 Temperatura jaj:          37.5°C  ✅
🐠 Temperatura akwarium:      36.0°C  ✅
💧 Wilgotność:               62%
🔥 Grzałka PWM:              35%

┌─────────────────────────────────────────────┐
│ Cel temperatury: 37.5°C                     │
│ Dzień inkubacji: 5/21                       │
└─────────────────────────────────────────────┘

🎯 Docelowa wilgotność:       55%
🛑 Lockdown:                  ✗ Nie
🚪 Drzwi:                     ✓ Zamknięte

♻️ Odświeża co 5 sekund | Ostatnia aktualizacja: 14:32:45
```

---

## 💾 EEPROM Storage - Zapamiętywanie Dnia

### Jak Działa?

```
START #1 (Dzień 1)
├─ ESP32 uruchamia się
├─ EEPROM jest pusty → NOWY START
├─ system_start_time = teraz (millis)
├─ Co 10 minut: ZAPISZ system_start_time do EEPROM
└─ 💾 Dzień 1 jest zapisany

WYŁĄCZENIE ZASILANIA (po 5 dniach)
├─ Zasilanie OFF
├─ system_start_time utracony (pamięć RAM wyczyszczona)
└─ Dane w EEPROM żyją! 💾

WŁĄCZENIE (Dzień 6)
├─ ESP32 uruchamia się
├─ EEPROM ma stary system_start_time (od Dnia 1!)
├─ Odczytaj: "Aha! Minęło 5 dni od Dnia 1!"
├─ Przywróć: system_start_time = stara wartość + offline czas
└─ ✅ Pokazuje Dzień 6 (ciągłość!)
```

### Konsola Startowa

Jeśli system był już uruchomiony:

```
✅ EEPROM: Przywrócono start_time (offline: 3600000 ms = 1.0 godz)
```

Nowy start:

```
⭐ EEPROM: Nowy start inkubacji!
```

---

## 🔗 REST API - Dla Programistów

Jeśli chcesz integrować z własną aplikacją, jest dostępny endpoint JSON:

### GET `/api/status`

```bash
curl http://192.168.1.123/api/status
```

Odpowiedź:

```json
{
  "bird_name": "🐔 CHICKEN",
  "incubation_days": 21,
  "current_day": 5,
  "temp_inside": 37.5,
  "temp_aquarium": 36.0,
  "humidity": 62.5,
  "target_temp": 37.5,
  "target_humidity": 55.0,
  "heater_pwm": 35,
  "water_level": 512,
  "door_open": false,
  "aquarium_temp_ok": true,
  "lockdown_day": 19,
  "timestamp": 432000000
}
```

---

## 🆘 TROUBLESHOOTING

### Problem: "WiFi Connected" nie pojawia się

**Przyczyna**: Złe SSID/hasło

**Rozwiązanie**:
1. Sprawdź SSID (może mieć znaki specjalne - typ spacji!)
2. Sprawdź hasło (wielkie/małe litery się liczą!)
3. Upewnij się, że router jest włączony
4. Zmień SSID/hasło w main.cpp i upload

### Problem: "Nie mogę się połączyć z http://192.168.1.123"

**Przyczyna**: 
- Nie jesteś na tej samej sieci WiFi co ESP32
- IP adres się zmienił

**Rozwiązanie**:
1. Sprawdź w Serial Monitor jaki jest **aktualny** IP
2. Skopiuj dokładnie ten IP do przeglądarki
3. Jeśli IP się zmienia, ustaw STATYCZNE IP w routerze

### Problem: Dashboard ładuje się ale dane nie aktualizują się

**Przyczyna**: ESP32 nie wysyła danych

**Rozwiązanie**:
1. Sprawdź Serial Monitor - czy dane są odczytywane?
2. Przeładuj stronę (F5)
3. Sprawdź czy WebServer jest Started w konsoli

### Problem: Po wyłączeniu zasilania dzień się zeruje

**Przyczyna**: EEPROM nie został zapisany

**Rozwiązanie**:
- Kod automatycznie ZAPISUJE co 10 minut
- Czekaj ~10 minut zanim wyłączysz zasilanie
- System powinien być włączony co najmniej 10 minut żeby gwarantować zapis

---

## 📈 Zaawansowane Opcje

### Zmiana Portu WebServer

Domyślnie: Port **80** (standard HTTP)

Jeśli chcesz inny port (np. 8080):

```cpp
WebServer webServer(8080);  // Zmień 80 na 8080
```

Wtedy otwórz: `http://192.168.1.123:8080`

### Auto-Refresh Dashboard

Dashboard odświeża co **5 sekund** (linijka w HTML).

Zmiana:
```javascript
setInterval(fetchData, 5000);  // 5000 ms = 5 sekund
// Zmień na 10000 dla 10 sekund
```

### Statyczne IP (zamiast DHCP)

W setup_wifi():

```cpp
void setup_wifi() {
  IPAddress staticIP(192, 168, 1, 100);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(staticIP, gateway, subnet);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // ...
}
```

---

## ✅ Checklist Setup

- [ ] Zmienić WIFI_SSID na moją sieć
- [ ] Zmienić WIFI_PASSWORD na moje hasło
- [ ] Upload do ESP32 (Ctrl+U)
- [ ] Sprawdzić Serial Monitor - IP address pojawił się
- [ ] Otworzyć http://[IP_ADDRESS] na telefonie
- [ ] Dashboard załadował się
- [ ] Dane aktualizują się co 5 sekund
- [ ] Wyłączyć zasilanie na 10 minut
- [ ] Włączyć zasilanie
- [ ] Sprawdzić: dzień powinien być taki sam lub wyższy!

---

## 🎯 Podsumowanie

```
┌─────────────────────────────────────────────────────────┐
│ DWIE GŁÓWNE FUNKCJE:                                    │
├─────────────────────────────────────────────────────────┤
│ 1. EEPROM Storage ← Zapamiętuje dzień                  │
│    └─ Zapisuje co 10 minut                             │
│    └─ Po wyłączeniu - przywraca dzień                  │
│                                                         │
│ 2. WiFi Web Dashboard ← Monitor na telefonie           │
│    └─ Wstęp: http://192.168.1.X                        │
│    └─ Odświeża co 5 sekund                             │
│    └─ Pokazuje: Temp, Wilgotność, Dzień, Lockdown     │
└─────────────────────────────────────────────────────────┘
```

**Teraz Twój inkubator je "INTELIGENTNY"! 🚀**

Możesz monitorować go z dowolnego miejsca w domu na telefonie!
