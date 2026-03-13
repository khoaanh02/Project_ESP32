#include <Arduino.h>
#include<Wire.h>
#include<BH1750.h>

/*
BH1750 cambien(0x23);

void setup(){
  Serial.begin(9600);
  Wire.begin(21, 22);
  if (cambien.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }
}


void loop(){
  float value = cambien.readLightLevel();

  Serial.print("Light: ");
  Serial.print(value);
  Serial.println(" lux");

  delay(1000);
}

*/

/**
 * BMP280 Driver cho ESP32 - Sử dụng thanh ghi trực tiếp (Register-level)
 * Giao tiếp: I2C
 * SDA: GPIO 21 | SCL: GPIO 22
 * Địa chỉ I2C: 0x76 (SDO = GND) hoặc 0x77 (SDO = VCC)
 */

#include <Wire.h>

// ============================================================
//  ĐỊA CHỈ & THANH GHI BMP280
// ============================================================
#define BMP280_ADDR       0x76   // Thay 0x77 nếu SDO nối VCC

// -- Thanh ghi ID & Reset --
#define REG_ID            0xD0   // Chip ID → luôn = 0x60
#define REG_RESET         0xE0   // Ghi 0xB6 để soft-reset

// -- Thanh ghi cấu hình --
#define REG_CTRL_MEAS     0xF4   // osrs_t[7:5] | osrs_p[4:2] | mode[1:0]
#define REG_CONFIG        0xF5   // t_sb[7:5] | filter[4:2] | spi3w_en[0]
#define REG_STATUS        0xF3   // measuring[3] | im_update[0]

// -- Thanh ghi dữ liệu thô --
#define REG_PRESS_MSB     0xF7   // 0xF7..0xF9 → áp suất 20-bit
#define REG_TEMP_MSB      0xFA   // 0xFA..0xFC → nhiệt độ 20-bit

// -- Thanh ghi hiệu chỉnh (Calibration) --
#define REG_CALIB_START   0x88   // 0x88..0x9F → 24 byte calib

// -- Chế độ hoạt động (mode[1:0]) --
#define MODE_SLEEP        0x00
#define MODE_FORCED       0x01   // Đo 1 lần rồi ngủ
#define MODE_NORMAL       0x03   // Đo liên tục

// -- Oversampling nhiệt độ (osrs_t[7:5]) --
#define OSRS_T_SKIP       (0x00 << 5)
#define OSRS_T_x1         (0x01 << 5)
#define OSRS_T_x2         (0x02 << 5)
#define OSRS_T_x4         (0x03 << 5)
#define OSRS_T_x8         (0x04 << 5)
#define OSRS_T_x16        (0x05 << 5)

// -- Oversampling áp suất (osrs_p[4:2]) --
#define OSRS_P_SKIP       (0x00 << 2)
#define OSRS_P_x1         (0x01 << 2)
#define OSRS_P_x2         (0x02 << 2)
#define OSRS_P_x4         (0x03 << 2)
#define OSRS_P_x8         (0x04 << 2)
#define OSRS_P_x16        (0x05 << 2)

// -- IIR Filter (filter[4:2]) --
#define FILTER_OFF        (0x00 << 2)
#define FILTER_2          (0x01 << 2)
#define FILTER_4          (0x02 << 2)
#define FILTER_8          (0x03 << 2)
#define FILTER_16         (0x04 << 2)

// -- Standby time ở Normal mode (t_sb[7:5]) --
#define T_SB_0_5ms        (0x00 << 5)
#define T_SB_62_5ms       (0x01 << 5)
#define T_SB_125ms        (0x02 << 5)
#define T_SB_250ms        (0x03 << 5)
#define T_SB_500ms        (0x04 << 5)
#define T_SB_1000ms       (0x05 << 5)

// ============================================================
//  BIẾN LƯU HỆ SỐ HIỆU CHỈNH (từ OTP của cảm biến)
// ============================================================
struct BMP280_Calib {
  uint16_t dig_T1;
  int16_t  dig_T2, dig_T3;
  uint16_t dig_P1;
  int16_t  dig_P2, dig_P3, dig_P4, dig_P5;
  int16_t  dig_P6, dig_P7, dig_P8, dig_P9;
} calib;

int32_t t_fine;   // biến chia sẻ giữa tính nhiệt độ và áp suất

// ============================================================
//  HÀM ĐỌC / GHI THANH GHI QUA I2C
// ============================================================

/** Ghi 1 byte vào thanh ghi */
void bmp280_write(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(BMP280_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

/** Đọc 1 byte từ thanh ghi */
uint8_t bmp280_read8(uint8_t reg) {
  Wire.beginTransmission(BMP280_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);           // restart (không stop)
  Wire.requestFrom(BMP280_ADDR, 1);
  return Wire.available() ? Wire.read() : 0;
}

/** Đọc nhiều byte liên tiếp bắt đầu từ reg */
void bmp280_readBytes(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(BMP280_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(BMP280_ADDR, (int)len);
  for (uint8_t i = 0; i < len && Wire.available(); i++) {
    buf[i] = Wire.read();
  }
}

// ============================================================
//  ĐỌC HỆ SỐ HIỆU CHỈNH
// ============================================================
void bmp280_readCalibration() {
  uint8_t raw[24];
  bmp280_readBytes(REG_CALIB_START, raw, 24);

  // Nhiệt độ
  calib.dig_T1 = (uint16_t)(raw[1] << 8 | raw[0]);
  calib.dig_T2 = (int16_t) (raw[3] << 8 | raw[2]);
  calib.dig_T3 = (int16_t) (raw[5] << 8 | raw[4]);

  // Áp suất
  calib.dig_P1 = (uint16_t)(raw[7]  << 8 | raw[6]);
  calib.dig_P2 = (int16_t) (raw[9]  << 8 | raw[8]);
  calib.dig_P3 = (int16_t) (raw[11] << 8 | raw[10]);
  calib.dig_P4 = (int16_t) (raw[13] << 8 | raw[12]);
  calib.dig_P5 = (int16_t) (raw[15] << 8 | raw[14]);
  calib.dig_P6 = (int16_t) (raw[17] << 8 | raw[16]);
  calib.dig_P7 = (int16_t) (raw[19] << 8 | raw[18]);
  calib.dig_P8 = (int16_t) (raw[21] << 8 | raw[20]);
  calib.dig_P9 = (int16_t) (raw[23] << 8 | raw[22]);
}

// ============================================================
//  KHỞI TẠO BMP280
// ============================================================
bool bmp280_init() {
  Wire.begin();

  // Kiểm tra Chip ID
  uint8_t id = bmp280_read8(REG_ID);
  if (id != 0x58 && id != 0x60) {
    Serial.printf("[BMP280] Chip ID không hợp lệ: 0x%02X\n", id);
    return false;
  }
  Serial.printf("[BMP280] Chip ID: 0x%02X ✓ (%s)\n", id,
                id == 0x60 ? "BME280" : "BMP280");

  // Soft reset
  bmp280_write(REG_RESET, 0xB6);
  delay(10);

  // Đọc hệ số hiệu chỉnh từ OTP
  bmp280_readCalibration();

  // Cấu hình:
  //   t_sb = 500ms | filter = 4 | spi3w_en = 0
  bmp280_write(REG_CONFIG, T_SB_500ms | FILTER_4);

  //   osrs_t = x2 | osrs_p = x4 | mode = Normal
  bmp280_write(REG_CTRL_MEAS, OSRS_T_x2 | OSRS_P_x4 | MODE_NORMAL);

  return true;
}

// ============================================================
//  TÍNH NHIỆT ĐỘ (°C × 100, ví dụ 2530 = 25.30°C)
//  Công thức bù từ datasheet Bosch BMP280
// ============================================================
int32_t bmp280_compensate_temp(int32_t adc_T) {
  int32_t var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1)))
                  * ((int32_t)calib.dig_T2)) >> 11;

  int32_t var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1))
                  * ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12)
                  * ((int32_t)calib.dig_T3)) >> 14;

  t_fine = var1 + var2;
  return (t_fine * 5 + 128) >> 8;  // đơn vị: 1/100 °C
}

// ============================================================
//  TÍNH ÁP SUẤT (Pa, Q24.8 fixed-point → chia 256 → Pa)
// ============================================================
uint32_t bmp280_compensate_pressure(int32_t adc_P) {
  int64_t var1 = ((int64_t)t_fine) - 128000;
  int64_t var2 = var1 * var1 * (int64_t)calib.dig_P6;
  var2 += (var1 * (int64_t)calib.dig_P5) << 17;
  var2 += ((int64_t)calib.dig_P4) << 35;
  var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8)
       + ((var1 * (int64_t)calib.dig_P2) << 12);
  var1 = (((int64_t)1 << 47) + var1) * ((int64_t)calib.dig_P1) >> 33;

  if (var1 == 0) return 0;  // tránh chia cho 0

  int64_t p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = ((int64_t)calib.dig_P9 * (p >> 13) * (p >> 13)) >> 25;
  var2 = ((int64_t)calib.dig_P8 * p) >> 19;
  p = ((p + var1 + var2) >> 8) + ((int64_t)calib.dig_P7 << 4);

  return (uint32_t)p;   // đơn vị: Pa × 256 (Q24.8)
}

// ============================================================
//  ĐỌC DỮ LIỆU RAW & TÍNH TOÁN
// ============================================================
void bmp280_read(float &temperature, float &pressure) {
  // Chờ cho đến khi không còn đo (bit 3 của STATUS = 0)
  while (bmp280_read8(REG_STATUS) & 0x08) {
    delay(1);
  }

  // Đọc 6 byte: 0xF7(press MSB) → 0xFC(temp XLSB)
  uint8_t raw[6];
  bmp280_readBytes(REG_PRESS_MSB, raw, 6);

  // Ghép dữ liệu thô 20-bit
  int32_t adc_P = ((int32_t)raw[0] << 12)
                | ((int32_t)raw[1] << 4)
                | ((int32_t)raw[2] >> 4);

  int32_t adc_T = ((int32_t)raw[3] << 12)
                | ((int32_t)raw[4] << 4)
                | ((int32_t)raw[5] >> 4);

  // Tính nhiệt độ (phải tính trước để có t_fine cho áp suất)
  int32_t T = bmp280_compensate_temp(adc_T);
  temperature = T / 100.0f;

  // Tính áp suất
  uint32_t P = bmp280_compensate_pressure(adc_P);
  pressure = (P / 256.0f) / 100.0f;   // Pa → hPa (mbar)
}

// ============================================================
//  TÍNH ĐỘ CAO XẤP XỈ (m) từ áp suất đo & áp suất mực biển
// ============================================================
float bmp280_altitude(float pressure_hPa, float seaLevel_hPa = 1013.25f) {
  return 44330.0f * (1.0f - pow(pressure_hPa / seaLevel_hPa, 0.1903f));
}

// ============================================================
//  SETUP & LOOP
// ============================================================
void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("=== BMP280 Register-Level Driver ===");

  if (!bmp280_init()) {
    Serial.println("[LỖII] Không tìm thấy BMP280! Kiểm tra kết nối.");
    while (true) delay(1000);
  }

  Serial.println("[OK] BMP280 đã sẵn sàng.\n");
}

void loop() {
  float temp, press;
  bmp280_read(temp, press);
  float alt = bmp280_altitude(press);

  Serial.printf("Nhiệt độ : %.2f °C\n", temp);
  Serial.printf("Áp suất  : %.2f hPa\n", press);
  Serial.printf("Độ cao   : %.1f m\n\n", alt);

  delay(2000);
}
