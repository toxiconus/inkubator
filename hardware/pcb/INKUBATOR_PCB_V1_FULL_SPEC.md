# INKUBATOR PCB V1 - Pelna Specyfikacja (Start Od Zera)

Ten dokument jest przygotowany tak, zeby dac Ci kompletny, praktyczny opis pierwszej plytki.
Priorytet V1: stabilnosc, prostota, latwy serwis.

## 1. Zalozenia V1

1. Glowny zasilacz zewnetrzny: 12V DC.
2. Sekcja mocy oddzielona od sekcji logiki.
3. ESP32-S3 jako modul na zlaczu (nie lutowany na stale).
4. Przetwornice 12V->5V i 5V->3V3 jako gotowe moduly (na V1).
5. DS18B20 i DHT11 na osobnych złączach, plus I2C dla RTC/BMP.
6. Wyjscia mocy przez MOSFET low-side.

## 2. Architektura zasilania

## 2.1 Wejscie zasilania

- J1: Terminal block 2-pin, raster 5.08 mm
  - Pin 1: +12V_IN
  - Pin 2: GND_IN

- F1: Bezpiecznik na wejsciu
  - Typ: polimerowy resetowalny (PPTC) albo topikowy
  - Wartosc startowa: 3A

- D1: Ochrona przed odwrotna polaryzacja
  - Opcja A: dioda Schottky duzej mocy (prosta)
  - Opcja B: ideal diode na P-MOSFET (lepsza, mniejsze straty)

- C1: Kondensator elektrolityczny na wejsciu 12V
  - 470 uF / 25V (minimum)
- C2: Kondensator ceramiczny przy wejsciu
  - 100 nF / 50V

## 2.2 Rozdzial galezi

Po zabezpieczeniu wejscia zrob rozdzial na:

1. POWER_12V (obciazenia mocy)
2. LOGIC_12V (zasilanie przetwornic)

## 2.3 Przetwornice (V1 jako moduly)

- U5: Modul BUCK 12V->5V
  - Wyjscie: NET_5V
  - Docelowy prad: min 2A

- U6: Modul BUCK 5V->3V3
  - Wyjscie: NET_3V3
  - Docelowy prad: min 1A

## 2.4 Kondensatory sekcji logiki

- C5: 220 uF na linii 5V
- C6: 100 nF na linii 5V
- C7: 220 uF na linii 3V3
- C8: 100 nF na linii 3V3

Dodatkowo blisko ESP:
- C9: 10 uF
- C10: 100 nF

## 3. ESP32-S3 (centrum ukladu)

## 3.1 Montaz

- U1: ESP32-S3-DevKit (modul) na złączach goldpin.
- Zalecenie V1: dwa dlugie złącza zenskie 2.54 mm, aby modul byl wymienny.

## 3.2 Piny sygnalowe (zgodne z firmware)

- GPIO6: DS18B20_DATA
- GPIO8: DHT11_DATA
- GPIO4: I2C_SDA
- GPIO5: I2C_SCL
- GPIO7: FAN_CTRL
- GPIO1: HEATER_CH1_CTRL
- GPIO2: HEATER_CH2_CTRL
- GPIO16: TRAY1_CTRL
- GPIO17: TRAY2_CTRL
- GPIO18: TRAY3_CTRL
- GPIO21: DIFFUSER1_CTRL

## 3.3 Programowanie i serwis

- J10: Header serwisowy 1x6 lub 1x4
  - UART_TX
  - UART_RX
  - EN
  - BOOT
  - 3V3
  - GND

- Testpointy:
  - TP1: 12V
  - TP2: 5V
  - TP3: 3V3
  - TP4: GND
  - TP5: DS_DATA (GPIO6)

## 4. Wejscia czujnikow

## 4.1 DS18B20 (OneWire)

- J2: Zlacze 3-pin (raster 2.54 albo 3.5 mm)
  - Pin 1: GND
  - Pin 2: NET_DS_DATA
  - Pin 3: 3V3

- R1: 4.7k (pull-up)
  - Miedzy NET_DS_DATA i 3V3
  - Umiesc blisko ESP (lub blisko wejscia J2)

Uwagi:
1. Dwa DS18B20 moga byc na tej samej linii NET_DS_DATA.
2. Dla dlugich przewodow rozwaz 2.2k-3.3k jako test, ale startuj od 4.7k.

## 4.2 DHT11

- J3: Zlacze 3-pin
  - Pin 1: GND
  - Pin 2: NET_DHT_DATA
  - Pin 3: 3V3

Uwagi:
1. Jesli modul DHT11 nie ma pull-up na DATA, dodaj R2=10k do 3V3.

## 4.3 I2C (RTC DS3231, BMP)

- J4: Zlacze 4-pin
  - Pin 1: 3V3
  - Pin 2: GND
  - Pin 3: NET_I2C_SDA
  - Pin 4: NET_I2C_SCL

- Pull-up I2C:
  - R3: 4.7k SDA -> 3V3
  - R4: 4.7k SCL -> 3V3

Wazne:
- Jesli moduly I2C juz maja pull-up, nie dubluj ich wiele razy.

## 5. Wyjscia mocy

Zalecana topologia: N-MOSFET low-side dla kazdego kanalu.

## 5.1 Kanaly

- CH1: HEATER_1
- CH2: HEATER_2
- CH3: FAN
- CH4: DIFFUSER

## 5.2 Elementy na kanal (x4)

Dla kazdego kanalu (przyklad CHx):

- Qx: N-MOSFET logic level
  - Vds min 30V
  - Niski Rds(on) przy Vgs=3.3V

- Rg_x: 100R (szeregowo w bramce)
- Rpd_x: 100k (bramka do GND, pull-down)

- Dfly_x: dioda flyback (dla obciazen indukcyjnych)
  - Przy wentylatorze i silniku: konieczna
  - Przy grzalce rezystancyjnej: opcjonalna

- Jx_OUT: złącze wyjsciowe 2-pin
  - Pin 1: +12V
  - Pin 2: LOAD_RETURN (do drenu MOSFET)

## 5.3 Sterowanie z ESP

- GPIO -> Rg_x -> Gate MOSFET
- Source MOSFET -> GND
- Drain MOSFET -> minus obciazenia
- Plus obciazenia -> +12V

## 6. Zlacza i mapowanie I/O (proponowane)

- J1: 12V IN (2-pin, 5.08 mm)
- J2: DS18B20 (3-pin)
- J3: DHT11 (3-pin)
- J4: I2C (4-pin)
- J5: HEATER1 OUT (2-pin)
- J6: HEATER2 OUT (2-pin)
- J7: FAN OUT (2-pin)
- J8: DIFFUSER OUT (2-pin)
- J9: TRAY CTRL/OUT (wg potrzeb mechaniki)
- J10: UART/BOOT/EN debug

## 7. Proponowane wartosci elementow (V1)

Zasilanie:
- F1: 3A
- C1: 470uF/25V
- C2: 100nF
- C5: 220uF/10V
- C6: 100nF
- C7: 220uF/6.3V
- C8: 100nF
- C9: 10uF
- C10: 100nF

Sensory:
- R1: 4.7k (DS pull-up)
- R2: 10k (DHT optional)
- R3: 4.7k (I2C SDA)
- R4: 4.7k (I2C SCL)

MOSFET per kanal:
- Rg: 100R
- Rpd: 100k

## 8. Reguly PCB (bardzo praktyczne)

1. 2 warstwy wystarcza na V1.
2. Grubsze sciezki dla 12V i obciazen:
   - minimum 1.0 mm dla srednich pradow
   - wiecej dla grzalek o wyzszych pradach
3. Sygnaly sensorow cienkie, ale z daleka od sekcji mocy.
4. Pelna masa polygon na obu warstwach.
5. Oddziel fizycznie strefe POWER od LOGIC.
6. Daj otwory montazowe w rogach PCB.
7. Zachowaj margines od krawedzi plytki min 3 mm.

## 9. ERC/DRC - checklista

Schemat (ERC):
1. Brak niepodlaczonych pinow krytycznych.
2. Zasilanie ma PWR_FLAG tam gdzie potrzeba.
3. Nazwy netow sygnalowych zgodne z firmware.

PCB (DRC):
1. Brak zwarc.
2. Brak zbyt malych odstepow.
3. Wszystkie footprinty przypisane.
4. Szerokosci sciezek mocy ustawione swiadomie.

## 10. Kolejnosc pracy (bardzo prosta)

1. Zrob schemat zasilania i ESP.
2. Dodaj złącza sensorow i pull-up.
3. Dodaj 4 kanaly MOSFET.
4. ERC i poprawki.
5. Przypisz footprinty.
6. Przenies do PCB.
7. Uloz elementy blokami.
8. Poprowadz najpierw 12V/GND, potem sygnaly.
9. DRC i poprawki.
10. Wygeneruj Gerbery.

## 11. Czego celowo nie komplikujemy w V1

1. Brak integracji buck od zera (gotowe moduly).
2. Brak pomiaru pradu na kazdym kanale.
3. Brak izolacji galwanicznej sygnalow.

To swiadomy wybor, zeby szybciej uruchomic stabilny prototyp.

## 12. Uwaga o bezpieczenstwie

1. Zanim podlaczysz grzalki, uruchom testy tylko z logika i sensorami.
2. Najpierw testuj wyjscia mocy na sztucznym obciazeniu (np. zarowka 12V).
3. Sprawdz temperature MOSFETow przy dluzszej pracy.
