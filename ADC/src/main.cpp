#include <Arduino.h>

void setup() {

  Serial.begin(9600);
  pinMode(15, OUTPUT);

}

void loop() {
  int value = analogRead(15);
  Serial.println(value);
  delay(1000);

}

/*
#define VR 34

void setup(){
  Serial.begin(115200);
}
void loop(){
  int value = analogRead(VR);
  float v = 3.3 / 4095 * value;
  Serial.begin(v);
  delay(500);
}

*/