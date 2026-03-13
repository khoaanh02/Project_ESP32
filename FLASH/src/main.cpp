#include <Arduino.h>
#include<EEPROM.h>

unsigned long now_time = millis();
unsigned long last_time = 0;
bool check1 = 0;
bool lastbutton = 1;
bool ledstate = 0;
int value = 0;

#define led 2
#define button 4

void setup(){
  Serial.begin(9600);
  EEPROM.begin(1);
  pinMode(button, OUTPUT);
  pinMode(led, OUTPUT);
  //pinMode(23, INPUT_PULLUP);
  ledstate = EEPROM.read(0);

}

int readbutton;

void loop() {
  now_time = millis();
  value = touchRead(button);
  Serial.println(value);
  delay(5);

  if(value < 10){
    readbutton = 0;
  }
  else{
    readbutton = 1;
  }
  //int readbutton = digitalRead(value);
   
  if(readbutton != lastbutton){
    last_time = millis();
    check1 = 1;
  }
  if((now_time - last_time) > 50){
    if(readbutton == LOW && lastbutton == 1){
      ledstate != ledstate;
      digitalWrite(led, ledstate);

      EEPROM.write(0, ledstate);
      EEPROM.commit();
    }
  }
  lastbutton = readbutton;

}

