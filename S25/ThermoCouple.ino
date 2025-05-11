/*
Uses Following components:
(1) Arudino UNO Rev 3
(1) Standard Arduino Kit SPDT Relay (pins: 5V, GND, D2)
(1) Arduino MicroSD Card Reader (pins: 5V, GND, D10, D11, D12, D13)
(1) Adafruit Max6675 K-type Thermocouple (pins: 5V, GND, D4, D5, D6)
*/

#include <SPI.h>
#include <SD.h>
#include <max6675.h>

// —— Configuration —— 
constexpr int THERMO_DO = 4;                                // Adafruit ThermoCouple Module Data Out pin : to send data
constexpr int THERMO_CS = 5;                                // Adafruit ThermoCouple Module Chip Select pin : to interface
constexpr int THERMO_CLK = 6;                               // Adafruit ThermoCouple Module Clock pin : to sync pings
constexpr int SD_CS = 10;                                   // MicroSD Card Reader Module Chip Select : to interface
constexpr int RELAY_PIN = 2;                                // Single-Pull-Double-Throw (SDPT) Relay pin : r/w

constexpr float TEMP_THRESHOLD = 35.0;                      // °C
constexpr unsigned long LOG_INTERVAL = 250UL;               // ms

MAX6675 thermocouple(THERMO_CLK, THERMO_CS, THERMO_DO);
File myFile;
unsigned long lastLogTime = 0;
bool RELAY_STATUS = true;

String makeTimestamp(unsigned long ms);

void setup() {
	Serial.begin(9600);                                     // Can remove if Serial Port not being used for competition

	pinMode(RELAY_PIN, OUTPUT);                             // Sets the Relay Pin on Arduino to Output
	digitalWrite(RELAY_PIN, HIGH);                          // Turns Relay on

  // initialize SD
  if (!SD.begin(SD_CS)) {
    Serial.println(F("SD init failed!"));                 // Serial print for debugging
    while (true) delay(1000);
  }

  myFile = SD.open("data.csv", FILE_WRITE);           
  if (!myFile) {
    Serial.println(F("Failed to open data.csv"));
    while (true) delay(1000);
  }

  myFile.println(F("Time, Temp_C, Relay"));
  myFile.flush();
}

void loop() {
  unsigned long now = millis();
  if (now - lastLogTime < LOG_INTERVAL) return;
  lastLogTime = now;

  float tempC = thermocouple.readCelsius();
  String ts  = makeTimestamp(now);

  updateRelay(tempC, RELAY_STATUS);
  logReading(ts, tempC);
}

String makeTimestamp(unsigned long ms) {
  unsigned long totalSec = ms / 1000;
  int m = (totalSec / 60) % 60;
  int s = totalSec % 60;
  int msRem = ms % 1000;
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d.%03d", m, s, msRem);
  return String(buf);
}

void logReading(const String& ts, float temp) {
  myFile.print(ts);
  myFile.print(',');
  myFile.print(temp, 2);
  myFile.print(',');
  myFile.println(RELAY_STATUS ? "ON" : "OFF");
  myFile.flush();

  // Serial printing for debugging purposes
  Serial.print(ts);
  Serial.print(" | ");
  Serial.print(temp, 2);
  Serial.print(" °C | Relay: ");
  Serial.println(RELAY_STATUS ? "ON" : "OFF");
}

void updateRelay(float temp, bool RELAY_STATUS) {
  bool shouldSwitch = (temp >= TEMP_THRESHOLD);
  digitalWrite(RELAY_PIN, shouldSwitch ? LOW : HIGH);
  if (shouldSwitch){
    RELAY_STATUS = false;
  }
}
