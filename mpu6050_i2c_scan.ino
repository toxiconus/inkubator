#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 5);   // SDA = GPIO4, SCL = GPIO5
  Serial.println("Skanowanie I2C...");
  for (byte address = 1; address < 127; address++) {
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
  // Nic nie robimy w pętli
}
