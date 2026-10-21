#include "controller.h"

Controller controller; // Globalna instancja

Controller::Controller()
  : heaterPower(0), fan1Power(0), fan2Power(0), fan3Power(0),
    humidifierState(false), trayState(false), lightState(false),
    incubationDay(1), currentMode(MODE_IDLE),
    lastTrayRotation(0), fan2LastToggle(0), fan2IsOn(false)
{
}

void Controller::begin() {
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
  
  // PWM = 0
  setHeaterPower(0);
  setFan1Power(0);
  setFan2Power(0);
  setFan3Power(0);
  
  Serial.println("PWM: OK");
  Serial.println("GPIO: OK");
  Serial.println("=========================================\n");
  
  currentMode = MODE_INCUBATION;
  lastTrayRotation = millis();
}

void Controller::update() {
  // Tu będzie logika sterowania wywołana z main loop
  // Będzie tutaj logika PID/threshold steracji dla każdego komponentu
}

// ========== SETTERY PWM ==========
void Controller::setHeaterPower(uint8_t pwm) {
  heaterPower = pwm;
  ledcWrite(HEATER_PIN, pwm);
}

void Controller::setFan1Power(uint8_t pwm) {
  fan1Power = pwm;
  ledcWrite(FAN1_PIN, pwm);
}

void Controller::setFan2Power(uint8_t pwm) {
  fan2Power = pwm;
  ledcWrite(FAN2_PIN, pwm);
}

void Controller::setFan3Power(uint8_t pwm) {
  fan3Power = pwm;
  ledcWrite(FAN3_PIN, pwm);
}

// ========== SETTERY GPIO RELAY ==========
void Controller::setHumidifier(bool state) {
  humidifierState = state;
  digitalWrite(HUMIDIFIER_PIN, state ? HIGH : LOW);
}

void Controller::setTray(bool state) {
  trayState = state;
  digitalWrite(TRAY_PIN, state ? HIGH : LOW);
}

void Controller::setLight(bool state) {
  lightState = state;
  digitalWrite(LIGHT_PIN, state ? HIGH : LOW);
}

// ========== GETTERY ==========
uint8_t Controller::getHeaterPower() {
  return heaterPower;
}

bool Controller::getHumidifierState() {
  return humidifierState;
}

bool Controller::getTrayState() {
  return trayState;
}

bool Controller::getLightState() {
  return lightState;
}

// ========== INKUBACJA ==========
void Controller::setIncubationDay(int day) {
  if (day >= 1 && day <= 28) {
    incubationDay = day;
  }
}

int Controller::getIncubationDay() {
  return incubationDay;
}

void Controller::setMode(ControlMode mode) {
  currentMode = mode;
}

ControlMode Controller::getMode() {
  return currentMode;
}

// ========== FUNKCJE POMOCNICZE ==========
float Controller::getTargetTemp(int day) {
  if (day <= 21) {
    return TEMP_DAY_1_21;
  } else {
    return TEMP_DAY_22_HATCH;
  }
}

float Controller::getTargetHumidity(int day) {
  if (day <= 18) {
    return TARGET_HUMIDITY_EARLY;
  } else {
    return TARGET_HUMIDITY_LATE;
  }
}

void Controller::updateHeater(float currentTemp, float targetTemp) {
  // Prosty sterownik threshold z histerezą
  if (currentTemp < (targetTemp - TEMP_TOLERANCE)) {
    setHeaterPower(255);  // Full power
  } else if (currentTemp > (targetTemp + TEMP_TOLERANCE)) {
    setHeaterPower(0);    // Off
  }
  // Wewnątrz tolerancji - utrzymaj ostatni stan
}

void Controller::updateHumidifier(float currentHumidity, float targetHumidity) {
  if (currentHumidity < (targetHumidity - HUMIDITY_TOLERANCE)) {
    setHumidifier(true);
  } else if (currentHumidity > (targetHumidity + HUMIDITY_TOLERANCE)) {
    setHumidifier(false);
  }
}

void Controller::updateFans(float currentTemp, int day) {
  // Fan 1 - stały
  setFan1Power(FAN1_PWM);
  
  // Fan 2 - cykliczny (pule/wył)
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

void Controller::updateTray(int day) {
  // Przestaje się obracać od dnia 21
  if (day >= TRAY_STOP_DAY) {
    setTray(false);
    return;
  }
  
  unsigned long now = millis();
  if (trayState) {
    // Taca pracuje - sprawdź czy czas końca
    if (now - lastTrayRotation > (TRAY_ROTATION_TIME * 1000)) {
      setTray(false);
      lastTrayRotation = now;
    }
  } else {
    // Taca off - sprawdź czy czas kolejnego cyklu
    if (now - lastTrayRotation > (TRAY_CYCLE_INTERVAL * 1000)) {
      setTray(true);
      lastTrayRotation = now;
    }
  }
}

void Controller::updateLight(int day) {
  if (day >= LIGHT_HATCH_DAY) {
    setLight(true);
  } else {
    setLight(false);
  }
}
