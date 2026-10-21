#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

// ╔════════════════════════════════════════════════════════════════════╗
// ║                     KONFIGURACJA INKUBATORA                        ║
// ║           Wgrać na ESP32 - jeden plik do symulatora                ║
// ╚════════════════════════════════════════════════════════════════════╝

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

#define TEMP_DAY_1_21 37.5          // Dni 1-21
#define TEMP_DAY_22_HATCH 37.2      // Dni 22-28

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

// ========== SYMULACJA ==========
#define ENABLE_SIMULATION true      // Włącz symulację zamiast czujników
#define SIMULATION_TEMP_RATE 0.02   // Jak szybko zmienia się temp w symulacji
#define SIMULATION_HUMIDITY_RATE 0.5 // Jak szybko zmienia się wilg w symulacji

// ========== ZMIENNE GLOBALNE - SENSORY ==========
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
DHT dht(DHT_PIN, DHT_TYPE);

int deviceCount = 0;
float lastDHTTemp = 25.0;
float lastDHTHumidity = 50.0;
unsigned long lastSensorReadTime = 0;

// ========== ZMIENNE GLOBALNE - KONTROLER ==========
uint8_t heaterPower = 0;
uint8_t fan1Power = 0;
uint8_t fan2Power = 0;
uint8_t fan3Power = 0;

bool humidifierState = false;
bool trayState = false;
bool lightState = false;

int incubationDay = 1;

unsigned long lastTrayRotation = 0;
unsigned long fan2LastToggle = 0;
bool fan2IsOn = false;

// ========== ZMIENNE GLOBALNE - SYSTEM ==========
unsigned long lastSensorRead = 0;
unsigned long lastControlLoop = 0;
unsigned long lastLog = 0;
unsigned long startTime = 0;
int currentDay = 1;

// ========== SYMULACJA CZUJNIKÓW ==========
float simTemp = 25.0;
float simHumidity = 50.0;

// ════════════════════════════════════════════════════════════════════
// ═══════════════════ FUNKCJE SENSORÓW ════════════════════════════════
// ════════════════════════════════════════════════════════════════════

void initSensors() {
  Serial.println("\n========== Inicjalizacja sensorów ==========");
  
  if (ENABLE_SIMULATION) {
    Serial.println("🔷 TRYB SYMULACJI - brak rzeczywistych czujników");
    Serial.println("   Temperatura: 25°C → 37.5°C (symulacja)");
    Serial.println("   Wilgotność: 50% (symulacja)");
    deviceCount = 3;  // Symuluj 3 czujniki
  } else {
    ds18b20.begin();
    deviceCount = ds18b20.getDeviceCount();
    ds18b20.setResolution(12);
    
    Serial.print("DS18B20: Znaleziono ");
    Serial.print(deviceCount);
    Serial.println(" czujników");
    
    dht.begin();
    Serial.println("DHT22: Inicjalizacja OK");
  }
  
  Serial.println("=========================================\n");
}

void updateSensors() {
  unsigned long currentTime = millis();
  
  if (ENABLE_SIMULATION) {
    // ===== SYMULACJA =====
    float targetTemp = (currentDay <= 21) ? TEMP_DAY_1_21 : TEMP_DAY_22_HATCH;
    float targetHumidity = (currentDay <= 18) ? TARGET_HUMIDITY_EARLY : TARGET_HUMIDITY_LATE;
    
    // Temperatura dąży do celu (z szumem)
    if (simTemp < targetTemp - 0.5) {
      simTemp += SIMULATION_TEMP_RATE;
    } else if (simTemp > targetTemp + 0.5) {
      simTemp -= SIMULATION_TEMP_RATE * 0.5;  // Wolniejsze chłodzenie
    }
    
    // Dodaj szum
    simTemp += (random(-10, 10) * 0.001);
    
    // Wilgotność - zmienia się w zależności od nawilżacza
    if (humidifierState) {
      simHumidity += SIMULATION_HUMIDITY_RATE;
    } else {
      simHumidity -= SIMULATION_HUMIDITY_RATE * 0.5;
    }
    
    simHumidity = constrain(simHumidity, 20, 90);
    
    lastDHTTemp = simTemp;
    lastDHTHumidity = simHumidity;
  } else {
    // ===== RZECZYWISTE CZUJNIKI =====
    if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
      ds18b20.requestTemperatures();
      
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      
      if (!isnan(h) && !isnan(t)) {
        lastDHTTemp = t;
        lastDHTHumidity = h;
      }
      
      lastSensorReadTime = currentTime;
    }
  }
}

float getDS18B20Temp(int index) {
  if (index >= deviceCount) return -999.0;
  
  if (ENABLE_SIMULATION) {
    // Symuluj 3 czujniki wokół średniej
    float offset = (index - 1) * 0.2 - 0.2;
    return simTemp + offset + (random(-5, 5) * 0.01);
  } else {
    float temp = ds18b20.getTempCByIndex(index);
    if (temp == DEVICE_DISCONNECTED_C) return -999.0;
    return temp;
  }
}

float getDHTTemp() {
  return lastDHTTemp;
}

float getDHTHumidity() {
  return lastDHTHumidity;
}

int getDS18B20Count() {
  return deviceCount;
}

// ════════════════════════════════════════════════════════════════════
// ═══════════════════ FUNKCJE KONTROLERA ══════════════════════════════
// ════════════════════════════════════════════════════════════════════

void initController() {
  Serial.println("\n========== Inicjalizacja kontrolera ==========");
  
  // Ustaw PWM dla wyjść (Arduino Core 2.0+ API)
  ledcAttach(HEATER_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(FAN1_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(FAN2_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(FAN3_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  
  // GPIO dla relaków
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  pinMode(TRAY_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  
  // Wszystko off na start
  digitalWrite(HUMIDIFIER_PIN, LOW);
  digitalWrite(TRAY_PIN, LOW);
  digitalWrite(LIGHT_PIN, LOW);
  
  setHeaterPower(0);
  setFan1Power(0);
  setFan2Power(0);
  setFan3Power(0);
  
  Serial.println("PWM: OK");
  Serial.println("GPIO: OK");
  Serial.println("=========================================\n");
  
  lastTrayRotation = millis();
}

// ========== SETTERY PWM ==========
void setHeaterPower(uint8_t pwm) {
  heaterPower = pwm;
  ledcWrite(HEATER_PIN, pwm);
}

void setFan1Power(uint8_t pwm) {
  fan1Power = pwm;
  ledcWrite(FAN1_PIN, pwm);
}

void setFan2Power(uint8_t pwm) {
  fan2Power = pwm;
  ledcWrite(FAN2_PIN, pwm);
}

void setFan3Power(uint8_t pwm) {
  fan3Power = pwm;
  ledcWrite(FAN3_PIN, pwm);
}

// ========== SETTERY GPIO RELAY ==========
void setHumidifier(bool state) {
  humidifierState = state;
  digitalWrite(HUMIDIFIER_PIN, state ? HIGH : LOW);
}

void setTray(bool state) {
  trayState = state;
  digitalWrite(TRAY_PIN, state ? HIGH : LOW);
}

void setLight(bool state) {
  lightState = state;
  digitalWrite(LIGHT_PIN, state ? HIGH : LOW);
}

// ========== FUNKCJE LOGIKI STEROWANIA ==========
float getTargetTemp(int day) {
  if (day <= 21) {
    return TEMP_DAY_1_21;
  } else {
    return TEMP_DAY_22_HATCH;
  }
}

float getTargetHumidity(int day) {
  if (day <= 18) {
    return TARGET_HUMIDITY_EARLY;
  } else {
    return TARGET_HUMIDITY_LATE;
  }
}

void updateFans(float currentTemp, int day) {
  // Fan 1 - stały
  setFan1Power(FAN1_PWM);
  
  // Fan 2 - cykliczny
  unsigned long now = millis();
  if (fan2IsOn) {
    if (now - fan2LastToggle > FAN2_CYCLE_ON) {
      setFan2Power(0);
      fan2IsOn = false;
      fan2LastToggle = now;
    }
  } else {
    if (now - fan2LastToggle > FAN2_CYCLE_OFF) {
      setFan2Power(255);
      fan2IsOn = true;
      fan2LastToggle = now;
    }
  }
  
  // Fan 3 - awaryjne chłodzenie
  if (currentTemp > FAN3_TEMP_THRESHOLD) {
    setFan3Power(255);
  } else {
    setFan3Power(0);
  }
}

void updateTray(int day) {
  if (day >= TRAY_STOP_DAY) {
    setTray(false);
    return;
  }
  
  unsigned long now = millis();
  if (trayState) {
    if (now - lastTrayRotation > (TRAY_ROTATION_TIME * 1000)) {
      setTray(false);
      lastTrayRotation = now;
    }
  } else {
    if (now - lastTrayRotation > (TRAY_CYCLE_INTERVAL * 1000)) {
      setTray(true);
      lastTrayRotation = now;
    }
  }
}

void updateLight(int day) {
  if (day >= LIGHT_HATCH_DAY) {
    setLight(true);
  } else {
    setLight(false);
  }
}

// ════════════════════════════════════════════════════════════════════
// ═══════════════════ FUNKCJE SYSTEMOWE ════════════════════════════════
// ════════════════════════════════════════════════════════════════════

void printStatus() {
  static unsigned long lastPrint = 0;
  unsigned long now = millis();
  
  if (now - lastPrint < 10000) return;
  lastPrint = now;
  
  float temp1 = getDS18B20Temp(0);
  float temp2 = (getDS18B20Count() > 1) ? getDS18B20Temp(1) : temp1;
  float temp3 = (getDS18B20Count() > 2) ? getDS18B20Temp(2) : temp1;
  float avgTemp = (temp1 + temp2 + temp3) / 3.0;
  float humidity = getDHTHumidity();
  
  float targetTemp = getTargetTemp(currentDay);
  float targetHumidity = getTargetHumidity(currentDay);
  
  Serial.println("\n┌─────────────────────────────────────────────────┐");
  Serial.print("│ DZIEŃ: ");
  Serial.print(currentDay);
  Serial.println("/28                                 │");
  Serial.println("├─────────────────────────────────────────────────┤");
  
  Serial.print("│ 🌡️  TEMPERATURA:                                │\n│    ");
  Serial.print("DS1:");
  Serial.print(temp1, 1);
  Serial.print("°C  DS2:");
  Serial.print(temp2, 1);
  Serial.print("°C  DS3:");
  Serial.print(temp3, 1);
  Serial.println("°C  │");
  
  Serial.print("│    Średnia: ");
  Serial.print(avgTemp, 1);
  Serial.print("°C  Cel: ");
  Serial.print(targetTemp, 1);
  Serial.println("°C         │");
  
  Serial.print("│ 💧 WILGOTNOŚĆ: ");
  Serial.print(humidity, 1);
  Serial.print("%  Cel: ");
  Serial.print(targetHumidity, 0);
  Serial.println("%              │");
  
  Serial.print("│ 🔥 GRZAŁKA: ");
  Serial.print(heaterPower);
  Serial.print("/255");
  Serial.println("                          │");
  
  Serial.print("│ 💨 WIATRAKI: F1:");
  Serial.print(fan1Power > 0 ? "ON " : "OFF");
  Serial.print(" F2:");
  Serial.print(fan2IsOn ? "ON " : "OFF");
  Serial.print(" F3:");
  Serial.print(fan3Power > 0 ? "ON " : "OFF");
  Serial.println("                │");
  
  Serial.print("│ 💦 NAWILŻACZ: ");
  Serial.print(humidifierState ? "ON " : "OFF");
  Serial.println("                      │");
  
  Serial.print("│ 🔄 TACA: ");
  Serial.print(trayState ? "OBRACA" : "STOP ");
  Serial.println("                         │");
  
  Serial.print("│ 💡 ŚWIATŁO: ");
  Serial.print(lightState ? "ON " : "OFF");
  Serial.println("                         │");
  
  if (ENABLE_SIMULATION) {
    Serial.println("│ 🔷 TRYB SYMULACJI (brak rzeczywistych czujników) │");
  }
  
  Serial.println("└─────────────────────────────────────────────────┘\n");
}

void manageIncubationDay() {
  static unsigned long lastDayCheck = 0;
  unsigned long now = millis();
  unsigned long elapsedSeconds = (now - startTime) / 1000;
  
  // DLA TESTÓW: zmień dzielnik na 1 aby dzień liczył się co sekundę
  // int calculatedDay = (elapsedSeconds / 1) + 1;  // SZYBKIE TESTYOWANIE
  
  int calculatedDay = (elapsedSeconds / 86400) + 1;  // NORMALNE: 1 dzień = 86400s
  
  if (calculatedDay != currentDay && calculatedDay <= 28) {
    currentDay = calculatedDay;
    Serial.print("\n🗓️  NOWY DZIEŃ INKUBACJI: ");
    Serial.println(currentDay);
  }
}

// ════════════════════════════════════════════════════════════════════
// ═══════════════════ SETUP I LOOP ════════════════════════════════════
// ════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║       INKUBATOR NA ESP32 - START        ║");
  Serial.println("║   DS18B20 + DHT22 + PWM + Relay        ║");
  if (ENABLE_SIMULATION) {
    Serial.println("║   🔷 TRYB SYMULACJI 🔷                 ║");
  }
  Serial.println("╚════════════════════════════════════════╝");
  
  initSensors();
  delay(100);
  
  initController();
  delay(100);
  
  setFan1Power(FAN1_PWM);
  
  startTime = millis();
  
  Serial.println("\n✓ System gotowy do pracy!\n");
  
  lastSensorRead = millis();
  lastControlLoop = millis();
  lastLog = millis();
  
  randomSeed(analogRead(0));
}

void loop() {
  unsigned long currentTime = millis();
  
  // ========== FAZA 1: ODCZYT SENSORÓW (co 2 sekundy) ==========
  if (currentTime - lastSensorRead >= SENSOR_READ_INTERVAL) {
    updateSensors();
    lastSensorRead = currentTime;
  }
  
  // ========== FAZA 2: PĘTLA STEROWANIA (co 5 sekund) ==========
  if (currentTime - lastControlLoop >= CONTROL_LOOP_INTERVAL) {
    
    float temp1 = getDS18B20Temp(0);
    float temp2 = (getDS18B20Count() > 1) ? getDS18B20Temp(1) : temp1;
    float temp3 = (getDS18B20Count() > 2) ? getDS18B20Temp(2) : temp1;
    float avgTemp = (temp1 + temp2 + temp3) / 3.0;
    
    float humidity = getDHTHumidity();
    
    // Smoothing filter
    static float smoothedTemp = avgTemp;
    smoothedTemp = smoothedTemp * 0.8 + avgTemp * 0.2;
    
    float targetTemp = getTargetTemp(currentDay);
    float targetHumidity = getTargetHumidity(currentDay);
    
    // LOGIKA STEROWANIA
    if (smoothedTemp < (targetTemp - TEMP_TOLERANCE)) {
      setHeaterPower(255);
    } else if (smoothedTemp > (targetTemp + TEMP_TOLERANCE)) {
      setHeaterPower(0);
    }
    
    if (humidity < (targetHumidity - HUMIDITY_TOLERANCE)) {
      setHumidifier(true);
    } else if (humidity > (targetHumidity + HUMIDITY_TOLERANCE)) {
      setHumidifier(false);
    }
    
    updateFans(smoothedTemp, currentDay);
    updateTray(currentDay);
    updateLight(currentDay);
    
    printStatus();
    
    lastControlLoop = currentTime;
  }
  
  // ========== FAZA 3: ZARZĄDZANIE DNIEM ==========
  manageIncubationDay();
  
  delay(50);
}

// ════════════════════════════════════════════════════════════════════
// ═════════════════════ KONIEC PROGRAMU ════════════════════════════════
// ════════════════════════════════════════════════════════════════════
