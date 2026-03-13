#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6Sr5D1H_9"
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "-gGhW1gqIHhy71Km0vELc54OFICz3z3d"

#include <Arduino.h>
#include<WiFi.h>
#include<BlynkSimpleEsp32.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

const char *ssid = "P.204";
const char *pass = "23456789";
const char *auth = "-gGhW1gqIHhy71Km0vELc54OFICz3z3d";

bool check_led = 0;
int check_dongco = 0;
int in1 = 13;
int in2 = 12;

void wifi();
void dieukhien(int chedo);

// Nhận dữ liệu

BLYNK_WRITE(V0){
  check_led = param.asInt();
  if(check_led == 0){
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Da tat den");
  }
  if(check_led == 1){
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Da bat den");
  }
}
BLYNK_WRITE(V1){
  check_dongco = param.asInt();
  switch(check_dongco){
    case 1:
      lcd.clear();
      lcd.setCursor(2, 1);
      lcd.print("Xe chay tien!");
      dieukhien(1);
      Serial.println("Xe chay tien!");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(3, 1);
      lcd.print("Xe chay lui!");
      dieukhien(2);
      Serial.println("Xe chay lui!");
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(3, 1);
      lcd.print("Xe dung!");
      dieukhien(3);
      Serial.println("Xe dung!");
      break;
    default:
      lcd.print("Che do khong hop le!");
      dieukhien(3); 
      break;
  }
}


void setup() {
  Serial.begin(9600);  
  lcd.init();
  lcd.backlight();
  wifi();
  pinMode(15, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
}

void loop() {
  Blynk.run();
  digitalWrite(15, check_led);
  // digitalWrite(13, check_dongco);
  // digitalWrite(12, check_dongco);
  Serial.println(check_led);
  // Serial.println(check_dongco);
   delay(500);
}

// đẩy dữ liệu lên Blynk
void wifi(){
  WiFi.begin(ssid, pass);
  lcd.setCursor(0, 0);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println(".");
    lcd.setCursor(0, 0);
    lcd.print("..");
  }
  Serial.println("Wifi connected!");
  lcd.print("Wifi connected!");
  delay(3000);
  lcd.clear();
  Blynk.begin(auth, ssid, pass);
}

void dieukhien(int chedo){
  switch(chedo){
    case 1:
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      break;
    case 2:
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      break;
    case 3:
    default:
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      break;
  }
}
