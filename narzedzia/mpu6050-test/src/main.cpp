#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "MPU6050.h"

const char* AP_SSID = "MPU6050-ESP";
const char* AP_PASSWORD = "esp12345";

MPU6050 mpu;
bool mpuOk = false;
unsigned long lastStatusMillis = 0;
const unsigned long statusIntervalMs = 5000;

float lastPitch = 0.0;
float lastRoll = 0.0;
int16_t lastAx = 0;
int16_t lastAy = 0;
int16_t lastAz = 0;
int16_t lastGx = 0;
int16_t lastGy = 0;
int16_t lastGz = 0;

WebServer server(80);

String scanI2cBusList() {
  String result = "";
  bool first = true;
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    uint8_t res = Wire.endTransmission();
    if (res == 0) {
      if (!first) result += ",";
      result += "\"0x";
      if (address < 16) result += "0";
      String hexAddr = String(address, HEX);
      hexAddr.toUpperCase();
      result += hexAddr;
      result += "\"";
      first = false;
    }
  }
  return result;
}

void reportStatus() {
  float temp = temperatureRead();
  Serial.println("\n=== ESP Status ===");
  Serial.printf("uptime=%lu ms freeHeap=%u temp=%.1f C\n", millis(), ESP.getFreeHeap(), temp);
  Serial.printf("MPU6050 connection=%s\n", mpuOk ? "OK" : "FAIL");
  Serial.print("I2C devices:");
  String devices = scanI2cBusList();
  if (devices.length() == 0) {
    Serial.print(" none\n");
  } else {
    Serial.print(" ");
    Serial.println(devices);
  }
  Serial.println("==================\n");
}

void handleRoot() {
  const char* page = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>MPU6050 / ESP Status</title>
  <style>
    body { font-family: Arial, sans-serif; background: #f4f7fb; color: #1a1a1a; margin: 0; padding: 20px; }
    .card { background: white; border-radius: 12px; padding: 18px; box-shadow: 0 2px 10px rgba(0,0,0,0.08); margin-bottom: 16px; }
    h1 { margin-top: 0; }
    pre { background: #eef1f8; padding: 12px; border-radius: 8px; overflow-x: auto; }
  </style>
</head>
<body>
  <div class="card">
    <h1>MPU6050 / ESP Status</h1>
    <p>Połącz się z siecią Wi-Fi <strong>MPU6050-ESP</strong> hasło <strong>esp12345</strong> i otwórz tę stronę ponownie.</p>
    <p>Strona automatycznie odświeża stan co 5 sekund.</p>
  </div>
  <div class="card">
    <h2>Stan urządzenia</h2>
    <pre id="status">Ładowanie...</pre>
  </div>
  <script>
    async function updateStatus() {
      try {
        const res = await fetch('/status');
        const data = await res.json();
        document.getElementById('status').textContent = JSON.stringify(data, null, 2);
      } catch (err) {
        document.getElementById('status').textContent = 'Błąd odczytu statusu: ' + err;
      }
    }
    updateStatus();
    setInterval(updateStatus, 5000);
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", page);
}

void handleStatus() {
  String i2cDevices = scanI2cBusList();
  if (i2cDevices.length() == 0) {
    i2cDevices = "";
  }
  String payload = "{";
  payload += "\"uptime\":" + String(millis()) + ",";
  payload += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  payload += "\"temp\":" + String(temperatureRead(), 1) + ",";
  payload += "\"mpuOk\":" + String(mpuOk ? "true" : "false") + ",";
  payload += "\"pitch\":" + String(lastPitch, 1) + ",";
  payload += "\"roll\":" + String(lastRoll, 1) + ",";
  payload += "\"ax\":" + String(lastAx) + ",";
  payload += "\"ay\":" + String(lastAy) + ",";
  payload += "\"az\":" + String(lastAz) + ",";
  payload += "\"gx\":" + String(lastGx) + ",";
  payload += "\"gy\":" + String(lastGy) + ",";
  payload += "\"gz\":" + String(lastGz) + ",";
  payload += "\"i2c\": [" + i2cDevices + "]";
  payload += "}";
  server.send(200, "application/json", payload);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(4, 5); // SDA = GPIO4, SCL = GPIO5
  Serial.println("Start MPU6050 test...");

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 nie znaleziony!");
    while (true) {
      delay(1000);
    }
  }
  mpuOk = true;
  Serial.println("MPU6050 OK");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.printf("Access point started: %s\n", AP_SSID);
  Serial.printf("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();
  Serial.println("Web server started on port 80");
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  lastAx = ax;
  lastAy = ay;
  lastAz = az;
  lastGx = gx;
  lastGy = gy;
  lastGz = gz;

  lastPitch = atan2(-ax, sqrt((float)ay*ay + (float)az*az)) * 180.0 / PI;
  lastRoll = atan2((float)ay, (float)az) * 180.0 / PI;

  Serial.printf("Pitch: %.1f°  Roll: %.1f°  | ax=%d ay=%d az=%d gx=%d gy=%d gz=%d\n",
                lastPitch, lastRoll, lastAx, lastAy, lastAz, lastGx, lastGy, lastGz);

  unsigned long now = millis();
  if (now - lastStatusMillis >= statusIntervalMs) {
    reportStatus();
    lastStatusMillis = now;
  }

  server.handleClient();
  delay(200);
}
