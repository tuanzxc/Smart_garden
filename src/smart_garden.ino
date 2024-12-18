#include <DHT.h> // Cảm biến nhiệt độ https://github.com/Seeed-Studio/Grove_Temperature_And_Humidity_Sensor
#include <Servo.h> // https://docs.arduino.cc/libraries/servo/
#include <ChainableLED.h> // https://github.com/Seeed-Studio/Grove_Chainable_RGB_LED
#include <rgb_lcd.h> // https://github.com/Seeed-Studio/Grove_LCD_RGB_Backlight
#include <Wire.h> // https://docs.arduino.cc/language-reference/en/functions/communication/wire/
#include <BlynkGate.h> // https://github.com/makerlabvn/BlynkGate


// Lưu trữ thông tin kết nối mạch wifi với internet và cloud Blynk
#define BLYNK_TEMPLATE_ID "TMPL6qNeYMyeY"
#define BLYNK_TEMPLATE_NAME "Garden"
#define BLYNK_AUTH_TOKEN "zLFIusg2ccN-BIaMxcadO8LWNHtKDWPh"
char auth[] = "zLFIusg2ccN-BIaMxcadO8LWNHtKDWPh";
// Tên và mật khẩu mạng wifi
char ssid[] = "GalaxyJ89C42";
char pass[] = "";


// Khởi tạo các linh kiện
#define rel_fan 3 // Quạt gió (D3)
#define rel_pump 2 // Máy bơm (D2)

DHT dht(4, DHT11); // Cảm biến nhiệt độ (D4)
float temp = 0, temp1 = 0; // Giá trị nhiệt độ

Servo myservo; // Rèm cửa (Động cơ servo)
int pos = 0; // Góc quay của servo

ChainableLED leds1(5, 6, 1); // Đèn sáng (D5, D6)
ChainableLED heater(7, 8, 1); // Đèn sưởi (D7, D8)

rgb_lcd lcd; // Màn hình LCD 16x2
int light = 0; // Giá trị ánh sáng
float humid = 0; // Giá trị độ ẩm
int mode = 0; // Tạo chế độ tự động (1), chế độ thủ công - người dùng tự điều khiển (0)
int DetectPin, PinValue; // Lưu giá trị nhận được từ cloud
int arr[5] = {0, 0, 0, 0, 0}; // Lưu giá trị 0/1 (tắt/bật) của các linh kiện trong chế độ tự động
unsigned long lasttime = 0; //Dùng mạch wifi không dùng hàm delay(), nên dùng biến này thay thế


void setup() {
  /*
  Blynk.begin(auth, ssid, pass);
  Hàm này được dùng trong lần đầu tiên sử dụng để kết nối với internet và cloud qua I2C
  Nhưng do bị lỗi (khi dùng hàm này thì mạch bị treo) nên khi kết nối xong không dùng hàm này nữa.
  */
  Wire.begin(); // Khởi tạo kết nối I2C thay thế hàm trên từ lần chạy tiếp theo
  dht.begin(); // Khởi tạo cảm biến nhiệt độ
  pinMode(3, OUTPUT); // Khởi tạo quạt
  pinMode(2, OUTPUT); // Khởi tạo máy bơm
  myservo.attach(12); // Kết nối servo với chân D12
  leds1.setColorRGB(0, 0, 0, 0); // Reset đèn sáng
  heater.setColorRGB(0, 0, 0, 0); // Reset đèn sưởi
  lcd.begin(16, 2); // Khởi tạo màn hình lcd
}


void sendss() { // Hàm này dùng để gửi dữ liệu đến màn hình lcd và cloud
  humid = (analogRead(A2)*0.2); // đọc độ ẩm đất (A2)
  light = analogRead(A6); // đọc cảm biến ánh sáng (A6)
  temp = dht.readTemperature(); // đọc cảm biến nhiệt độ (D4)
  if (!isnan(temp)) {temp1 = temp;} 
  // Lấy giá trị temp1. Chỉ khi đọc cảm biến bình thường, temp1 = temp. Nếu bị lỗi, lấy giá trị temp1 trước đó.
  
  lcd.clear(); // Xóa màn hình lcd để hiển thị dữ liệu hiện tại
  // Gửi giá trị cảm biến lên màn hình LCD
  lcd.setCursor(0, 0); lcd.print("As:"); lcd.print(light);
  lcd.setCursor(6, 0); lcd.print(" Hm:"); lcd.print(humid); lcd.print("%");
  lcd.setCursor(0, 1); lcd.print("Tmp:"); lcd.print(temp1);
  lcd.setCursor(10, 1);
  if (mode == 0) {
    lcd.print("MANUAL"); // Chế độ thủ công được báo lên màn hình lcd
  } else {
    lcd.print("  AUTO"); // Chế độ tự động được báo lên màn hình lcd
    // Blynk.virtualWrite(pin, value) Hàm này gửi giá trị value tới địa chỉ pin trên cloud
    // pin - địa chỉ VirtualPin được thiết lập trên cloud. Mỗi địa chỉ gắn với từng linh kiện của mạch
    // Trong chế độ tự động, các linh kiện tự động bật hay tắt sẽ lưu vào biến arr[i] và gửi lên cloud
    Blynk.virtualWrite(3, arr[0]); // Quạt gió -> địa chỉ V3
    Blynk.virtualWrite(4, arr[1]); // Máy bơm -> địa chỉ V4
    Blynk.virtualWrite(5, arr[2]); // Rèm cửa -> địa chỉ V5
    Blynk.virtualWrite(6, arr[3]); // Đèn sáng -> địa chỉ V6
    Blynk.virtualWrite(7, arr[4]); // Đèn sưởi -> địa chỉ V7
  }
  // Gửi giá trị cảm biến đến cloud bất kể chế độ nào
  Blynk.virtualWrite(0, humid); // Gửi giá trị độ ẩm -> địa chỉ V0
  Blynk.virtualWrite(1, light); // Gửi giá trị ánh sáng -> địa chỉ V1
  Blynk.virtualWrite(2, temp1); // Gửi giá trị nhiệt độ -> địa chỉ V2
}

/*
Khi chạy hàm Blynk.run() trong loop, mạch sẽ liên tục gửi và nhận dữ liệu với cloud.
Hàm BLYNK_WRITE_DEFAULT() sẽ giúp mạch thực hiện việc nhận và lưu dữ liệu () từ cloud.
Có thể xem là thực hiện yêu cầu của người dùng thông qua cloud.
*/
BLYNK_WRITE_DEFAULT() {
  // Dữ liệu mạch nhận về sẽ gồm:
  DetectPin = request.pin; // Địa chỉ gửi yêu cầu tới linh kiện tương ứng
  PinValue = param.asInt(); // Yêu cầu cần thực hiện (0/1 - bật/tắt)
}


void loop() {
  Blynk.run(); // Duy trì kết nối với cloud. Dùng hàm delay(1000) sẽ khiến việc này bị gián đoạn.
  if (millis() - lasttime >= 1000) {
    sendss(); // Đọc cảm biến và gửi dữ liệu; đặt trong hàm if để thay thế delay(1000)
    lasttime = millis();
  }
// Hàm dưới kiểm tra yêu cầu chế độ tự động hay thủ công từ địa chỉ V8
  if (DetectPin == 8 && PinValue == 1) {
    mode = 1; // Chế độ tự động
  } else if (DetectPin == 8 && PinValue == 0) {
    mode = 0; // Chế độ thủ công
  }
  if (mode == 0) { // Khi chọn chế độ thủ công
    switch (DetectPin) { // Yêu cầu của người dùng tới linh kiện nào (Các Lệnh switch-case này chỉ hoạt động trong chế độ thủ công)
      case 3: // Từ V3 -> quạt
        if (DetectPin == 3 && PinValue == 1) {
          digitalWrite(3, 1); // Bật quạt
        } else if (DetectPin == 3 && PinValue == 0) {
          digitalWrite(3, 0);//Tắt quạt
        }
        break;
      case 4:
        if (DetectPin == 4 && PinValue == 1) { // Từ V4 -> máy bơm
          digitalWrite(2, 1); // Bơm nước
        } else if (DetectPin == 4 && PinValue == 0) {
          digitalWrite(2, 0); // Tắt máy bơm
        }
        break;
      case 5:
        if (DetectPin == 5 && PinValue == 1) { // Từ V5 -> rèm cửa (servo)
          myservo.write(180); // Mở rèm cửa - servo xoay đến góc 180
        } else if (DetectPin == 5 && PinValue == 0) {
          myservo.write(0); // Đóng rèm của - servo xoay về góc 0
        }
        break;
      case 6:
        if (PinValue == 1 && DetectPin == 6) { // Từ V6 -> đèn sáng
          leds1.setColorRGB(0, 255, 255, 255); // Bật đèn sáng - Cả 3 màu đỏ, lam, lục sáng cao nhất
        } else if (DetectPin == 6 && PinValue == 0) {
          leds1.setColorRGB(0, 0, 0, 0); // Tắt đèn
        }
        break;
      case 7:
        if (PinValue == 1 && DetectPin == 7) { // Từ V7 -> đèn sưởi
          heater.setColorRGB(0, 255, 0, 0); // Bật đèn sưởi - Đèn màu đỏ sáng cao nhất; tắt 2 đèn lam, lục
        } else if (DetectPin == 7 && PinValue == 0) {
          heater.setColorRGB(0, 0, 0, 0); // Tắt đèn
        }
        break;
      default:
        break;
    }
  } else { // Chế độ tự động được bật
    // Rèm cửa mở khi ánh sáng > 1200 hoặc nhiệt độ > 30*C 
    if (light > 1200 || temp1 > 30) {
      myservo.write(180);
      arr[2] = 1;
      // Biến arr[i] lưu giá trị bật tắt của linh kiện rồi gửi lên địa chỉ tương ứng trên cloud (chỉ ở chế độ tự động)
    } else {
      myservo.write(0);
      arr[2] = 0;
    }
    // Máy bơm được bật khi độ ẩm đất < 200, và sẽ bơm đến khi độ ẩm đất > 350
    if (humid < 50) {
      digitalWrite(rel_pump, 1);
      arr[1] = 1;
    } else if (humid > 65) {
      digitalWrite(rel_pump, 0);
      arr[1] = 0;
    }
    // Đèn sưởi được bật khi nhiệt độ < 24*C đến khi nhiệt độ > 27*C thì tắt
    if (temp1 < 24) {
      heater.setColorRGB(0, 255, 0, 0);
      arr[4] = 1;
    } else if (temp1 > 27) {
      heater.setColorRGB(0, 0, 0, 0);
      arr[4] = 0;
    }
    // Quạt gió được bật khi nhiệt độ > 30*C đến khi nhiệt độ < 27*C thì tắt
    if (temp1 > 30) {
      digitalWrite(rel_fan, 1);
      arr[0] = 1;
    } if (temp1 < 27) {
      digitalWrite(rel_fan, 0);
      arr[0] = 0;
    }
    // Đèn sáng được bật khi ánh sáng < 100 đến khi ánh sáng > 150 thì tắt
    if (light < 100) {
      leds1.setColorRGB(0, 255, 255, 255);
      arr[3] = 1;
    } else if (light > 150) {
      leds1.setColorRGB(0, 0, 0, 0);
      arr[3] = 0;
    }
  }
}