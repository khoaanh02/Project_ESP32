#include <DHT.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#define DHTPIN D4     // Chân kết nối với DHT11
#define DHTTYPE DHT11 // Chọn loại cảm biến là DHT11

// Thông tin Firebase

#define API_KEY "AIzaSyD33o7rwfGM4-7GO2-0-P91GjQlEDajCd8"
#define USER_EMAIL "minhkhoa2k5kiss@gmail.com"
#define USER_PASSWORD "A23102020a@"
#define DATABASE_URL "https://esp8266-fed1c-default-rtdb.asia-southeast1.firebasedatabase.app/"


FirebaseData firebaseData;
FirebaseAuth authentication;
FirebaseConfig config;
FirebaseData maybom;
DHT dht(DHTPIN, DHTTYPE);
void mottor_run(uint8_t xung , uint8_t in1 , uint8_t in2);
// Thay đổi thông tin mạng WiFi của bạn

const char* ssid = "Labee";     
const char* password = "labmitsu";  // Mật khẩu mạng WiFi

int led_blue = D0;
String data_led;
uint8_t pwm[3] = {D0 , D1 , D2};
void setup() {
  Serial.begin(115200); // Khởi động Serial với tốc độ 115200
  dht.begin();          // Khởi động cảm biến DHT11
  ketnoiwifi();
   pinMode(D0,OUTPUT);
  // Khởi tạo Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  authentication.user.email = USER_EMAIL; // Đã sửa lại
  authentication.user.password = USER_PASSWORD; // Đã sửa lại
  
  Firebase.begin(&config, &authentication);
  for(int i = 0 ; i < 3;i++)
  {
    pinMode(pwm[i] , OUTPUT);
  }
} 

void loop() {
  if (Firebase.getString(maybom, "/led")) {  // Đọc từ đường dẫn "/led"
        data_led = maybom.stringData();        // Lấy giá trị đọc được
    } else{
        Serial.println("Lỗi khi đọc Firebase: " + maybom.errorReason());
    }
    if(data_led == "1") mottor_run(200 , 1 , 0);
    if(data_led == "0") mottor_run(200 , 0 , 0);
  //delay(2000); // Chờ 2 giây giữa các lần đọc

  // Đọc độ ẩm và nhiệt độ
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Kiểm tra lỗi
  if (isnan(h) || isnan(t)) {
    Serial.println("Không thể đọc từ DHT sensor!");
    return;
  }

  // In kết quả
  Serial.print("Độ ẩm: ");
  Serial.print(h);
  Serial.print("%  Nhiệt độ: ");
  Serial.print(t);
  Serial.println("°C");

  // Gửi dữ liệu lên Firebase
  Firebase.setFloat(firebaseData, "/humidity", h);
  Firebase.setFloat(firebaseData, "/temperature", t);

  // Kiểm tra và in kết quả gửi lên Firebase
  if (firebaseData.dataType() == "float") {
    Serial.println("Gửi dữ liệu thành công!");
  } else {
    Serial.print("Lỗi: ");
    Serial.println(firebaseData.errorReason());
  }
}
void mottor_run(uint8_t xung , uint8_t in1 , uint8_t in2)
{
  analogWrite(pwm[0] , xung);
  digitalWrite(pwm[1] , in1);
  digitalWrite(pwm[2] , in2);
}
void ketnoiwifi() {
  Serial.println("Bắt đầu kết nối WiFi!");
  WiFi.begin(ssid, password);
  
  // Chờ kết nối
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Đã kết nối thành công
  Serial.println();
  Serial.println("Kết nối thành công");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP()); // In địa chỉ IP
}