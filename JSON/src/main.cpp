#include <Arduino.h>
#include<SoftwareSerial.h>
#include<ArduinoJson.h>

#define rx_pin 5
#define tx_pin 18

SoftwareSerial cong_uart(rx_pin, tx_pin);

void setup(){
  Serial.begin(9600);
  cong_uart.begin(9600);  // mặc định là SERIAL_8N1 
}

void loop(){
  /*
  // Khởi tạo
  StaticJsonDocument<200> doc;  // độ dài 200
  doc["nhietdo"] = 26;
  doc["doam"] = 80;
  doc["khigass"] = 1000;
  doc["ten"] = "esp32";
  // gửi đi
  serializeJson(doc, cong_uart);  
  delay(1000);
  */

  // hàm nhận
  if(cong_uart.available()){
    StaticJsonDocument<300> doc;
    DeserializationError err = deserializeJson(doc, cong_uart);
    if(err == DeserializationError::Ok){
      Serial.print("nhiet do = ");
      Serial.println(doc["nhietdo"].as<int>());

      Serial.print("Do am = ");
      Serial.println(doc["doam"].as<int>());

      Serial.print("khigass = ");
      Serial.println(doc["khigass"].as<int>());

      Serial.print("Ten: ");
      Serial.println(doc["ten"].as<String>());
    }
    else{
      Serial.print("return");
      Serial.println(err.c_str());
      while(cong_uart.available() > 0){  // bé hơn 0 thì nó gửi hết lên và hiển thị
        cong_uart.read();
      }
    }
  }
}