#include <Arduino.h>

int digital = 14;

void setup() {
    Serial.begin(9600);
    delay(1000);
    pinMode(digital, INPUT);
}

void loop(){
    Serial.print("Doc tin hieu = ");
    int tinhieu = digitalRead(digital);
    Serial.println(tinhieu);
    delay(1000);
}
