#include <Arduino.h>
#include<Wire.h>
#include<PubSubClient.h>
#include<WiFi.h>

#define tocken "D5VU2gJfc8oiFVGT9XaT"
#define led 2
const char *ssid = "Thuychuc 2";
const char *pass = "04010611";
const char *mqtt = "thingsboard.cloud";

WiFiClient esp;
PubSubClient client(esp);

unsigned long now_time = 0, last_time = 0;
bool check = 0;
void start_connectWifi(){
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");
}

void Reconnect_WiFi(){
  now_time = millis();
  if((WiFi.status() != WL_CONNECTED) && (now_time - last_time > 5000)){
    Serial.print("Dang ket noi lai WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    check = 1;
    last_time = now_time;
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.print("Ket noi lai thanh cong!");
    delay(500);
  }
}

void Reconect_ThingsBoard(){
  while(!client.connected()){
    Serial.print("Dang ket noi ThingsBoard...");
    delay(500);
    if(client.connect("esp32", tocken, NULL)){
      Serial.print("Ket noi ThingsBoard thanh cong!");
    }
    else{
      Serial.print("That bai...");
    }
  }
}

void handle_string(char *c, byte *b, unsigned int length){
  String message ="";
  for(int i = 0; i < length; i++){
    message += (char)b[i];
  }
  Serial.print(message);
  if(message.indexOf("\"method\":\"setValue\"") != -1){
    if (message.indexOf("\"params\":true") != -1) {
      digitalWrite(led, HIGH); // Bật LED
      Serial.println("-> Da BAT den!");
      client.publish("v1/devices/me/telemetry", "{\"led_status\": true}");
    }
    else if (message.indexOf("\"params\":false") != -1) {
      digitalWrite(led, LOW); // Tắt LED
      Serial.println("-> Da TAT den!");
      // Gửi phản hồi trạng thái ngược lại ThingsBoard (tùy chọn)
      client.publish("v1/devices/me/telemetry", "{\"led_status\": false}");
    }
  }
  String responseTopic = String(c);
  responseTopic.replace("request", "response");
  client.publish(responseTopic.c_str(), "{}");
}

void setup(){
  Serial.begin(9600);
  start_connectWifi();
  
  pinMode(led, OUTPUT);
  client.setServer(mqtt, 1883);
  client.setCallback(handle_string);
}
void loop(){
  if(WiFi.status() == WL_CONNECTED) {
    if(!client.connected()){
      Reconect_ThingsBoard();
    }
    client.loop();
  } 
  else {
    Reconnect_WiFi();
  }
}