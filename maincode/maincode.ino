#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <SPI.h>
#include <LoRa.h>

// === GPS Setup ===
static const int GPS_RX = 15, GPS_TX = 4;
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPSPlus gps;

// === SIM800L Setup ===
SoftwareSerial sim800l(16, 17);  // RX, TX for SIM800L
const int button = 26;           // Emergency button pin
String phoneNumbers[] = { "+919370718105" }; // Add more if needed
int numNumbers = 1;

// === LoRa Setup ===
#define ss 5
#define rst 14
#define dio0 2

// === GPS State Tracking ===
unsigned long lastValidFixTime = 0;
bool gpsHasFix = false;

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600);
  sim800l.begin(9600);

  pinMode(button, INPUT_PULLUP);

  // === Initialize SIM800L ===
  if (!sendATCommand("AT", "OK", 2000)) {
    Serial.println("SIM800L initialization failed!");
  }
  if (!sendATCommand("AT+CMGF=1", "OK", 1000)) {
    Serial.println("Failed to set SMS text mode!");
  }

  // === Initialize LoRa ===
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(856E6)) {
    Serial.print(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initialized");

  Serial.println("System ready. Waiting for GPS fix...");
}

void loop() {
  // === Process incoming GPS data ===
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        lastValidFixTime = millis();
        gpsHasFix = true;
        Serial.print("Valid GPS Fix - ");
        Serial.print("Lat: "); Serial.print(gps.location.lat(), 6);
        Serial.print(", Lng: "); Serial.println(gps.location.lng(), 6);
      }
    }
  }

  // === Check for GPS fix timeout ===
  if (gpsHasFix && millis() - lastValidFixTime > 5000) {
    gpsHasFix = false;
    Serial.println("GPS signal lost!");
  }

  // === Button handling ===
  if (digitalRead(button) == LOW) {
    delay(50); // debounce
    if (digitalRead(button) == LOW) {
      sendGPSLocation(); // Send via SMS & LoRa
      while (digitalRead(button) == LOW); // wait for release
    }
  }

  // === Periodic GPS Status Display ===
  static unsigned long lastStatusTime = 0;
  if (millis() - lastStatusTime > 10000) {
    lastStatusTime = millis();
    Serial.print("GPS Status: ");
    Serial.println(gpsHasFix ? "Fix acquired" : "No fix");
    if (!gpsHasFix) {
      Serial.println("Make sure GPS has clear sky view");
    }
  }
}

// === Send GPS via SMS and LoRa ===
void sendGPSLocation() {
  String message;

  if (gpsHasFix) {
    message = "EMERGENCY! Location: http://maps.google.com/?q=";
    message += String(gps.location.lat(), 6);
    message += ",";
    message += String(gps.location.lng(), 6);
  } else {
    Serial.println("No GPS fix available!");
    message = "EMERGENCY! Location: http://maps.google.com/?q=18.525187,73.845484";
  }

  // 1. Send via LoRa first
  sendLoRa(message);

  // 2. Send via SMS
  sendSMS(phoneNumbers[0], message);
}

// === Send SMS with SIM800L ===
bool sendSMS(String number, String message) {
  if (!sendATCommand("AT+CMGS=\"" + number + "\"", ">", 2000)) {
    Serial.println("Failed to prepare SMS");
    return false;
  }

  sim800l.print(message);
  sim800l.write(26); // Ctrl+Z to send SMS

  if (!sendATCommand("", "OK", 5000)) {
    Serial.println("Failed to send SMS");
    return false;
  }

  Serial.println("SMS sent successfully");
  return true;
}

  bool sendLoRa(String data) {
  Serial.print("Sending via LoRa: ");
  Serial.println(data);
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
  return true;
}

 

// === AT Command Helper ===
bool sendATCommand(String cmd, String resp, unsigned long timeout) {
  if (cmd != "") {
    Serial.print("Sending: "); Serial.println(cmd);
    sim800l.println(cmd);
  }

  unsigned long start = millis();
  String response = "";

  while (millis() - start < timeout) {
    while (sim800l.available()) {
      char c = sim800l.read();
      response += c;
      Serial.write(c);
    }
    if (response.indexOf(resp) != -1) {
      return true;
    }
  }

  Serial.print("Timeout waiting for: "); Serial.println(resp);
  return false;
}
