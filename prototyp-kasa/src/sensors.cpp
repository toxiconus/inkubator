#include "sensors.h"

Sensors sensors; // Globalna instancja

Sensors::Sensors() 
  : oneWire(ONE_WIRE_BUS), 
    ds18b20(&oneWire),
    dht(DHT_PIN, DHT_TYPE),
    deviceCount(0),
    lastDHTTemp(0),
    lastDHTHumidity(0),
    lastReadTime(0)
{
}

void Sensors::begin() {
  Serial.println("\n========== Inicjalizacja sensorów ==========");
  
  // DS18B20
  ds18b20.begin();
  deviceCount = ds18b20.getDeviceCount();
  Serial.print("DS18B20: Znaleziono ");
  Serial.print(deviceCount);
  Serial.println(" czujników");
  
  // Ustaw rozdzielczość wszystkich czujników na 12-bit
  ds18b20.setResolution(12);
  
  // Wyświetl adresy
  if (deviceCount > 0) {
    printDS18B20Addresses();
  }
  
  // DHT22
  dht.begin();
  Serial.println("DHT22: Inicjalizacja OK");
  
  Serial.println("=========================================\n");
  lastReadTime = millis();
}

void Sensors::update() {
  unsigned long currentTime = millis();
  
  // Odczyt DS18B20 (szybko, przez requestTemperatures())
  ds18b20.requestTemperatures();
  
  // Odczyt DHT22 (co 2 sekundy minimum - ma własne ograniczenia)
  if (currentTime - lastReadTime >= 2000) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (!isnan(h) && !isnan(t)) {
      lastDHTTemp = t;
      lastDHTHumidity = h;
    } else {
      Serial.println("⚠ Błąd odczytu DHT22!");
    }
    lastReadTime = currentTime;
  }
}

float Sensors::getDS18B20Temp(int index) {
  if (index >= deviceCount) return -999.0;
  float temp = ds18b20.getTempCByIndex(index);
  if (temp == DEVICE_DISCONNECTED_C) return -999.0;
  return temp;
}

float Sensors::getDHTTemp() {
  return lastDHTTemp;
}

float Sensors::getDHTHumidity() {
  return lastDHTHumidity;
}

int Sensors::getDS18B20Count() {
  return deviceCount;
}

void Sensors::printDS18B20Addresses() {
  Serial.println("Adresy DS18B20:");
  for (int i = 0; i < deviceCount; i++) {
    DeviceAddress address;
    if (ds18b20.getAddress(address, i)) {
      Serial.print("  Czujnik ");
      Serial.print(i);
      Serial.print(": ");
      printAddress(address);
      Serial.println();
    }
  }
}

void Sensors::printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
