#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>


#define DHTPIN    2     //DATA Pin 
#define DHTTYPE   DHT11 //DHT11 라고 TYPE을 정의함
#define LEDPIN    3 //LED PIN
#define Temp_C    26.0
#define red 26 // red led 5번핀 설정

DHT dht(DHTPIN, DHTTYPE);//dht 클랙서 생성 (Data Pin, Sensor Type)
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD 객체 생성
SoftwareSerial MP3Module(52, 53);  //MP3(Rx,Tx)
Servo servo_gar; // SmartGarage 서보모터 객체 생성
DFRobotDFPlayerMini MP3Player;	// MP3 모듈 객체 생성


//주차장 서보모터, 초음파 센서(트리거, 에코), 불꽃 감지 센서, 진동 센서, 재난 상황 표시 led 초기화
//4번 8번 xx
const int gar_PIN = 5;
const int TRIG_PIN = 6;
const int ECHO_PIN = 7;
int fire = 11;
int vibr = 13;
int led_R = 22;

// 재난 상황, 평시 상황 제어 변수
int tries1, tries2 ;
int pos;
int val_fire = 0;

// 초음파 센서로부터 받아오는 거리 및 속도, 임계거리 변수
float duration_us, distance_cm;
const int DISTANCE_THRESHOLD = 7;

// 블루투스 모듈로부터 받는 데이터 => 한 글자로 표시
char data1;

void setup() {
  Serial.begin(9600); // 모니터 프로그램을 위한 시리얼 시작
  Serial1.begin(9600); // Serial1 포트 초기화(19번 Rx, 18번 Tx)
  MP3Module.begin(9600); // MP3 모듈 초기화
  dht.begin(); //DHT11 시작
  pinMode(LEDPIN, OUTPUT); //LED PIN 출력으로 설정
  pinMode(led_R, OUTPUT); // 빨간색 led 핀 초기화
  
  // 초음파 센서(트리거, 에코 핀) 초기화
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(fire, INPUT); // 화재 감지 센서 초기화
  pinMode(vibr, INPUT); // 진동 감지 센서 초기화
  
  // 90도로 초기화 함 --> 스마트 주차장 초기 각도
  servo_gar.attach(gar_PIN);
  servo_gar.write(90);
  
  // lcd 초기화
  lcd.init();
  lcd.backlight();
  
 // MP3 모듈을 초기화합니다. 초기화에 실패하면 오류를 발생시킵니다.
 // 볼륨은 15로 설정
 if (!MP3Player.begin(MP3Module, false)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  MP3Player.volume(15);
}

void loop() {
  // 재난 상황 감지 => Low, 트리거 핀으로부터 물체가 다가올 때 초음파를 발생시켜 거리 및 속도 측정
  digitalWrite(led_R, LOW);
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, HIGH);

  // ECHO 핀에서 펄스의 지속 시간 측정
  duration_us = pulseIn(ECHO_PIN, HIGH);
  
  // 거리 측정
  distance_cm = ((duration_us * 340) / 10000) / 2;
  delay(500);

  //온,습도 측정 , 지진 및 화재 상황 측정 
  float Temp = dht.readTemperature();
  float Humi = dht.readHumidity();
  int val_fire = digitalRead(fire);
  int val_vib = digitalRead(vibr);

  //시리얼 모니터를 통해 찍히는 값과 현재 상황
  Serial.print("Temperature : ");
  Serial.println(Temp);
  Serial.print("Humidity : ");
  Serial.println(Humi);
  Serial.print("distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
  Serial.print("Fire = ");
  Serial.println(val_fire);
  Serial.print("Vibr = ");
  Serial.println(val_vib);
  //delay(100);

    //lcd.clear();
    /*lcd.setCursor(0, 0);
    lcd.print("Temp : ");
    lcd.print(Temp);
    lcd.setCursor(0, 1);
    lcd.print("Humi : ");
    lcd.print(Humi);*/
    

  //  Serial.println(tries);

  //에어컨 작동
  if (Temp >= Temp_C) {
    AirconOn();
    } else if (Temp < Temp_C) {
    AirconOff();
  }

  // 차량 입출차
  if (0 < distance_cm && distance_cm < DISTANCE_THRESHOLD) {
    GarageIn();
    //delay(500); // 1초간의 딜레이를 줌
  }
  else {
    GarageOut();
    //delay(500); //2.5초간의 딜레이를 줌
  }

  // 평시 상황 시 블루투스 모듈로 부터 얻은 값을 통해 스마트홈 모듈을 제어할 함수 각각 호출
  if(Serial1.available()){
    data1 = Serial1.read();
    if(data1 == '4'){
      lcd.backlight();
    } else if(data1 == '5'){
      lcd.noBacklight();
    } else if(data1 == '6'){
      AirconOn();
    } else if(data1 == '7'){
      AirconOff();
    } else if(data1 == '8'){
      GarageIn();
    } else if(data1 == '9'){
      GarageOut();
    }
  }
  
  // 재난 상황 발생 시 LCD, 문, 창문의 활성화를 통한 재난 상황 표시
   //  val_fire => 화재 발생 시, val_vib => 지진 발생 시
  
  // 평시 상황 => 재난 상황
  if (val_fire == 0) {
    lcd.clear(); // lcd 초기화
    digitalWrite(led_R, HIGH); // 불꽃 감지가 일어났으므로 LED => high
    //servo_win.write(90);
    Serial1.write('1'); // 평시 상황일 경우 문, 창문 열지 x
    //delay(1000);
    // lcd (0,0)에 WARNINGS 글자 출력, lcd(0, 1)에 FIRE DETECTED 글자 출력
    lcd.setCursor(0, 0);
    lcd.print("!!!!WARNINGS!!!!");
    lcd.setCursor(0, 1);
    lcd.print(" !FIRE DETECTED!");
    fire_sound(); // 화재 발생 경보음 재생
  }
  
  // 지진 상황 발생 
  else if (val_vib == 1) {
    lcd.clear();
    digitalWrite(led_R, HIGH); // 지진이 발생 LED => high
    //servo_main.write(90);
    Serial1.write('2'); // 지진 상황일 경우, 문, 창문 개방 x
    // lcd (0,0)에 WARNINGS 글자 출력, lcd(0, 1)에 EARTHQUAKE 글자 출력
    lcd.setCursor(0, 0);
    lcd.print("!!!!WARNINGS!!!!");
    lcd.setCursor(0, 1);
    lcd.print("!!!EARTHQUAKE!!!");
    //delay(1000);
    earth_sound(); // 지진 발생 경보음 재생
  }
  
  // 화재 상황 발생, 지진 상황 발생 종료
  else if(val_fire == 1 && val_vib == 0) {
    Serial1.write('3'); // 화재 발생 하였으므로 창문, 문 모두 개방
    // lcd(0,0)은 현재 온도, lcd(0,1)은 현재 습도 측정
    lcd.setCursor(0, 0);
    lcd.print("Temp : ");
    lcd.print(Temp);
    lcd.setCursor(0, 1);
    lcd.print("Humi : ");
    lcd.print(Humi);
  }

}

// 지진이 발생할 경우 2초 간 지진 발생 경고음 재생
void earth_sound(){
  MP3Player.play(6);
    delay(2000);
}

// 화재가 발생할 경우 2초 간 화재 발생 경고음 재생
void fire_sound(){
  MP3Player.play(5);
    delay(2000);
}

// 에어컨 On
void AirconOn(){
  //현재온도가 Temp_C 값 이상이면 LED(에어컨)가 켜진다.
    digitalWrite(LEDPIN, HIGH);
    // 에어컨이 꺼져 있으면 에어컨이 켜지는 음악 재생 =>  0(off)->1(On)
    if (tries1 < 1) {
      MP3Player.play(1);
      tries1++;
  }
}

// 에어컨 Off
void AirconOff(){
   //현재온도가 Temp_C 값 이상이면 LED(에어컨)가 켜진다.
    digitalWrite(LEDPIN, LOW);
    // 에어컨이 꺼져 있으면 에어컨이 꺼지는 음악 재생 =>  1(On)->0(Off)
    if (tries1 >= 1) {
      MP3Player.play(2);
    }
    tries1 = 0;
}

// 차량이 주차장에 진입
void GarageIn() {
  servo_gar.write(10); // 측정거리가 임계거리보다 작을 때 서보모터 90도로 읽음
    lcd.clear(); // 모니터에 남아있는 글자가 있으면 모두 삭제
    // 물체가 다가오는 경우 차량 진입 음악 재생 =>  0(Off)->1(On)
    if (tries2 < 1) {
      MP3Player.play(3);
      lcd.print("Close"); // 모니터에 열려 있다는 것을 구현
      delay(2000);
      tries2++;
    }
}

// 차량이 주차장에 들어가 있는 경우
void GarageOut(){
  servo_gar.write(80);  // 측정거리가 임계거리보다 클 시에는 서보모터 0도로 읽음
    lcd.clear();
    // 물체가 다가오는 경우 차량 진입 음악 재생 =>  1(On)->0(Off)
    if (tries2 >= 1) {
      MP3Player.play(4);
      lcd.print("Open");
      delay(2000);
    }
    tries2 = 0;
}
