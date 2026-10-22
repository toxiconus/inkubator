# Bill of Materials - Inkubator Jaj

Stan: **wstępny / do weryfikacji**
Data: 2026-05-16

---

## Zatwierdzone komponenty

| # | Nazwa | Model | Ilość | Uwagi |
|---|-------|-------|-------|-------|
| 1 | Mikrokontroler ESP32-S3 | ESP32-S3 N8R2 44-pin Type-C | 1 | Możliwa wymiana na N16R8 |
| 2 | Zasilacz impulsowy | Mean Well LRS-150-12 (12V/12.5A) | 1 | Zasilanie główne |
| 3 | Przetwornica DC/DC | LM2596-V3 (step-down, adj.) | 1 | 12V→5V dla ESP32 i logiki |
| 4 | Przewód grzewczy | 12V / 24W / 3m | 4 | Ogrzewanie komory |
| 5 | Moduł MOSFET PWM | 2x MOSFET 15A (na jednej płytce) | 1 (2 kanały) | Sterowanie grzałkami |

## Brakujące komponenty (do wyboru)

| # | Funkcja | Opcja A | Opcja B | Decyzja |
|---|---------|---------|---------|---------|
| 6 | Czujnik temp. | DS18B20 (OneWire, ±0.5°C) | SHT31 (I2C, ±0.3°C + wilgotność) | ? |
| 7 | Czujnik wilgotności | DHT22 (oddzielny, ±2%RH) | SHT31 (combo z temp.) | ? |
| 8 | Wyświetlacz | OLED 0.96" I2C SSD1306 | None (tylko WiFi web UI) | ? |
| 9 | RTC | DS3231 (I2C, ±2ppm) | Wbudowany NTP (WiFi) | ? |
| 10 | Wentylator | 12V 80mm DC fan | 12V 60mm DC fan | ? |
| 11 | Nawilżacz | Ultradźwiękowy 5V (USB) + MOSFET | Miska + grzałka PTC | ? |
| 12 | Silnik tac | Serwo MG996R | Silnik krokowy 28BYJ-48 | ? |
| 13 | Obudowa/komora | Styropian | Sklejka + izolacja | ? |

---

## Parametry elektryczne

| Komponent | Napięcie | Prąd (max) | Moc |
|-----------|----------|------------|-----|
| Grzałka x4 | 12V | 4 × 2A = 8A | 4 × 24W = 96W |
| ESP32-S3 | 5V | ~0.5A | ~2.5W |
| Wentylator | 12V | ~0.3A | ~3.6W |
| Logika/sensory | 3.3V | ~0.2A | ~0.66W |
| **SUMA** | 12V | **~9A** | **~103W** |
| **Zasilacz** | 12V | 12.5A | **150W** | rezerwa ~47W ✓ |
