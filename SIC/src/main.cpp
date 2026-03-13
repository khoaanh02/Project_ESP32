#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6mhhBSuv6"
#define BLYNK_TEMPLATE_NAME "SIC"
#define BLYNK_AUTH_TOKEN "jdxwS8baIFs-TGJvyHLtb-P4ZT7G0JRX"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WiFi credentials
char ssid[] = "UTC-Office";
char pass[] = "";
const char *auth = "jdxwS8baIFs-TGJvyHLtb-P4ZT7G0JRX";

// Sensor pins
const int photoresistorPin = 34;  // Analog
const int lightBlockPin = 35;     // Digital
const int ballSwitchPin = 32;     // Digital
const int hallSensorPin = 33;     // Digital
const int soilMoisturePin = 36;   // Analog
const int buzzerPin = 25;         // Digital output

// Thresholds for landslide detection
const int soilMoistureThreshold = 300;  // Adjust based on calibration
bool landslideDetected = false;

BlynkTimer timer;

void sendSensorData();
void wifi();

void setup() {
  Serial.begin(9600);
  pinMode(lightBlockPin, INPUT);
  pinMode(ballSwitchPin, INPUT_PULLDOWN);  // Assuming normally open switch
  pinMode(hallSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  
  wifi();

  timer.setInterval(1000L, sendSensorData);
}

void sendSensorData() {
  int photoresistorValue = analogRead(photoresistorPin);
  int lightBlockValue = digitalRead(lightBlockPin);
  int ballSwitchValue = digitalRead(ballSwitchPin);
  int hallSensorValue = digitalRead(hallSensorPin);
  int soilMoistureValue = analogRead(soilMoisturePin);

  Blynk.virtualWrite(V1, photoresistorValue);
  Blynk.virtualWrite(V2, lightBlockValue);
  Blynk.virtualWrite(V3, ballSwitchValue);
  Blynk.virtualWrite(V4, hallSensorValue);
  Blynk.virtualWrite(V5, soilMoistureValue);

  if ((ballSwitchValue == LOW || hallSensorValue == HIGH) && soilMoistureValue < soilMoistureThreshold) {
    if (!landslideDetected) {
      landslideDetected = true;
      Blynk.logEvent("landslide_alert", "Landslide detected! Warning!");
      digitalWrite(buzzerPin, HIGH);  
    }
  } else {
    landslideDetected = false;
    digitalWrite(buzzerPin, LOW);  
  }

  Serial.printf("Photoresistor: %d, LightBlock: %d, BallSwitch: %d, HallSensor: %d, SoilMoisture: %d\n", photoresistorValue, lightBlockValue, ballSwitchValue, hallSensorValue, soilMoistureValue);
}

void loop() {
  Blynk.run();
  timer.run();
}

void wifi(){
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected!");
  Blynk.begin(auth, ssid, pass);
}
