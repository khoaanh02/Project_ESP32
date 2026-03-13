
#include <Arduino.h>
#include<Wire.h>
#include<SimpleKalmanFilter.h>

#define PI 3.14159265359f
#define SDA_PIN  21
#define SCL_PIN  22

SimpleKalmanFilter LocNhieuRoll(0.05, 0.05, 0.015);
SimpleKalmanFilter LocNhieuPitch(0.05, 0.05, 0.025);
SimpleKalmanFilter LocNhieuGyro_Roll(0.03, 0.05, 0.008);
SimpleKalmanFilter LocNhieuGyro_Pitch(0.03, 0.05, 0.008);

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

void gyro_signals(void){
  // Init Accelerometer ±8g
  Wire.beginTransmission(mpu_address);
  Wire.write(0x1C); // địa chỉ thanh ghi
  Wire.write(0x10); // full scale là 2 vì AFS_SEL[1:0] chuyển sang 16 bit
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
  Wire.write(0x08); // full scale = 1
  Wire.endTransmission();

  // Đọc thanh ghi Gyroscope Measurements
  Wire.beginTransmission(mpu_address);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6, 1);

  int16_t GyroX = Wire.read() << 8 | Wire.read();
  int16_t GyroY = Wire.read() << 8 | Wire.read();
  int16_t GyroZ = Wire.read() << 8 | Wire.read();

  // Gia tốc kế: Xác định hướng so với trọng lực
  AccX = (float)AccXLSB / 4096.0 - offset_AccX;
  AccY = (float)AccYLSB / 4096.0 - offset_AccY;
  AccZ = (float)AccZLSB / 4096.0 - offset_AccZ;

  // Con quay hồi chuyển: Phát hiện chuyển động quay
  Rate_Roll = (float)GyroX / 65.5 - offset_GyroX;
  Rate_Pitch = (float)GyroY / 65.5 - offset_GyroY;
  Rate_Yaw = (float)GyroZ / 65.5 - offset_GyroZ;

  AngleRoll = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * (180.0 / PI);
  AnglePitch = atan(-AccX / sqrt(AccY * AccY + AccZ * AccZ)) * (180.0 / PI);

  // Lọc Kalman
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
  // Init Accelerometer ±8g
  Wire.beginTransmission(mpu_address);
  Wire.write(0x1C); // địa chỉ thanh ghi
  Wire.write(0x10); // full scale là 2 vì AFS_SEL[1:0] chuyển sang 16 bit
  Wire.endTransmission();

  Wire.beginTransmission(mpu_address);
  Wire.write(0x1B);
  Wire.write(0x08); // full scale = 1
  Wire.endTransmission();

  float sum_AccX = 0, sum_AccY = 0, sum_AccZ = 0;
  float sum_GyroX = 0, sum_GyroY = 0, sum_GyroZ = 0;

  for(int i = 0; i < 1000; i++){
    Wire.beginTransmission(mpu_address);
    Wire.write(0x3B);
    Wire.endTransmission();
    Wire.requestFrom(0x68, 14, 1);

    // Đọc Acc
    int16_t AccXLSB = Wire.read() << 8 | Wire.read();
    int16_t AccYLSB = Wire.read() << 8 | Wire.read();
    int16_t AccZLSB = Wire.read() << 8 | Wire.read();

    Wire.read();
    Wire.read();

    // Đọc Gyro
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


void setup(){
  Serial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);
  delay(250);
  
  // Wake up MPU
  Wire.beginTransmission(mpu_address);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  Check_offset();
  last_time = micros();

  
}
void loop(){
  now_time = micros();
  dt = (now_time - last_time) / 1000000.0;
  last_time = now_time;

  gyro_signals();
  // Serial.print("Rate_Pitch: "); Serial.print(Rate_Pitch, 3);
  // Serial.print("  | E_GyroRoll: "); Serial.print(estimate_GyroRoll, 3);
  // Serial.print("  | E_GyroPitch "); Serial.println(estimate_GyroPitch, 3);

  Serial.print("AngleRoll: "); Serial.print(AngleRoll, 2);        // Giá trị thô
  Serial.print(" | Filtered: "); Serial.print(estimate_GyroRoll, 2); // Đã lọc

  Serial.print("  AnglePitch: "); Serial.print(AnglePitch, 2);        // Giá trị thô
  Serial.print(" | Filtered: "); Serial.println(estimate_GyroPitch, 2); // Đã lọc
  // Serial.print("  AccX:"); Serial.print(AccX, 3);
  // Serial.print("  AccY:"); Serial.print(AccY, 3);
  // Serial.print("  AccZ:"); Serial.println(AccZ, 3);
  
  // Serial.print("  Roll:"); Serial.print(Rate_Roll, 3);
  // Serial.print("  Pitch:"); Serial.print(Rate_Pitch, 3);
  // Serial.print("  Yaw:"); Serial.println(Rate_Yaw, 3);
}
