#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

// ╔════════════════════════════════════════════════════════════════════╗
// ║              INKUBATOR NA WOKWI.COM - SYMULACJA                    ║
// ║   Pełna symulacja z rzeczywistymi sensorami Wokwi + wizualizacja   ║
// ╚════════════════════════════════════════════════════════════════════╝

// ========== KONFIGURACJA PINÓW ==========
#define ONE_WIRE_BUS 4              // DS18B20 (OneWire)
#define DHT_PIN 15                  // DHT22 Pin

// ========== PWM / TIMERY ==========
#define PWM_FREQUENCY 5000          // 5kHz dla grzałek
#define PWM_RESOLUTION 8            // 8-bit (0-255)

#define HEATER_PIN 2                // LED1: Grzałka
#define HEATER_CHANNEL 0
#define FAN1_PIN 19                 // LED2: Wentylator 1
#define FAN1_CHANNEL 1
#define FAN2_PIN 18                 // LED3: Wentylator 2
#define FAN2_CHANNEL 2
#define FAN3_PIN 5                  // LED4: Wentylator 3
#define FAN3_CHANNEL 3

// ========== GPIO - RELAY ==========
#define HUMIDIFIER_PIN 12           // LED5: Nawilżacz
#define TRAY_PIN 14                 // LED6: Taca
#define LIGHT_PIN 13                // LED7: Światło

#define DHT_TYPE DHT22

// ========== PARAMETRY TEMPERATUROWE ==========
#define TARGET_TEMP 37.5
#define TEMP_TOLERANCE 0.5
#define TEMP_DAY_1_21 37.5
#define TEMP_DAY_22_HATCH 37.2

// ========== WILGOTNOŚĆ ==========
#define TARGET_HUMIDITY_EARLY 40
#define TARGET_HUMIDITY_LATE 75
#define HUMIDITY_TOLERANCE 5

// ========== CYKL WIATRAKÓW ==========
#define FAN1_PWM 200
#define FAN2_CYCLE_ON 300
#define FAN2_CYCLE_OFF 200
#define FAN3_TEMP_THRESHOLD 38.5

// ========== TACA ==========
#define TRAY_CYCLE_INTERVAL 3600
#define TRAY_ROTATION_TIME 10
#define TRAY_STOP_DAY 21

// ========== ŚWIATŁO ==========
#define LIGHT_HATCH_DAY 21

// ========== OPÓŹNIENIA ==========
#define SENSOR_READ_INTERVAL 2000
#define CONTROL_LOOP_INTERVAL 5000
#define LOG_INTERVAL 60000

// ========== ZMIENNE GLOBALNE ==========
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
DHT dht(DHT_PIN, DHT_TYPE);

int deviceCount = 0;
float lastDHTTemp = 25.0;
float lastDHTHumidity = 50.0;
unsigned long lastSensorReadTime = 0;

// Kontroler
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

// System
unsigned long lastSensorRead = 0;
unsigned long lastControlLoop = 0;
unsigned long lastLog = 0;
unsigned long startTime = 0;
int currentDay = 1;

// ════════════════════════════════════════════════════════════════════
// ═══════════════════ INICJALIZACJA ════════════════════════════════════
// ════════════════════════════════════════════════════════════════════

void initSensors() {
  Serial.println("\n========== Inicjalizacja sensorów ==========");
  
  ds18b20.begin();
  deviceCount = ds18b20.getDeviceCount();
  ds18b20.setResolution(12);
  
  Serial.print("✓ DS18B20: Znaleziono ");
  Serial.print(deviceCount);
  Serial.println(" czujników");
  
  // Wyświetl adresy
  for (int i = 0; i < deviceCount; i++) {
    DeviceAddress address;
    if (ds18b20.getAddress(address, i)) {
      Serial.print("  Czujnik ");
      Serial.print(i);
      Serial.print(": ");
      for (uint8_t j = 0; j < 8; j++) {
        if (address[j] < 16) Serial.print("0");
        Serial.print(address[j], HEX);
      }
      Serial.println();
    }
  }
  
  dht.begin();
  Serial.println("✓ DHT22: Inicjalizacja OK");
  Serial.println("=========================================\n");
}

void initController() {
  Serial.println("\n========== Inicjalizacja kontrolera ==========");
  
  // PWM (Arduino Core 2.0+ API)
  ledcAttach(HEATER_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(FAN1_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(FAN2_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(FAN3_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  
  // GPIO Relay
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  pinMode(TRAY_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  
  digitalWrite(HUMIDIFIER_PIN, LOW);
  digitalWrite(TRAY_PIN, LOW);
  digitalWrite(LIGHT_PIN, LOW);
  
  ledcWrite(HEATER_PIN, 0);
  ledcWrite(FAN1_PIN, 0);
  ledcWrite(FAN2_PIN, 0);
  ledcWrite(FAN3_PIN, 0);
  
  Serial.println("✓ PWM: OK");
  Serial.println("✓ GPIO: OK");
  Serial.println("=========================================\n");
  
  lastTrayRotation = millis();
}

// ════════════════════════════════════════════════════════════════════
// ═══════════════════ SENSORY ══════════════════════════════════════════
// ════════════════════════════════════════════════════════════════════

void updateSensors() {
  unsigned long currentTime = millis();
  
  ds18b20.requestTemperatures();
  
  if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (!isnan(h) && !isnan(t)) {
      lastDHTTemp = t;
      lastDHTHumidity = h;
    }
    
    lastSensorReadTime = currentTime;
  }
}

float getDS18B20Temp(int index) {
  if (index >= deviceCount) return -999.0;
  float temp = ds18b20.getTempCByIndex(index);
  if (temp == DEVICE_DISCONNECTED_C) return -999.0;
  return temp;
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
// ═══════════════════ KONTROLER ════════════════════════════════════════
// ════════════════════════════════════════════════════════════════════

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
  setFan1Power(FAN1_PWM);
  
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
// ═══════════════════ WYŚWIETLANIE STATUS ══════════════════════════════
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
  
  Serial.println("│ 🌐 WOKWI SIMULATION                             │");
  Serial.println("└─────────────────────────────────────────────────┘\n");
}

void manageIncubationDay() {
  unsigned long now = millis();
  unsigned long elapsedSeconds = (now - startTime) / 1000;
  
  // WOKWI TESTING: zmień dzielnik do szybszych testów
  // int calculatedDay = (elapsedSeconds / 1) + 1;     // 1 dzień = 1 sekunda
  // int calculatedDay = (elapsedSeconds / 10) + 1;    // 1 dzień = 10 sekund
  
  int calculatedDay = (elapsedSeconds / 86400) + 1;  // 1 dzień = 24 godziny
  
  if (calculatedDay != currentDay && calculatedDay <= 28) {
    currentDay = calculatedDay;
    Serial.print("\n🗓️  NOWY DZIEŃ INKUBACJI: ");
    Serial.println(currentDay);
  }
}

// ════════════════════════════════════════════════════════════════════
// ═══════════════════ SETUP I LOOP ═════════════════════════════════════
// ════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║       INKUBATOR NA ESP32 - WOKWI       ║");
  Serial.println("║   DS18B20 + DHT22 + PWM + Relay        ║");
  Serial.println("║   🌐 SYMULACJA W PRZEGLĄDARCE 🌐       ║");
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
}

void loop() {
  unsigned long currentTime = millis();
  
  // ========== FAZA 1: ODCZYT SENSORÓW ==========
  if (currentTime - lastSensorRead >= SENSOR_READ_INTERVAL) {
    updateSensors();
    lastSensorRead = currentTime;
  }
  
  // ========== FAZA 2: PĘTLA STEROWANIA ==========
  if (currentTime - lastControlLoop >= CONTROL_LOOP_INTERVAL) {
    
    float temp1 = getDS18B20Temp(0);
    float temp2 = (getDS18B20Count() > 1) ? getDS18B20Temp(1) : temp1;
    float temp3 = (getDS18B20Count() > 2) ? getDS18B20Temp(2) : temp1;
    float avgTemp = (temp1 + temp2 + temp3) / 3.0;
    float humidity = getDHTHumidity();
    
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
  
  // ========== ZARZĄDZANIE DNIEM ==========
  manageIncubationDay();
  
  delay(50);
}
