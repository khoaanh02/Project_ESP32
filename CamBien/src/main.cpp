#include <Arduino.h>

#define in1 15
#define in2 2
#define in3 5
#define in4 18

#define freq 5000
#define kenh0 0
#define kenh1 1
#define reslution 8

const int socambien = 5; 
int cambien[socambien] = {12, 14, 27, 26, 25};  
int value[socambien];           

void read_senior();
void tien();
void lui();
void phai();
void trai();

void setup() {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  ledcSetup(kenh0, freq, reslution);
  ledcSetup(kenh1, freq, reslution);

  ledcAttachPin(in1, kenh0);
  ledcAttachPin(in3, kenh1);
  ledcWrite(kenh0, 0);
  ledcWrite(kenh1, 0);
  delay(1);

  Serial.begin(9600);
  for(int i = 0; i < socambien; i++){
    pinMode(cambien[i], INPUT);
  }
  Serial.println("Doc du lieu tu cam bien:");
}

void loop() {
  read_senior();
}

void read_senior(){
  for(int i = 0; i < socambien; i++){
    value[i] = digitalRead(cambien[i]);
  }
  for(int i = 0; i < socambien; i++){
    Serial.print(value[i]);
    Serial.print("    |   ");
  }
  Serial.println();
  delay(100);

  if(value[1] == 1){
    ledcWrite(kenh0, 32);
    ledcWrite(kenh1, 32);
  }
  else{
    ledcWrite(kenh0, 0);
    ledcWrite(kenh1, 0);
  }
}
void tien(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  ledcWrite(kenh0, 64);
  ledcWrite(kenh1, 64);
}

