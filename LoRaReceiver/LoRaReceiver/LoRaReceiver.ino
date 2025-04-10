#include <SPI.h>
#include <LoRa.h>

// LoRa pins
#define ss 5
#define rst 14
#define dio0 2

// Buzzer pin
#define BUZZER_PIN 26

void setup() {
  // Serial for debugging
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver with Buzzer");

  // Set buzzer pin as output
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Make sure buzzer is off initially

  // Initialize LoRa
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(865E6)) { // Use 865E6 or 866E6 for India
    Serial.print(".");
    delay(500);
  }

  LoRa.setSyncWord(0xF3); // Sync word must match transmitter
  Serial.println("\nLoRa Initializing OK!");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet: ");

    String LoRaData = "";
    while (LoRa.available()) {
      LoRaData += (char)LoRa.read();
    }

    Serial.println(LoRaData);

    // Check command and activate buzzer
    if (LoRaData == "ON") {
      digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer
      Serial.println("Buzzer ON");
    } 
    else if (LoRaData == "OFF") {
      digitalWrite(BUZZER_PIN, LOW); // Turn off buzzer
      Serial.println("Buzzer OFF");
    }

    // Show RSSI
    Serial.print("RSSI: ");
    Serial.println(LoRa.packetRssi());
  }
}
