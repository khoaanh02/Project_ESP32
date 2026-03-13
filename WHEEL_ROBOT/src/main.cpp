#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6Sr5D1H_9"
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "-gGhW1gqIHhy71Km0vELc54OFICz3z3d"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include<MPU6050_tockn.h>


// Thông tin WiFi và Blynk
const char *ssid = "P. 205";
const char *pass = "23456789";
const char *auth = "-gGhW1gqIHhy71Km0vELc54OFICz3z3d";

#define in1 15
#define in2 2 
#define in3 5
#define in4 18

#define freq 5000
#define kenh0 0
#define kenh1 1
#define resolution 8
#define ENA 32
#define ENB 33

const int socambien = 5; 
int cambien[socambien] = {12, 14, 27, 26, 25};  
int value[socambien];   

float vitricambien();
void chedo();
void chaytheo_line();
void stop();
void tien(int left, int right);
void phai(int left, int right);
void trai(int left, int right);
void lui(int left, int right);
void tien_thang(int time);
void quay_trai_90();
void quay_phai_90();
void quay_trai_45();
void quay_phai_45();

unsigned long now_timer = 0;
unsigned long last_timer = 0;

// Hệ số PID - thay đổi từ Blynk
float kp = 50; 
float ki = 20;
float kd = 10;

float error = 0;
float last_error = 0;
float time_error = 0;
float daoham = 0;
float tichphan = 0;
float output = 0;

int tocdo = 190;     
int tocdo_max = 210;  
int stage = 0;
bool auto_mode = true;  // true = chạy, false = dừng

// ===== BLYNK Virtual Pins =====
// V0 - Kp
// V1 - Ki  
// V2 - Kd
// V3 - Dừng/Chạy (0 = dừng, 1 = chạy)

// Nhận Kp từ Blynk
BLYNK_WRITE(V0) {
  kp = param.asFloat();
  Serial.print("Kp = ");
  Serial.println(kp);
}

// Nhận Ki từ Blynk
BLYNK_WRITE(V1) {
  ki = param.asFloat();
  Serial.print("Ki = ");
  Serial.println(ki);
}

// Nhận Kd từ Blynk
BLYNK_WRITE(V2) {
  kd = param.asFloat();
  Serial.print("Kd = ");
  Serial.println(kd);
}

// Nhận lệnh Dừng/Chạy từ Blynk
BLYNK_WRITE(V3) {
  auto_mode = param.asInt();
  if(auto_mode == 1) {
    Serial.println("XE CHẠY");
  } 
  else {
    Serial.println("XE DỪNG");
    stop();
  }
}

float controller_pid(float setvalue, float readvalue, float kp, float ki, float kd){
  now_timer = micros();
  time_error = (now_timer - last_timer) / 1000000.0; 
  if(time_error <= 0){
    time_error = 0.001;
  }
  error = setvalue - readvalue;
  
  tichphan += (error * time_error);
  tichphan = constrain(tichphan, -1000, 1000);
  if(error==0) tichphan=0;
  daoham = (error - last_error) / time_error;
  output = (kp * error) + (ki * tichphan) + (kd * daoham);

  last_error = error;
  last_timer = now_timer;
  return output;
}

void setup(){
  Serial.begin(9600); 
  
  // Kết nối WiFi và Blynk
  Serial.println("Connecting to WiFi...");
  Blynk.begin(auth, ssid, pass);
  
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  stop();
  delay(20);
  ledcSetup(kenh0, freq, resolution);
  ledcSetup(kenh1, freq, resolution);
  ledcAttachPin(ENA, kenh0);
  ledcAttachPin(ENB, kenh1);

  for(int i = 0; i < socambien; i++){
    pinMode(cambien[i], INPUT);
  }
  
  Serial.println("System Ready!");
}

unsigned long now = 0;
unsigned quet_line = 0;

void loop(){
  Blynk.run(); // Chạy Blynk
  
  now = micros();
  if(now - quet_line >= 5){
    quet_line = micros();
    for(int i = 0; i < socambien; i++){
      value[i] = digitalRead(cambien[i]);                   
    }
    
    if(auto_mode) {  // Chỉ chạy khi auto_mode = true
      chedo();
    }
  }
}

float vitricambien(){
  // Phát hiện giao lộ
  if(value[0] == 1 && value[1] == 1 && value[2] == 1 && value[3] == 1 && value[4] == 1){
    return 100.0; 
  }
  if(value[0] == 1 && value[1] == 1 && value[2] == 1 && value[3] == 1 && value[4] == 0){
    return 100.0;
  }
  if(value[0] == 0 && value[1] == 1 && value[2] == 1 && value[3] == 1 && value[4] == 1){
    return 100.0;
  }
  if(value[0] == 1 && value[1] == 1 && value[2] == 1 && value[3] == 0 && value[4] == 0){
    return 100.0;
  }
  if(value[0] == 0 && value[1] == 0 && value[2] == 1 && value[3] == 1 && value[4] == 1){
    return 100.0;
  }
  
  // Mất line
  if(value[0] == 0 && value[1] == 0 && value[2] == 0 && value[3] == 0 && value[4] == 0){
    return 200.0; 
  }
  
  // Các vị trí thông thường
  if(value[0] == 0 && value[1] == 0 && value[2] == 0 && value[3] == 0 && value[4] == 1){
    return -6.0;
  }
  if(value[0] == 0 && value[1] == 0 && value[2] == 0 && value[3] == 1 && value[4] == 1){
    return -3.0;
  }
  if(value[0] == 0 && value[1] == 0 && value[2] == 0 && value[3] == 1 && value[4] == 0){
    return -2.0;
  }
  if(value[0] == 0 && value[1] == 0 && value[2] == 1 && value[3] == 1 && value[4] == 0){
    return -1.0;
  }
  if(value[0] == 0 && value[1] == 0 && value[2] == 1 && value[3] == 0 && value[4] == 0){
    return 0.0;
  }
  if(value[0] == 0 && value[1] == 1 && value[2] == 1 && value[3] == 0 && value[4] == 0){
    return 1.0;
  }
  if(value[0] == 0 && value[1] == 1 && value[2] == 0 && value[3] == 0 && value[4] == 0){
    return 2.0;
  }
  if(value[0] == 1 && value[1] == 1 && value[2] == 0 && value[3] == 0 && value[4] == 0){
    return 3.0;
  }
  if(value[0] == 1 && value[1] == 0 && value[2] == 0 && value[3] == 0 && value[4] == 0){
    return 6.0;
  }
  
  return error; 
}

void chedo(){
  float vitri = vitricambien();
  switch (stage)
  {
  case 0:
    chaytheo_line();
    break;
    
  default:
    stop();
    break;
  }
}

float setpoit = 0.0;
float read_last_value = 0.0;

void chaytheo_line(){
  read_last_value = vitricambien();
  if(read_last_value == 100.0 || read_last_value == 200.0){
    return;
  }
  bool check = false;
  for(int i = 0; i < socambien; i++){
    if(value[i] == 1){
      check = true;
    }
  }
  if(check){ 
    float output_pid = controller_pid(setpoit, read_last_value, kp, ki, kd);
    int left = tocdo + output_pid;
    int right = tocdo - output_pid;
    left = constrain(left, 0, tocdo_max);
    right = constrain(right, 0, tocdo_max);
    tien(left, right);
  }
}

void stop(){
  digitalWrite(in1, LOW); 
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); 
  digitalWrite(in4, LOW);
}

void tien(int left, int right){
  ledcWrite(kenh0, left);
  ledcWrite(kenh1, right);
  digitalWrite(in1, HIGH); 
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); 
  digitalWrite(in4, LOW);
}

void phai(int left, int right){
  ledcWrite(kenh0, left);
  ledcWrite(kenh1, right);
  digitalWrite(in1, HIGH); 
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); 
  digitalWrite(in4, HIGH);
}

void trai(int left, int right){
  ledcWrite(kenh0, left);
  ledcWrite(kenh1, right);
  digitalWrite(in1, LOW); 
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); 
  digitalWrite(in4, LOW);
}

void lui(int left, int right){
  ledcWrite(kenh0, left);
  ledcWrite(kenh1, right);
  digitalWrite(in1, LOW); 
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); 
  digitalWrite(in4, HIGH);
}

void tien_thang(int time){
  tien(tocdo, tocdo);
  delay(time);
}

void quay_phai_90(){
  phai(200, 200);
  delay(300);
  stop();
  delay(100);
}

void quay_trai_90(){
  trai(200, 200);
  delay(300);
  stop();
  delay(100);
}

void quay_phai_45(){
  phai(200, 200);
  delay(150);
  stop();
  delay(100);
}

void quay_trai_45(){
  trai(200, 200);
  delay(150);
  stop();
  delay(100);
}
