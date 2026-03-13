#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6Sr5D1H_9"
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "-gGhW1gqIHhy71Km0vELc54OFICz3z3d"

#define relay1 27
#define relay2 23
#define button1 32
#define button2 35
#define pin_rx 2
#define pin_tx 4 

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include<HardwareSerial.h>

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
BlynkTimer timer;

// dùng uart giao tiếp cho Module Sim
HardwareSerial sim_uart1(1);

const char *ssid = "P.204";
const char *pass = "23456789";
const char *auth = "-gGhW1gqIHhy71Km0vELc54OFICz3z3d";

unsigned long now_time = millis();
unsigned long last_time = 0;


bool flag1 = 0, flag2 = 0;

void wifi();
void IRAM_ATTR nutnhan1();
void IRAM_ATTR nutnhan2();

BLYNK_WRITE(V0){
  flag1 = param.asInt();
}
BLYNK_WRITE(V1){
  flag2 = param.asInt();
}

void checkbutton(){
  Blynk.virtualWrite(V0, flag1);
  Blynk.virtualWrite(V1, flag2);
}

void setup(){
  Serial.begin(9600);
  wifi();

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  attachInterrupt(button1, nutnhan1, FALLING);
  attachInterrupt(button2, nutnhan2, FALLING);

  // dùng timer trong blynk để đẩy dữ liệu lên app chính xác. Nếu để trong hàm loop thì đẩy lên không như mong muốn
  timer.setInterval(200L, checkbutton);

  lcd.init();
  lcd.backlight();

  sim_uart1.begin(9600, SERIAL_8N1, pin_rx, pin_tx);
  delay(500);
  
  sim_uart1.println("AT");
  sim_uart1.println("AT + CMGF = 1");
  sim_uart1.println("AT + CNMI = 1, 2, 0, 0, 0");
}

void loop(){
  Blynk.run();
  digitalWrite(relay1, flag1);
  digitalWrite(relay2, flag2);

}

void wifi(){
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    lcd.setCursor(2, 0);
    lcd.print(".");
  }
  lcd.setCursor(2, 0);
  lcd.print("WiFi connected!");
  Blynk.begin(auth, ssid, pass);
}

void IRAM_ATTR nutnhan1(){
  if(now_time - last_time > 50){ // giữ ổn định lớn hơn 50
    flag1 != flag1;
    last_time = millis();
  }
}
void IRAM_ATTR nutnhan2(){
  if(now_time - last_time > 50){
    flag2 != flag2;
    last_time = millis();
  }
}