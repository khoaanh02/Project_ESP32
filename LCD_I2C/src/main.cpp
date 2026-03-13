#include <Arduino.h>
#include<Wire.h>
#include<LiquidCrystal_I2C.h>
   
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

void display(char a);

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
}

void loop() {
  lcd.setCursor(6, 0);
  lcd.print("Hello");
  
}

