#ifndef CONFIG_H
#define CONFIG_H

// ========== SENSORY ==========
#define ONE_WIRE_BUS 4              // DS18B20 (OneWire)
#define DHT_PIN 15                  // DHT22 Pin
#define DHT_TYPE DHT22

// ========== PWM / TIMERY ==========
#define PWM_FREQUENCY 5000          // 5kHz dla grzałek
#define PWM_RESOLUTION 8            // 8-bit (0-255)

// PWM WYJŚCIA (Timer 0)
#define HEATER_PIN 2                // Grzałka (4-kanałowa)
#define HEATER_CHANNEL 0
#define FAN1_PIN 19                 // Wentylator 1 (stały)
#define FAN1_CHANNEL 1
#define FAN2_PIN 18                 // Wentylator 2 (cykliczny)
#define FAN2_CHANNEL 2
#define FAN3_PIN 5                  // Wentylator 3 (chłodzący awaryjny)
#define FAN3_CHANNEL 3

// ========== GPIO - RELAY / PRZEŁĄCZNIKI ==========
#define HUMIDIFIER_PIN 12           // Relay nawilżacz soniczny
#define TRAY_PIN 14                 // Relay taca (silnik obracający)
#define LIGHT_PIN 13                // Relay światło (LED do wylęgu)

// Przyciski (future)
#define BUTTON_MENU 35              // Przycisk menu (input only)
#define BUTTON_OK 34                // Przycisk OK (input only)

// ========== PARAMETRY TEMPERATUROWE ==========
#define TARGET_TEMP 37.5            // Docelowa temperatura
#define TEMP_TOLERANCE 0.5          // Tolerancja sterowania
#define TEMP_HYSTERESIS 1.0         // Histereza do unikania oscylacji

// Profil temperaturowy (można zmieniać w zależności od dnia)
#define TEMP_DAY_1_21 37.5          // Dni 1-21 (pierwsze 3 tygodnie)
#define TEMP_DAY_22_HATCH 37.2      // Dni 22-28 (ostatni tydzień - kluje się)

// ========== WILGOTNOŚĆ ==========
#define TARGET_HUMIDITY_EARLY 40    // Wilgotność dni 1-18 (%)
#define TARGET_HUMIDITY_LATE 75     // Wilgotność dni 19+ (%)
#define HUMIDITY_TOLERANCE 5        // Tolerancja wilgotności

// ========== CYKL WIATRAKÓW ==========
#define FAN1_PWM 200                // Wentylator 1 - stały (0-255)
#define FAN2_CYCLE_ON 300           // Wentylator 2 - włączony (ms)
#define FAN2_CYCLE_OFF 200          // Wentylator 2 - wyłączony (ms)
#define FAN3_TEMP_THRESHOLD 38.5    // Wentylator 3 - włącza się powyżej

// ========== TACA ==========
#define TRAY_CYCLE_INTERVAL 3600    // Co ile sekund obrót tacy (1h = 3600s)
#define TRAY_ROTATION_TIME 10       // Jak długo silnik pracuje (sekundy)
#define TRAY_STOP_DAY 21            // Przestaje się obracać od dnia 21

// ========== ŚWIATŁO ==========
#define LIGHT_HATCH_DAY 21          // Włącza się od dnia kiedy zaczyna kluwać
#define LIGHT_PWM 255               // Jasność (0-255)

// ========== OPÓŹNIENIA / INTERWAŁY ==========
#define SENSOR_READ_INTERVAL 2000   // Co 2 sekundy odczyt sensorów (ms)
#define CONTROL_LOOP_INTERVAL 5000  // Co 5 sekund pętla sterowania (ms)
#define LOG_INTERVAL 60000          // Co 1 minutę logging do SPIFFS (ms)

// ========== SPIFFS LOGGING ==========
#define ENABLE_SPIFFS_LOG true
#define MAX_LOG_SIZE 1000           // Max wpisów w bufforze

#endif // CONFIG_H
