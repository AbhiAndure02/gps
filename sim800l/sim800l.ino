    #include <SoftwareSerial.h>
    #include <TinyGPS++.h>
    
    // GPS Setup
    static const int GPS_RX = 4, GPS_TX = 5;
    SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
    TinyGPSPlus gps;
    
    // SIM800L Setup
    SoftwareSerial sim800l(16, 17);
    const int button = 27;
    
    String phoneNumbers[] = { "+919881435328" };
    int numNumbers = 1;
    
    unsigned long lastValidFixTime = 0;
    bool gpsHasFix = false;
    
    void setup() {
      Serial.begin(115200);  // Increased baud rate for better debugging
      gpsSerial.begin(9600);
      sim800l.begin(9600);
    
      pinMode(button, INPUT_PULLUP);
    
      // Initialize SIM800L
      if (!sendATCommand("AT", "OK", 2000)) {
        Serial.println("SIM800L initialization failed!");
      }
      if (!sendATCommand("AT+CMGF=1", "OK", 1000)) {
        Serial.println("Failed to set text mode!");
      }
    
      Serial.println("System ready. Waiting for GPS fix...");
    }
    
    void loop() {
      // GPS data processing
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
    
      // Check if GPS lost fix
      if (gpsHasFix && millis() - lastValidFixTime > 5000) {
        gpsHasFix = false;
        Serial.println("GPS signal lost!");
      }
    
      // Button handling
      if (digitalRead(button) == LOW) {
        delay(50); // Debounce
        if (digitalRead(button) == LOW) {
          sendGPSLocation();
          while (digitalRead(button) == LOW); // Wait for release
        }
      }
    
      // Print GPS status periodically
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
    
    void sendGPSLocation() {
      if (gpsHasFix) {
        String message = "EMERGENCY! Location: http://maps.google.com/?q=";
        message += String(gps.location.lat(), 6);
        message += ",";
        message += String(gps.location.lng(), 6);
        sendSMS(phoneNumbers[0], message);
      } else {
        Serial.println("No GPS fix available!");
        sendSMS(phoneNumbers[0], "GPS signal lost - no location available");
      }
    }
    
    bool sendSMS(String number, String message) {
      if (!sendATCommand("AT+CMGS=\"" + number + "\"", ">", 2000)) {
        Serial.println("Failed to prepare SMS");
        return false;
      }
      
      sim800l.print(message);
      sim800l.write(26); // Ctrl+Z
      
      if (!sendATCommand("", "OK", 5000)) {
        Serial.println("Failed to send SMS");
        return false;
      }
      
      Serial.println("SMS sent successfully");
      return true;
    }
    
    bool sendATCommand(String cmd, String resp, unsigned long timeout) {
      Serial.print("Sending: "); Serial.println(cmd);
      sim800l.println(cmd);
    
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
