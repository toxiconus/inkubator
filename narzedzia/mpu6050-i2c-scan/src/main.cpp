#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Skanowanie I2C...");
  Wire.begin(4, 5);   // SDA = GPIO4, SCL = GPIO5
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Znaleziono: 0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
    }
  }
  Serial.println("Koniec");
}

void loop() {
  delay(1000);
}
