#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "config.h"

enum ControlMode {
  MODE_IDLE,
  MODE_INCUBATION,
  MODE_HATCHING,
  MODE_EMERGENCY
};

class Controller {
public:
  Controller();
  void begin();
  void update();
  
  // Sterowanie
  void setHeaterPower(uint8_t pwm);      // 0-255
  void setFan1Power(uint8_t pwm);        // 0-255
  void setFan2Power(uint8_t pwm);        // 0-255
  void setFan3Power(uint8_t pwm);        // 0-255
  
  void setHumidifier(bool state);        // ON/OFF
  void setTray(bool state);              // ON/OFF (rotacja)
  void setLight(bool state);             // ON/OFF
  
  // Gettery do debugowania
  uint8_t getHeaterPower();
  bool getHumidifierState();
  bool getTrayState();
  bool getLightState();
  
  // Profil inkubacji
  void setIncubationDay(int day);        // 1-28
  int getIncubationDay();
  
  // Ustawianie trybu
  void setMode(ControlMode mode);
  ControlMode getMode();
  
private:
  uint8_t heaterPower;
  uint8_t fan1Power;
  uint8_t fan2Power;
  uint8_t fan3Power;
  
  bool humidifierState;
  bool trayState;
  bool lightState;
  
  int incubationDay;
  ControlMode currentMode;
  
  unsigned long lastTrayRotation;
  unsigned long fan2LastToggle;
  bool fan2IsOn;
  
  // Funkcje wewnętrzne logiki
  void updateHeater(float currentTemp, float targetTemp);
  void updateHumidifier(float currentHumidity, float targetHumidity);
  void updateFans(float currentTemp, int day);
  void updateTray(int day);
  void updateLight(int day);
  
  float getTargetTemp(int day);
  float getTargetHumidity(int day);
};

extern Controller controller;

#endif // CONTROLLER_H
