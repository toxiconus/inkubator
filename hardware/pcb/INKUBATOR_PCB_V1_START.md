# INKUBATOR PCB V1 - Start projektu

Cel: wersja testowa plytki z ESP32-S3, zasilaniem zewnetrznym i rozdzieleniem sekcji LOGIC oraz POWER.

## 1) Architektura zasilania

Wejscie:
- J1 DC_IN 12V (zlacze srubowe 2-pin)
- F1 bezpiecznik 3A
- D1 ochrona odwrotnej polaryzacji (Schottky lub ideal diode MOSFET)

Rozdzial galezi:
- GALAZ POWER_12V: obciazenia wykonawcze
- GALAZ LOGIC: przetwornice

Przetwornice (na tym etapie jako moduly gotowe):
- U5 BUCK_12V_TO_5V (modul)
- U6 BUCK_5V_TO_3V3 (modul)

Szyny:
- NET_12V
- NET_5V
- NET_3V3
- GND

## 2) Bloki funkcjonalne

A. MCU
- U1 ESP32-S3-WROOM (modul na goldpinach)
- Zasilanie U1 z NET_3V3
- Testpointy: TP_3V3, TP_5V, TP_12V, TP_GND
- Programowanie: UART_TX, UART_RX, EN, BOOT

B. Czujniki
- J2 DS18B20_BUS (3-pin: GND, DATA, 3V3)
- R1 4k7 pull-up DATA do 3V3
- J3 DHT11 (3-pin: GND, DATA, 3V3)
- J4 I2C (4-pin: 3V3, GND, SDA, SCL)

C. Wyjscia mocy
- Q1 MOSFET HEATER_CH1
- Q2 MOSFET HEATER_CH2
- Q3 MOSFET FAN
- Q4 MOSFET DIFFUSER
- J5/J6/J7/J8 zlacza obciazen
- D2-D5 diody zabezpieczajace dla obciazen indukcyjnych

## 3) Wstepna mapa GPIO (zgodna z firmware)

- DS18B20_DATA -> GPIO6
- DHT11_DATA -> GPIO8
- I2C_SDA -> GPIO4
- I2C_SCL -> GPIO5
- FAN_CTRL -> GPIO7
- HEATER_CH1 -> GPIO1
- HEATER_CH2 -> GPIO2
- TRAY_1 -> GPIO16
- TRAY_2 -> GPIO17
- TRAY_3 -> GPIO18
- DIFFUSER_1 -> GPIO21

## 4) Rozmieszczenie blokow na PCB (top view)

- Lewy gorny rog: wejscie zasilania i zabezpieczenia
- Lewa strona: przetwornice
- Srodek: ESP32-S3 i logika
- Prawa strona: zlacza sensorow
- Dol i prawa-dol: MOSFETy i zlacza obciazen

## 5) Reguly prowadzenia

- Pelna masa polygon na obu warstwach
- Sekcja POWER oddzielona od MCU i sensorow
- Szerokie sciezki dla 12V i obciazen
- DATA od DS18B20 prowadzic z dala od MOSFETow
- Jeden punkt laczenia return dla power i logic blisko wejscia zasilania

## 6) Kolejnosc rysowania w KiCad

1. Utworz arkusz zasilania i zabezpieczen
2. Dodaj ESP32-S3 i interfejs programowania
3. Dodaj zlacza sensorow i rezystor pull-up DS
4. Dodaj bloki MOSFET i zlacza obciazen
5. Podpisz nety zgodnie z mapa
6. ERC i poprawki
7. Przypisanie footprintow
8. Przejscie do PCB i placement

## 7) Wersja V1 - co celowo zostaje proste

- Przetwornice jako gotowe moduly, nie integrowane na PCB
- Brak zaawansowanego pomiaru pradu
- Priorytet: stabilnosc i latwy serwis
