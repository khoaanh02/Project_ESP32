#include<Arduino.h>
#include<Wire.h>
// Gia tốc (hướng, trọng lực) getAccX
// Tốc độ quay getGyroX
// Góc từ gia tốc getAccAngleX
// Góc từ gyro => getgyroAngleX
// Góc cuối cùng đã lọc getAngleX

// Nhiet do
#define TEMP_OUT_H 0x41
#define TEMP_OUT_L 0x42
// Gia toc tuyen tinh (o/s) => đo rung, phát hiện nghiêng, đo chuyển động
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
// Con quay (đo tốc độ quay)
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48

const int mpu = 0x68 ;
float dt = 0.015;

float gyx = 0, gyy = 0, gyz = 0, Gyro_X = 0.0, Gyro_Y = 0.0, Gyro_Z = 0.0;
float sum_X = 0, sum_Y = 0, sum_Z = 0;
float fsum_X = 0, fsum_Y = 0, fsum_Z = 0;

float accx = 0.0, accy = 0.0, accz = 0.0, Acc_X = 0.0, Acc_Y = 0.0, Acc_Z = 0.0;

unsigned long now_time = 0;
unsigned long last_time = 0;
bool check_BoLoc = false;

void MPU6050_Init();
void check_locnhieu();

void setup(){
    Serial.begin(9600);
    MPU6050_Init();
    
}

void loop(){
    if(check_BoLoc == false){
        check_locnhieu();
        check_BoLoc = true;
        last_time = millis();
    }

    now_time = millis();
    dt =  (now_time - last_time) / 1000.0;
    last_time = now_time;

    Wire.beginTransmission(mpu);// khoi tao doc I2C MPU
    Wire.write(ACCEL_XOUT_H); // bat dau viet vao thanh ghi GYRO_XOUT_H
    Wire.endTransmission(false);
    Wire.requestFrom(mpu, 14, 1); // cho phep mpu truyen den ESP 1:"True"

    // ACCEL
    accx = (uint16_t)(uint16_t)(Wire.read() << 8 | Wire.read());
    accy = (uint16_t)(uint16_t)(Wire.read() << 8 | Wire.read());
    accz = (uint16_t)(uint16_t)(Wire.read() << 8 | Wire.read());

    // bỏ qua nhiệt độ
    Wire.read();
    Wire.read();

    //GYRO
    gyx = (uint16_t)(Wire.read() << 8 | Wire.read()); // truyen 8 bit High va 8 bit Low de ra gia toc, grx/131 = o/s, de * voi tgian t ra goc xoay
    gyy = (uint16_t)(Wire.read() << 8 | Wire.read());
    gyz = (uint16_t)(Wire.read() << 8 | Wire.read());
    
    Acc_X = accx / 16384.0;
    Acc_Y = accy / 16384.0;
    Acc_Z = accz / 16384.0;

    Gyro_X = gyx / 131.0;
    Gyro_Y = gyy / 131.0;
    Gyro_Z = gyz / 131.0;

    sum_X += (Gyro_X - fsum_X) * dt;
    sum_Y += (Gyro_Y - fsum_Y) * dt;
    sum_Z += (Gyro_Z - fsum_Z) * dt;

    // Serial.print(" X:"); Serial.print(sum_X, 2);
    // Serial.print(" Y:"); Serial.print(sum_Y, 2);
    // Serial.print(" Z:"); Serial.println(sum_Z, 2);
    
    
}

void MPU6050_Init(){
    Wire.begin();
    Wire.beginTransmission(mpu);
    Wire.write(0x6B); 
    Wire.write(0x00);
    Wire.endTransmission();
}
void check_locnhieu(){
    // Đo cảm biến đứng im, rồi cộng tổng chia trung bình 1000 lần cho hết nhiễu
    for(int i = 0; i < 1000; i++){
        Wire.beginTransmission(mpu);
        Wire.write(ACCEL_XOUT_H); 
        Wire.endTransmission(false);
        Wire.requestFrom(mpu, 6, 1); // 1:"True"
        gyx = (Wire.read() << 8 | Wire.read()) / 131.0; 
        gyy = (Wire.read() << 8 | Wire.read()) / 131.0;
        gyz = (Wire.read() << 8 | Wire.read()) / 131.0;
        fsum_X += gyx;
        fsum_Y += gyy;
        fsum_Z += gyz;
        delay(2);
    }
    fsum_X = fsum_X / 1000;
    fsum_Y = fsum_Y / 1000;
    fsum_Z = fsum_Z / 1000;

    Serial.print(" X:"); Serial.print(fsum_X);
    Serial.print(" Y:"); Serial.print(fsum_Y);
    Serial.print(" Z:"); Serial.println(fsum_Z);
}