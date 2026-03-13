#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  while (!Serial); // Chờ kết nối Serial (chỉ cần cho ESP32-S2/S3)

  if (!mlx.begin()) {
    Serial.println("Không tìm thấy cảm biến MLX90614!");
    while (1);
  }
  
  Serial.println("Cảm biến MLX90614 đã sẵn sàng");
  delay(500);
}

void loop() {
  // Đọc nhiệt độ đối tượng (Object) - không tiếp xúc
  float tempObject = mlx.readObjectTempC();
  
  // Đọc nhiệt độ môi trường (Ambient)
  float tempAmbient = mlx.readAmbientTempC();

  Serial.print("Nhiệt độ vật thể: "); Serial.print(tempObject); Serial.println("°C");
  Serial.print("Nhiệt độ môi trường: "); Serial.print(tempAmbient); Serial.println("°C");
  Serial.println("------------------------");

  delay(500);// Đọc mỗi giây
}