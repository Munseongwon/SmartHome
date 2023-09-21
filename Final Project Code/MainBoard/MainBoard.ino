#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DHT.h>

#define WINDOW_PIN 11 // 창문(서보모터) 핀 설정
#define DOOR_PIN 10 // 문(서보모터) 핀 설정
#define LEDPIN 14  //  경보 장치 LED 핀 설정
#define red 26 // red led 5번핀 설정
#define green 27 // green led 6번핀 설정
#define blue 28 // blue led 7번핀 설정

Servo Window; // 창문을 제어할 수 있는 서보모터 객체 생성
Servo Door; // 현관문을 제어할 수 있는 서보모터 객체 생성
//Servo servo_gar;


int led[] = {2, 3, 5};
static char;  
char data; // 재난 상황이 발생했을 때 SubBoard로부터 받을 데이터를 저장할 변수

void setup() {
  Serial.begin(9600); // 시리얼 통신(컴퓨터 통신, 아두이노 통신)
  Serial1.begin(9600); // Serial1 포트 초기화(19번 Rx, 18번 Tx)
  Serial2.begin(9600); //hm10 Serial2 포트 초기화(17번 Tx, 16번 Tx)
  
  // LED 초기화
  for (int i = 1; i <= 6; i++) {
    pinMode(led[i], OUTPUT);
  }

  // 창문, 문 핀 번호 초기화
  Window.attach(WINDOW_PIN);
  Door.attach(DOOR_PIN);
}

void loop() {
  btncheck(); // 블루투스 연결
  
  // 데이터를 전달받을 수 있다면 Serial1의 데이터를 읽음
  if (Serial1.available()) {
    data = Serial1.read();
    
    // '1', '2'의 값의 데이터를 전달 받으면 => 평시 상황[문을 열지 않고 창문 역시 열지 않음]
    // '3'의 데이터를 전달 받으면 => 재난 상황[문과 창문을 모두 엶]
    if (data == '1') {
      Window.write(0);
      Door.write(0);
    } else if (data == '2') {
      Window.write(0);
      Door.write(0);
    } else if (data == '3') {
      Window.write(80);
      Door.write(80);
    }
  } 
  else {
    int waterOfSoil = analogRead(A1); // 토양의 수분의 양 변수값을 A1핀을 통해 읽어들인다(0~1023)
    if ((waterOfSoil >= 200 && waterOfSoil < 499)) {
      Enough();
    }
    else if ((waterOfSoil < 200)) {
      Low();
    }
    else if ((waterOfSoil >= 499)) {
      High();
    }
  }
}

//무드등 켜고 끄는 함수
void turnOnLights() {
  for (int i = 1; i <= 6; i++) {
    digitalWrite(led[i], HIGH);
  }
}
void turnOffLights() {
  for (int i = 1; i <= 6; i++) {
    digitalWrite(led[i], LOW);
  }
}

// 문 On/Off[서보모터]
void OpenDoor() {
  Door.write(0);
}
void CloseDoor() {
  Door.write(80);
}

// 창문 On/Off[서보모터]
void OpenWindow() {
  Window.write(0);
}
void CloseWindow() {
  Window.write(80);
}

// Tv On/Off[LCD]
void turnOnTv() {
  Serial1.write('4');
}
void turnOffTv() {
  Serial1.write('5');
}

// AirCon On/Off[MP3, LED On/Off]
void turnOnAirConditioner() {
  Serial1.write('6');
}
void turnOffAirConditioner() {
  Serial1.write('7');
}

// Smart Garage Door On/Off
void OpenGarageDoor() {
  Serial1.write('8');
}
void CloseGarageDoor() {
  Serial1.write('9');
}

//식물 성장 감지 시스템
//물의 양 적정할 때, green_led 켜짐(1초간 깜빡거림 설정)
void Enough() {
  digitalWrite(green, HIGH);
  delay(1000);
  digitalWrite(green, LOW);
}
//물의 양이 너무 많을 때, blue_led 켜짐(1초간 깜빡거림 설정)
void High() {
  digitalWrite(blue, HIGH);
  delay(1000);
  digitalWrite(blue, LOW);
}
// 물의 양이 너무 적을 때, red_led 켜짐(1초간 깜빡거림 설정)
void Low() {
  digitalWrite(red, HIGH);
  delay(1000);
  digitalWrite(red, LOW);
}

// 메인 보드 - hm10 블루투스 통신을 통한 데이터 전송 및 전달
void btncheck() {
  if (Serial.available() > 0) {
    Serial2.write(Serial.read());
  }
  if (Serial2.available() > 0) {
    char s = Serial2.read(); // 시리얼 모니터로부터 들어온 데이터를 Serial1으로 출력
    
    // 'a' ~ 'l'의 문자열을 입력받을 때 모듈을 제어하는 함수 호출
    if (s == 'a') {
      turnOffLights();
    }
    else if (s == 'b') {
      turnOnLights();
    }
    else if (s == 'c') {
      CloseWindow();
    }
    else if (s == 'd') {
      OpenWindow();
    }
    else if (s == 'e') {
      turnOffTv();
    }
    else if (s == 'f') {
      turnOnTv();
    }
    else if (s == 'g') {
      CloseDoor();
    }
    else if (s == 'h') {
      OpenDoor();
    }
    else if (s == 'i') {
      turnOffAirConditioner();
    }
    else if (s == 'j') {
      turnOnAirConditioner();
    }
    else if (s == 'k') {
      CloseGarageDoor();
    }
    else if (s == 'l') {
      OpenGarageDoor();
    }
  }
}
