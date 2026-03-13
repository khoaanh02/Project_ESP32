
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6Sr5D1H_9"
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "-gGhW1gqIHhy71Km0vELc54OFICz3z3d"

#include <Arduino.h>
#include<Wire.h>
#include<SimpleKalmanFilter.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define PI 3.14159265359f
#define SDA_PIN  21
#define SCL_PIN  22

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

const char *ssid = "Vu Bon";
const char *pass = "hoilamgi";
const char *auth = "-gGhW1gqIHhy71Km0vELc54OFICz3z3d";

SimpleKalmanFilter LocNhieuRoll(0.005, 0.005, 0.009);
SimpleKalmanFilter LocNhieuPitch(0.005, 0.005, 0.01);
SimpleKalmanFilter LocNhieuGyro_Roll(0.05, 0.05, 0.001);
SimpleKalmanFilter LocNhieuGyro_Pitch(0.05, 0.05, 0.001);

const int mpu_address = 0x68 ;

float Rate_Roll, Rate_Pitch, Rate_Yaw;
float AccX, AccY, AccZ;
float offset_AccX, offset_AccY, offset_AccZ;
float offset_GyroX, offset_GyroY, offset_GyroZ;

float AngleRoll, AnglePitch;
float estimate_AngleRoll, estimate_AnglePitch, estimate_AngleRoll1;

float GyroRoll, GyroPitch;
unsigned long now_time = 0;
unsigned long last_time = 0;
float dt = 0.0;
float estimate_GyroRoll, estimate_GyroPitch;

bool firstRun = true;

void Filter_kalman();
void Check_offset();
void start();
void tien(int left, int right);
void lui(int left, int right);
void stop();

float kp = 12.01; 
float ki = 2.0;
float kd = 0.256;

float setpoit = 0.0; 
float read_value = 0.0;

float error = 0;
float prev_error = 0.0;
unsigned long current_time_error = 0;
float daoham = 0, tichphan = 0, output = 0;
unsigned long now_time_pid = 0, last_time_pid = 0;
float output_pid = 0;

int tocdo = 250;     
int tocdo_max = 255;  
const float max_angle = 60.0;
bool check = false;

BLYNK_WRITE(V0) {
  kp = param.asFloat();
}
BLYNK_WRITE(V1) {
  ki = param.asFloat();
}
BLYNK_WRITE(V2) {
  kd = param.asFloat();
}

void gyro_signals(void){
  // Init Accelerometer ±8g
  Wire.beginTransmission(mpu_address);
  Wire.write(0x1C);
  Wire.write(0x10);
  Wire.endTransmission();

  // Đọc thanh ghi gia tốc kế Accelerometer Measurements
  Wire.beginTransmission(mpu_address);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6, 1);
  int16_t AccXLSB = Wire.read() << 8 | Wire.read();
  int16_t AccYLSB = Wire.read() << 8 | Wire.read();
  int16_t AccZLSB = Wire.read() << 8 | Wire.read();

  // Init Gyroscope ±500°/s
  Wire.beginTransmission(mpu_address);
  Wire.write(0x1B);
  Wire.write(0x08);
  Wire.endTransmission();

  // Đọc thanh ghi Gyroscope Measurements
  Wire.beginTransmission(mpu_address);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6, 1);

  int16_t GyroX = Wire.read() << 8 | Wire.read();
  int16_t GyroY = Wire.read() << 8 | Wire.read();
  int16_t GyroZ = Wire.read() << 8 | Wire.read();

  AccX = (float)AccXLSB / 4096.0 - offset_AccX;
  AccY = (float)AccYLSB / 4096.0 - offset_AccY;
  AccZ = (float)AccZLSB / 4096.0 - offset_AccZ;

  Rate_Roll = (float)GyroX / 65.5 - offset_GyroX;
  Rate_Pitch = (float)GyroY / 65.5 - offset_GyroY;
  Rate_Yaw = (float)GyroZ / 65.5 - offset_GyroZ;

  AngleRoll = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * (180.0 / PI);
  AnglePitch = atan(-AccX / sqrt(AccY * AccY + AccZ * AccZ)) * (180.0 / PI);

  Filter_kalman();

}

void Filter_kalman(){
  estimate_AngleRoll = LocNhieuRoll.updateEstimate(AngleRoll);  
  estimate_AnglePitch = LocNhieuPitch.updateEstimate(AnglePitch);
  if (firstRun) {
    GyroRoll = estimate_AngleRoll;
    GyroPitch = estimate_AnglePitch;
    firstRun = false;
  }
  GyroRoll = 0.98 * (GyroRoll + Rate_Roll * dt) + 0.02 * estimate_AngleRoll;
  GyroPitch = 0.98 * (GyroPitch + Rate_Pitch * dt) + 0.02 * estimate_AnglePitch;

  estimate_GyroRoll = LocNhieuGyro_Roll.updateEstimate(GyroRoll);
  estimate_GyroPitch = LocNhieuGyro_Pitch.updateEstimate(GyroPitch);
}

void Check_offset(){
  Wire.beginTransmission(mpu_address);
  Wire.write(0x1C);
  Wire.write(0x10);
  Wire.endTransmission();

  Wire.beginTransmission(mpu_address);
  Wire.write(0x1B);
  Wire.write(0x08);
  Wire.endTransmission();

  float sum_AccX = 0, sum_AccY = 0, sum_AccZ = 0;
  float sum_GyroX = 0, sum_GyroY = 0, sum_GyroZ = 0;

  for(int i = 0; i < 1000; i++){
    Wire.beginTransmission(mpu_address);
    Wire.write(0x3B);
    Wire.endTransmission();
    Wire.requestFrom(0x68, 14, 1);

    int16_t AccXLSB = Wire.read() << 8 | Wire.read();
    int16_t AccYLSB = Wire.read() << 8 | Wire.read();
    int16_t AccZLSB = Wire.read() << 8 | Wire.read();

    Wire.read();
    Wire.read();

    int16_t GyroX = Wire.read() << 8 | Wire.read();
    int16_t GyroY = Wire.read() << 8 | Wire.read();
    int16_t GyroZ = Wire.read() << 8 | Wire.read();

    sum_AccX += AccXLSB / 4096.0;
    sum_AccY += AccYLSB / 4096.0;
    sum_AccZ += AccZLSB / 4096.0;

    sum_GyroX += GyroX / 65.5;
    sum_GyroY += GyroY / 65.5;
    sum_GyroZ += GyroZ / 65.5;

    delay(2);
  }
  offset_AccX = sum_AccX / 1000;
  offset_AccY = sum_AccY / 1000;
  offset_AccZ = sum_AccZ / 1000 - 1.0;

  offset_GyroX = sum_GyroX / 1000.0;
  offset_GyroY = sum_GyroY / 1000.0;
  offset_GyroZ = sum_GyroZ / 1000.0;
}

float controller_pid(float setvalue, float readvalue, float kp, float ki, float kd){
  now_time_pid = micros();
  current_time_error = (now_time_pid - last_time_pid) / 1000000.0;
  if(current_time_error <= 0){
    current_time_error = 0.001;
  }
  error = setvalue - readvalue;

  tichphan += (error * current_time_error);
  tichphan = constrain(tichphan, -300, 300);
  if(abs(error) < 0.5){
    tichphan = 0;
  }

  daoham = (error - prev_error) / current_time_error;
  daoham = constrain(daoham, -600, 600);
  output = (kp * error) + (ki * tichphan) + (kd * daoham);

  prev_error = error; 
  last_time_pid = now_time_pid;

  return output;
}

void setup(){
  Serial.begin(9600);
  Serial.println("Connecting to WiFi...");
  //Blynk.begin(auth, ssid, pass);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  stop();
  ledcSetup(kenh0, freq, resolution);
  ledcSetup(kenh1, freq, resolution);
  ledcAttachPin(ENA, kenh0);
  ledcAttachPin(ENB, kenh1);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  Wire.beginTransmission(mpu_address);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  Check_offset();
  last_time = micros();
  last_time_pid = micros();
}

void loop(){
  //Blynk.run();
  now_time = micros();
  dt = (now_time - last_time) / 1000000.0;
  
  last_time = now_time;

  gyro_signals();
  start();

  Serial.print("Roll:"); Serial.print(estimate_GyroRoll, 2);
  Serial.print(" | PID:"); Serial.println(output_pid, 1);
}

void start(){
  read_value = estimate_GyroRoll;
  if(abs(read_value) <= 0.3){ 
    read_value = 0;
  }
  output_pid = controller_pid(setpoit, read_value, kp, ki, kd);
  if(abs(read_value) > max_angle){
    stop();
    tichphan = 0;
    prev_error = 0;
    return;
  }
  int pwm = constrain(abs(output_pid), 30, tocdo_max);
  if(output_pid >= 0){
    tien(pwm, pwm);
  }
  else if(output_pid < 0){
    lui(pwm, pwm);
  }
  else{
    stop();
  }
}

void tien(int left, int right){
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
  digitalWrite(in1, HIGH); 
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); 
  digitalWrite(in4, HIGH);
}

void stop(){
  ledcWrite(kenh0, 0);  
  ledcWrite(kenh1, 0);
  digitalWrite(in1, LOW); 
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); 
  digitalWrite(in4, LOW);
}