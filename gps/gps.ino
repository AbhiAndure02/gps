#include <SoftwareSerial.h>

SoftwareSerial ss(16, 17); // RX, TX (GPIO4, GPIO5)
String gpsData = "";

void setup() {
  Serial.begin(115200);
  ss.begin(9600); // NEO-6M GPS baud rate
  Serial.println("GPS Latitude and Longitude Display");
}

void loop() {
  while (ss.available()) {
    char c = ss.read();
    gpsData += c;  // Append each character to gpsData string

    // Check if we have a complete NMEA sentence
    if (gpsData.indexOf("$GPGGA") != -1) {
      // Look for a valid GPGGA sentence
      parseGPGGA(gpsData);
      gpsData = "";  // Clear gpsData after parsing
    }
  }
}

// Function to parse the GPGGA sentence and extract latitude and longitude
void parseGPGGA(String sentence) {
  int latIndex = sentence.indexOf(",") + 1;  // Skip first comma
  int latEnd = sentence.indexOf(",", latIndex);  // Find next comma
  String latitude = sentence.substring(latIndex, latEnd);

  int lonIndex = sentence.indexOf(",", latEnd + 1) + 1;
  int lonEnd = sentence.indexOf(",", lonIndex);
  String longitude = sentence.substring(lonIndex, lonEnd);

  // Convert latitude and longitude to numeric values
  float lat = latitude.toFloat();
  float lon = longitude.toFloat();

  // Print the latitude and longitude
  Serial.print("Latitude: ");
  Serial.println(lat, 6);  // 6 decimal places
  Serial.print("Longitude: ");
  Serial.println(lon, 6);  // 6 decimal places
}
