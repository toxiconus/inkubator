# Schemat okablowania - wersja wstępna

> Plik uzupełniany wraz z postępem projektu.
> Docelowo: schemat KiCad w hardware/schematics/

---

## Blok zasilania

```
AC 230V
  L ──[Bezpiecznik 10A]──┐
  N ─────────────────────┤
                    [Mean Well LRS-150-12]
                         │
                        12V (+) ── Szyna 12V ─┬─────────────────────────────┐
                        GND (-)  ── Szyna GND ─┤                             │
                                               │                        [MOSFET CH1+CH2]
                                          [LM2596-V3]                        │
                                          IN: 12V                     [Grzałki 12V x4]
                                          OUT: 5.0V (nastawić!)              │
                                               │                            GND
                                           5V ─┤
                                              [ESP32-S3 5V pin]
                                               │
                                           3.3V─┤
                                              [Sensory / OLED / RTC]
```

---

## Moduł MOSFET (2-kanałowy 15A PWM)

```
Moduł MOSFET:
  VCC  ── 12V (szyna)
  GND  ── GND (wspólna)
  IN1  ── GPIO16 (ESP32-S3) [PWM CH1]
  IN2  ── GPIO17 (ESP32-S3) [PWM CH2]
  OUT1+── Grzałka 1+2 (+)
  OUT1-── GND
  OUT2+── Grzałka 3+4 (+)
  OUT2-── GND

Grzałki:
  Grzałka 1: (+)──OUT1  (-)──GND
  Grzałka 2: (+)──OUT1  (-)──GND  [równolegle z grzałką 1]
  Grzałka 3: (+)──OUT2  (-)──GND
  Grzałka 4: (+)──OUT2  (-)──GND  [równolegle z grzałką 3]
```

**UWAGA:** 2 grzałki równolegle na kanał = 4A/kanał @ 12V (48W/kanał).
Moduł MOSFET 15A - rezerwa wystarczająca.

---

## Piny ESP32-S3 (wstępnie)

```
GPIO  | Funkcja          | Do
------|------------------|------------------------
5V    | Zasilanie        | LM2596 OUT (+)
GND   | Masa             | Masa wspólna
GPIO4 | SDA (I2C)        | OLED + SHT31/RTC
GPIO5 | SCL (I2C)        | OLED + SHT31/RTC
GPIO8 | OneWire          | DS18B20 (opcjonalnie)
GPIO16| PWM1 (LEDC)      | IN1 MOSFET - grzałki 1+2
GPIO17| PWM2 (LEDC)      | IN2 MOSFET - grzałki 3+4
GPIO18| PWM3 / Relay     | Wentylator / nawilżacz
GPIO21| Status LED       | LED (przez 330Ω do GND)
GPIO0 | BOOT (wbudowany) | Nie używać jako wyjście
EN    | RESET            | Przycisk reset (opcjonalnie)
```

---

## LM2596-V3 - nastawy

1. **Przed podłączeniem ESP32:** zmierzyć napięcie wyjściowe multimetrem.
2. Potencjometr kręcić aż do uzyskania **5.0V ± 0.05V** na wyjściu.
3. Dopiero potem podłączyć ESP32-S3.
4. Można też użyć wyjścia 3.3V jeśli ESP32 ma pin 3.3V IN.

---

## Kolejność uruchomienia

1. Podłączyć tylko LM2596 do 12V, zmierzyć wyjście → ustawić 5V
2. Podłączyć ESP32 do LM2596, sprawdzić zasilanie
3. Wgrać minimalny test (Blink), sprawdzić działanie
4. Podłączyć MOSFET-y (BEZ grzałek), przetestować PWM
5. Podłączyć grzałki, przetestować moc grzania
6. Podłączyć sensory temperatury
7. Uruchomić pętlę PID/termostatową
