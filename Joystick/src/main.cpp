#include <Arduino.h>

int truc_x = 34;
int truc_y = 35;
int nutnhan = 32;

void setup() {
  Serial.begin(9600);
  pinMode(truc_x, INPUT);
  pinMode(truc_y, INPUT);
  pinMode(nutnhan, INPUT_PULLUP);
}

void loop() {
  int x = analogRead(truc_x);
  int y = analogRead(truc_y);
  int nut = digitalRead(nutnhan);

  Serial.print("Gia tri x: ");
  Serial.print(x);
  Serial.print("  ||  ");
  Serial.print("Gia tri y: ");
  Serial.print(y);
  Serial.print("  ||  ");
  Serial.print("Gia tri nut nhan: ");
  Serial.print(nut);
  Serial.print("\n");
  delay(200);
}

