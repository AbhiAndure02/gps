 #include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <LoRa.h>

// === LoRa Pins (adjust based on your ESP8266 wiring) ===
#define ss 15    // GPIO15
#define rst 16   // GPIO16
#define dio0 5  // GPIO5

// === Buzzer Pin (adjust as needed) ===
#define BUZZER_PIN 4  // GPIO4

// === WiFi AP Credentials ===
const char* apSSID = "LoRa_Receiver";
const char* apPassword = "12345678";

// === Web Server ===
ESP8266WebServer server(80);

// === LoRa Variables ===
String lastLoRaMessage = "Waiting...";
int lastRSSI = 0;
int messageCount = 0;
bool buzzerActive = false;
unsigned long buzzerStartTime = 0;

// === Manual Buzzer Control ===
bool manualBuzz = false;
unsigned long manualBuzzStart = 0;

void setup() {
  Serial.begin(9600);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Start WiFi Access Point
  WiFi.softAP(apSSID, apPassword);
  Serial.println("AP IP: " + WiFi.softAPIP().toString());

  // Web Server Routes
  server.on("/", handleRoot);
  server.on("/control", handleControlPage);
  server.on("/buzz", HTTP_POST, handleBuzz);
  server.begin();

  // LoRa Setup
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(865E6)) {
    Serial.print(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.enableCrc();
  Serial.println("LoRa Initialized");
}

void loop() {
  server.handleClient();

  // Send periodic beacon
  static unsigned long lastBeacon = 0;
  if (millis() - lastBeacon > 2000) {
    LoRa.beginPacket();
    LoRa.print("RECEIVER_READY");
    LoRa.endPacket();
    lastBeacon = millis();
  }

  // Receive LoRa packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String LoRaData = "";
    while (LoRa.available()) {
      LoRaData += (char)LoRa.read();
    }

    if (LoRaData == "RECEIVER_READY") return;

    lastLoRaMessage = LoRaData;
    lastRSSI = LoRa.packetRssi();
    messageCount++;

    Serial.println(">> LoRa Message Received <<");
    Serial.println("Message: " + lastLoRaMessage);
    Serial.println("RSSI: " + String(lastRSSI));


  }

  // LoRa message-based buzzer control
  if (buzzerActive && millis() - buzzerStartTime >= 1000) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerStartTime = millis();
    messageCount--;

    if (messageCount <= 0) {
      buzzerActive = false;
    } else {
      delay(200);
      digitalWrite(BUZZER_PIN, HIGH);
    }
  }

  // Manual buzzer control (10 seconds)
  if (manualBuzz && millis() - manualBuzzStart >= 10000) {
    digitalWrite(BUZZER_PIN, LOW);
    manualBuzz = false;
  }

  delay(20);
}

// === Root Page ===
void handleRoot() {
  String html = "<html><head><title>LoRa Receiver</title></head><body>";
  html += "<h2>LoRa Receiver - Access Point Mode</h2>";
  html += "<p><b>Last LoRa Message:</b> " + lastLoRaMessage + "</p>";
  html += "<p><b>Signal Strength (RSSI):</b> " + String(lastRSSI) + "</p>";
  html += "<p><b>Pending Buzzes:</b> " + String(messageCount) + "</p>";
  html += "<p><a href='/control'><button>Go to Control Page</button></a></p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// === Control Page ===
void handleControlPage() {
  String html = "<html><head><title>Control</title></head><body>";
  html += "<h2>Control Page</h2>";
  html += "<form action=\"/buzz\" method=\"POST\">";
  html += "<button type=\"submit\" style=\"padding:10px 20px; font-size:16px;\">Buzz for 10 Seconds</button>";
  html += "</form>";
  html += "<p><a href='/'><button>Back to Status</button></a></p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// === Handle Buzzer Trigger ===
void handleBuzz() {
  manualBuzz = true;
  manualBuzzStart = millis();
  digitalWrite(BUZZER_PIN, HIGH);

  server.sendHeader("Location", "/control");
  server.send(303);
}
