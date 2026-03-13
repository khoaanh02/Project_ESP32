
#include<WiFi.h>
#include <Arduino.h>

const char* ssid = "khoa";
const char* password = "khoaanh14";

void wifi();

void setup() {
  Serial.begin(9600);
  delay(1000);
  wifi();
  
}

void loop() {
  
}

void wifi(){
  Serial.println("Dang ket noi WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nDa ket noi WiFi!");
}