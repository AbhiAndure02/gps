#include <SPI.h>
#include <LoRa.h>

// LoRa pins
#define ss 5
#define rst 14
#define dio0 2

// Buzzer pin
#define BUZZER_PIN 26

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver with Buzzer");

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Make sure buzzer is off initially

  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(865E6)) {
    Serial.print(".");
    delay(500);
  }

  LoRa.setSyncWord(0xF3); // Sync word must match transmitter
  Serial.println("\nLoRa Initializing OK!");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.println(">> Packet Received <<");

    String LoRaData = "";
    while (LoRa.available()) {
      LoRaData += (char)LoRa.read();
    }

    Serial.print("Message: ");
    Serial.println(LoRaData);

    if (LoRaData == "ON") {
      digitalWrite(BUZZER_PIN, HIGH);
      Serial.println("Buzzer ON");
    } 
    else if (LoRaData == "OFF") {
      digitalWrite(BUZZER_PIN, HIGH);
      Serial.println("Buzzer OFF");
    } else {
      Serial.println("Unknown command.");
            digitalWrite(BUZZER_PIN, HIGH);

    }

    Serial.print("RSSI: ");
    Serial.println(LoRa.packetRssi());
  }

  delay(100); // Reduce CPU usage
}
