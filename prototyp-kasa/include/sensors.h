#ifndef SENSORS_H
#define SENSORS_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include "config.h"

class Sensors {
public:
  Sensors();
  void begin();
  void update();
  
  // Gettery dla temperatury
  float getDS18B20Temp(int index);
  float getDHTTemp();
  float getDHTHumidity();
  
  // Informacje o czujnikach
  int getDS18B20Count();
  void printDS18B20Addresses();
  
private:
  OneWire oneWire;
  DallasTemperature ds18b20;
  DHT dht;
  
  int deviceCount;
  float lastDHTTemp;
  float lastDHTHumidity;
  unsigned long lastReadTime;
  
  void printAddress(DeviceAddress deviceAddress);
};

extern Sensors sensors;

#endif // SENSORS_H
