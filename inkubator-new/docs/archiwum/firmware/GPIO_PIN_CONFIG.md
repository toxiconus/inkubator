# Konfiguracja pinów GPIO - ESP32-S3 N8R2

## Aktualny stan podlaczen (2026-06-12)

- Grzalka 1 (CH0): GPIO1, 6m, 12V, 48W (200% mocy pojedynczej grzalki 24W)
- Grzalka 2 (CH1): GPIO2, 3m, 12V, 24W (100% mocy odniesienia)
- Grzalka 3: 3m, 12V, 24W (na razie odlaczona / OFF)
- Grzalka 4: na razie odlaczona (`heater_pin_4=-1`)
- Taca 1: GPIO16 (aktywna)
- Taca 2: GPIO17 (domyslnie OFF)
- Taca 3: GPIO18 (rezerwa OFF)
- Dyfuzor 1: GPIO11 (aktywny)
- Dyfuzor 2: GPIO12 (aktywny)
- Nie uzywaj GPIO19/GPIO20 do sterowania (linie USB D-/D+)

## Pin Mapping dla firmware'u

```cpp
// ============================================================================
// ZASILANIE I STEROWANIE MOCĄ
// ============================================================================

// Wentylator 12V (zawsze włączany z systemem)
#define PIN_FAN_RELAY      5        // GPIO5  → Relay Module (12V fan control)

// Tace obracające (PWM - mogą być wyłączone całkowicie)
#define PIN_TRAY_PWM      18        // GPIO18 → PWM do Relay / MOSFET (tace rotary)

// ============================================================================
// GRZAŁKI (Sterowanie przez MOSFET Module - PWM)
// ============================================================================

#define PIN_HEATER_1       1        // GPIO1  → MOSFET IN1 (Grzałka grupa 1)
#define PIN_HEATER_2       2        // GPIO2  → MOSFET IN2 (Grzałka grupa 2)
#define PIN_HEATER_3       3        // GPIO3  → MOSFET IN3 (Grzałka grupa 3 - opcja)
#define PIN_HEATER_4       4        // GPIO4  → MOSFET IN4 (Grzałka grupa 4 - opcja)

// Kanały PWM
#define PWM_HEATER_FREQ    5000     // 5 kHz
#define PWM_HEATER_RES     8        // 8-bit (0-255)

// ============================================================================
// CZUJNIKI TEMPERATURY (OneWire - DS18B20)
// ============================================================================

#define PIN_TEMP_SENSOR_1  6        // GPIO6  → DS18B20 Data (DQ)
#define PIN_TEMP_SENSOR_2  7        // GPIO7  → DS18B20 Data (DQ) - opcja

// OneWire biblioteka
#define ONEWIRE_PIN_1      6
#define ONEWIRE_PIN_2      7        // Jeśli używasz 2 czujników na osobnych pinach

// ============================================================================
// CZUJNIK WILGOCI (DHT22 - DHT Protocol)
// ============================================================================

#define PIN_DHT22          8        // GPIO8  → DHT22 Data pin

// ============================================================================
// CZUJNIK POZIOMU PŁYNU (ADC)
// ============================================================================

#define PIN_WATER_LEVEL    9        // GPIO9  → ADC input (poziom wody)

// ADC Calibration (dostosować do rzeczywistych wartości)
#define WATER_LEVEL_EMPTY  200      // ADC value when empty
#define WATER_LEVEL_FULL   800      // ADC value when full

// ============================================================================
// CZUJNIK DRZWI (Digital Input)
// ============================================================================

#define PIN_DOOR_SENSOR   10        // GPIO10 → Przełącznik drzwi (pull-up wewnętrzny)

// ============================================================================
// NAWILRZACZE ULTRASONICZNE (5V - 2 szt)
// ============================================================================

#define PIN_HUMIDIFIER_1  11        // GPIO11 → Nawilrzacz #1 (PWM/ON-OFF)
#define PIN_HUMIDIFIER_2  12        // GPIO12 → Nawilrzacz #2 (PWM/ON-OFF)

// Kanały PWM
#define PWM_HUMIDIFIER_FREQ  1000   // 1 kHz
#define PWM_HUMIDIFIER_RES   8      // 8-bit (0-255)

// ============================================================================
// REZERWOWE PINY (do przyszłego rozszerzenia)
// ============================================================================

// GPIO dostępne: 13, 14, 15, 16, 17, 19, 20, 21
// I2C: GPIO3 (SDA), GPIO4 (SCL) - opcjonalnie dla wyświetlacza

// ============================================================================
// PINY ZAREZERWOWANE (System)
// ============================================================================

// Zasilanie
// PIN_VDD: 3.3V (wewnętrzny regulator)
// PIN_5V: +5V z LM2596
// PIN_GND: GND (Pins 2, 3, 14, 15, 16, 17 - minimum 2)

// USB
// PIN_USB_DN: GPIO 19 (USB D-)
// PIN_USB_DP: GPIO 20 (USB D+)

// JTAG / SWD (debugowanie)
// PIN_TCO: GPIO 39
// PIN_TMS: GPIO 40
// PIN_TDI: GPIO 41
// PIN_TDO: GPIO 42

// ============================================================================
// PRZYKŁADOWA INICJALIZACJA W SETUP()
// ============================================================================

void setup_gpio_pins() {
    // Grzałki (PWM output)
    pinMode(PIN_HEATER_1, OUTPUT);
    pinMode(PIN_HEATER_2, OUTPUT);
    pinMode(PIN_HEATER_3, OUTPUT);
    pinMode(PIN_HEATER_4, OUTPUT);
    
    // Wentylator i tace
    pinMode(PIN_FAN_RELAY, OUTPUT);
    pinMode(PIN_TRAY_PWM, OUTPUT);
    
    // Nawilrzacze
    pinMode(PIN_HUMIDIFIER_1, OUTPUT);
    pinMode(PIN_HUMIDIFIER_2, OUTPUT);
    
    // Czujniki (Input)
    pinMode(PIN_DOOR_SENSOR, INPUT_PULLUP);  // Pull-up wewnętrzny
    
    // Inicjalizacja PWM
    // ledcSetup(channel, frequency, resolution);
    ledcSetup(0, PWM_HEATER_FREQ, PWM_HEATER_RES);      // Heater channel
    ledcSetup(1, PWM_HUMIDIFIER_FREQ, PWM_HUMIDIFIER_RES); // Humidifier channel
    
    // Przypisanie PWM do pinów
    ledcAttachPin(PIN_HEATER_1, 0);
    ledcAttachPin(PIN_HEATER_2, 0);
    ledcAttachPin(PIN_HUMIDIFIER_1, 1);
    ledcAttachPin(PIN_HUMIDIFIER_2, 1);
}

// ============================================================================
// PRZYKŁADY UŻYCIA
// ============================================================================

void example_control_heaters() {
    // Włącz grzałkę 1 na 50%
    ledcWrite(0, 128);  // 128 = ~50% (0-255)
    
    // Włącz grzałkę 2 na 100%
    ledcWrite(0, 255);
    
    // Wyłącz wszystkie grzałki
    ledcWrite(0, 0);
}

void example_control_humidifiers() {
    // Włącz nawilrzacz 1 na 100%
    ledcWrite(1, 255);
    
    // Nawilrzacz 2 na 75%
    ledcWrite(1, 192);
}

void example_read_sensors() {
    // Czujnik drzwi
    if (digitalRead(PIN_DOOR_SENSOR) == LOW) {
        Serial.println("Drzwi OTWARTE");
    }
    
    // Czujnik poziomu wody (ADC)
    int water_level = analogRead(PIN_WATER_LEVEL);
    Serial.print("Poziom wody ADC: ");
    Serial.println(water_level);
}

// ============================================================================
```

## Tabela szybkiej referencji

| Funkcja | Pin GPIO | Typ | Zasilanie | Uwagi |
|---------|----------|-----|-----------|-------|
| **Sterowanie** | | | | |
| Wentylator 12V (Relay) | 5 | Digital Out | 5V | HIGH = ON |
| Tace obracające (PWM) | 18 | PWM Out | 5V (logic) | 0-255 |
| | | | | |
| **Grzałki PWM** | | | | |
| Grzałka 1 | 1 | PWM Out | 5V (logic) | 0-255 |
| Grzałka 2 | 2 | PWM Out | 5V (logic) | 0-255 |
| Grzałka 3 | 3 | PWM Out | 5V (logic) | 0-255 (opcja) |
| Grzałka 4 | 4 | PWM Out | 5V (logic) | 0-255 (opcja) |
| | | | | |
| **Temperatura** | | | | |
| DS18B20 #1 (OneWire) | 6 | OneWire | 5V | Pull-up 4.7kΩ |
| DS18B20 #2 (OneWire) | 7 | OneWire | 5V | Pull-up 4.7kΩ (opcja) |
| | | | | |
| **Wilgotność** | | | | |
| DHT22 (Data) | 8 | DHT In | 5V | Pull-up 10kΩ |
| | | | | |
| **Poziom płynu** | | | | |
| Water Level Sensor | 9 | ADC In | 5V | Analogowy (0-4095) |
| | | | | |
| **Przełączniki** | | | | |
| Czujnik drzwi | 10 | Digital In | 5V | Pull-up wewnętrzny |
| | | | | |
| **Nawilrzacze** | | | | |
| Nawilrzacz 1 | 11 | PWM Out | 5V (logic) | 0-255 |
| Nawilrzacz 2 | 12 | PWM Out | 5V (logic) | 0-255 (opcja) |

---

**Wersja**: 1.0  
**Data**: 2026-05-16  
**Kompatybilność**: ESP32-S3 N8R2
