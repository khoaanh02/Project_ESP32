#include <Arduino.h>

#define led 15
#define button 4

unsigned long now_time = millis();
unsigned long last_time = 0;

bool check = 0;

void IRAM_ATTR batden(){
  digitalWrite(led, HIGH);
  check = 1;
  last_time = millis();
}
void thuchien_tatden(){
  now_time = millis();
  if(check && (now_time - last_time > 3000)){
    digitalWrite(led, LOW);
    check = 0;
  }
}

void setup() {
  pinMode(led, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  attachInterrupt(button, batden, RISING);
}

void loop() {
  thuchien_tatden();
}

