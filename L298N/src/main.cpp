#include <Arduino.h>

#define in1 15
#define in2 2  
#define in3 5
#define in4 18
#define ENA 32
#define ENB 33
#define STBY 19

#define freq 5000
#define kenh0 0
#define kenh1 1
#define reslution 8


void chaytien();
void chaylui();
void phai();
void trai();

void setup() {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
   pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  ledcSetup(kenh0, freq, reslution);
  ledcSetup(kenh1, freq, reslution);
  ledcAttachPin(ENA, kenh0);
  ledcAttachPin(ENB, kenh1);

 
}

void loop() {
  chaytien();
  // delay(1500); 

  // phai();
  // delay(500);

  // chaytien();
  // delay(1500); 

  // trai();
  // delay(500);
}

void chaytien(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
   digitalWrite(in3, HIGH);
   digitalWrite(in4, LOW);
  ledcWrite(kenh0, 256);
  ledcWrite(kenh1, 256);
}

void phai(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  ledcWrite(kenh0, 0);   
  ledcWrite(kenh1, 128);
}

void trai(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  ledcWrite(kenh0, 128);
  ledcWrite(kenh1, 0);
}