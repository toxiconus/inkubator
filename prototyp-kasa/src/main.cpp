#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "controller.h"

// ========== ZMIENNE SYSTEMOWE ==========
unsigned long lastSensorRead = 0;
unsigned long lastControlLoop = 0;
unsigned long lastLog = 0;
unsigned long startTime = 0;  // Czas startu, aby obliczyć dzień

int currentDay = 1;  // Bieżący dzień inkubacji (1-28)

// ========== PROTOTYPY ==========
void printStatus();
void manageIncubationDay();

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║       INKUBATOR NA ESP32 - START        ║");
  Serial.println("║   DS18B20 + DHT22 + PWM + Relay        ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  // Inicjalizacja modułów
  sensors.begin();
  delay(100);
  
  controller.begin();
  delay(100);
  
  // Ustaw początkowe warunki
  controller.setMode(MODE_INCUBATION);
  controller.setIncubationDay(1);
  
  // Fan 1 zawsze działa na pewną moc
  controller.setFan1Power(FAN1_PWM);
  
  startTime = millis();
  
  Serial.println("\n✓ System gotowy do pracy!\n");
  
  lastSensorRead = millis();
  lastControlLoop = millis();
  lastLog = millis();
}

// ========== MAIN LOOP ==========
void loop() {
  unsigned long currentTime = millis();
  
  // ========== FAZA 1: ODCZYT SENSORÓW (co 2 sekundy) ==========
  if (currentTime - lastSensorRead >= SENSOR_READ_INTERVAL) {
    sensors.update();
    lastSensorRead = currentTime;
  }
  
  // ========== FAZA 2: PĘTLA STEROWANIA (co 5 sekund) ==========
  if (currentTime - lastControlLoop >= CONTROL_LOOP_INTERVAL) {
    
    // Pobierz aktualne odczyty
    float temp1 = sensors.getDS18B20Temp(0);
    float temp2 = (sensors.getDS18B20Count() > 1) ? sensors.getDS18B20Temp(1) : temp1;
    float temp3 = (sensors.getDS18B20Count() > 2) ? sensors.getDS18B20Temp(2) : temp1;
    float avgTemp = (temp1 + temp2 + temp3) / 3.0;
    
    float humidity = sensors.getDHTHumidity();
    
    // Oblicz średnią dla stabilności
    static float smoothedTemp = avgTemp;
    smoothedTemp = smoothedTemp * 0.8 + avgTemp * 0.2;  // EMA filtr
    
    // Pobierz parametry dla bieżącego dnia
    float targetTemp = controller.getTargetTemp(currentDay);
    float targetHumidity = controller.getTargetHumidity(currentDay);
    
    // LOGIKA STEROWANIA
    // - Grzałka z threshold + histereza
    if (smoothedTemp < (targetTemp - TEMP_TOLERANCE)) {
      controller.setHeaterPower(255);  // Pełna moc
    } else if (smoothedTemp > (targetTemp + TEMP_TOLERANCE)) {
      controller.setHeaterPower(0);    // Wyłącz
    }
    
    // - Wilgotność (nawilżacz soniczny)
    if (humidity < (targetHumidity - HUMIDITY_TOLERANCE)) {
      controller.setHumidifier(true);
    } else if (humidity > (targetHumidity + HUMIDITY_TOLERANCE)) {
      controller.setHumidifier(false);
    }
    
    // - Wiatraki (patrz controller.cpp dla logiki)
    controller.updateFans(smoothedTemp, currentDay);
    controller.updateTray(currentDay);
    controller.updateLight(currentDay);
    
    // Wyświetl status co pętlę sterowania
    printStatus();
    
    lastControlLoop = currentTime;
  }
  
  // ========== FAZA 3: LOGGING (co 1 minutę) ==========
  if (currentTime - lastLog >= LOG_INTERVAL) {
    // Tutaj można dodać zapis do SPIFFS w przyszłości
    // logData(sensors, controller);
    lastLog = currentTime;
  }
  
  // ========== ZARZĄDZANIE DNIEM INKUBACJI ==========
  manageIncubationDay();
  
  // Mały delay aby nie zator CPU
  delay(50);
}

// ========== FUNKCJE POMOCNICZE ==========

void printStatus() {
  static unsigned long lastPrint = 0;
  unsigned long now = millis();
  
  // Drukuj info co 10 sekund (aby nie zalewać Serial)
  if (now - lastPrint < 10000) return;
  lastPrint = now;
  
  // Odczyty
  float temp1 = sensors.getDS18B20Temp(0);
  float temp2 = (sensors.getDS18B20Count() > 1) ? sensors.getDS18B20Temp(1) : temp1;
  float temp3 = (sensors.getDS18B20Count() > 2) ? sensors.getDS18B20Temp(2) : temp1;
  float avgTemp = (temp1 + temp2 + temp3) / 3.0;
  float humidity = sensors.getDHTHumidity();
  
  // Parametry sterowania
  float targetTemp = controller.getTargetTemp(currentDay);
  float targetHumidity = controller.getTargetHumidity(currentDay);
  
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
  Serial.print(controller.getHeaterPower());
  Serial.print("/255");
  Serial.println("                          │");
  
  Serial.print("│ 💨 WIATRAKI: F1:");
  Serial.print(controller.getHeaterPower() ? "ON " : "OFF");
  Serial.print(" F2:");
  Serial.print(controller.getTrayState() ? "ON " : "OFF");
  Serial.print(" F3:");
  Serial.print(controller.getLightState() ? "ON " : "OFF");
  Serial.println("                │");
  
  Serial.print("│ 💦 NAWILŻACZ: ");
  Serial.print(controller.getHumidifierState() ? "ON " : "OFF");
  Serial.println("                      │");
  
  Serial.print("│ 🔄 TACA: ");
  Serial.print(controller.getTrayState() ? "OBRACA" : "STOP ");
  Serial.println("                         │");
  
  Serial.print("│ 💡 ŚWIATŁO: ");
  Serial.print(controller.getLightState() ? "ON " : "OFF");
  Serial.println("                         │");
  
  Serial.println("└─────────────────────────────────────────────────┘\n");
}

void manageIncubationDay() {
  static unsigned long lastDayCheck = 0;
  unsigned long now = millis();
  unsigned long elapsedSeconds = (now - startTime) / 1000;
  
  // Oblicz dzień (1 dzień = 86400 sekund) - ewentualnie zmień dla testowania
  // Dla testów: wykomentuj poniżej aby dzień liczył się szybciej
  int calculatedDay = (elapsedSeconds / 86400) + 1;
  
  if (calculatedDay != currentDay && calculatedDay <= 28) {
    currentDay = calculatedDay;
    Controller temp;
    temp.setIncubationDay(currentDay);
    Serial.print("\n🗓️  NOWY DZIEŃ INKUBACJI: ");
    Serial.println(currentDay);
  }
}

// ========== DO IMPLEMENTACJI W PRZYSZŁOŚCI ==========
// void logData(Sensors& sensors, Controller& controller) {
//   // Zapis do SPIFFS
// }
