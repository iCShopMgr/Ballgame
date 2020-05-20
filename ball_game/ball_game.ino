/*
 * 可以透過超音波距離感測器、紅外線反射感測器,觸發馬達轉動.
 * 並透過觸碰感測器(按鈕)來停止馬達轉動.
 */

//匯入軟體序列埠函式庫並指定腳位與存取變數
#include <SoftwareSerial.h>
SoftwareSerial mySerial(11, 12); // RX, TX
int ul = 100, Text, app_button = 0, motor1_speed = 200, motor2_speed = 200;

// 定義感測器與馬達連接的腳位
#define trig 3
#define echo 2
#define infrared 4
#define button 7
#define M1A 5
#define M1B 6
#define M2A 9
#define M2B 10

unsigned long last_time;
bool sonar_switch = true;

// 超音波HC-SR04距離讀取程式(單位:公分)
int sonar() {
  if (sonar_switch) {
    digitalWrite(trig, LOW);
    delayMicroseconds(5);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
  
    pinMode(echo, INPUT);
    last_time = millis();
    long duration = pulseIn(echo, HIGH);
    if ((millis() - last_time) > 100)sonar_switch=false;
    int distance_cm = constrain((duration/2) / 29.1, 0, 100); 
    delay(250);
    return distance_cm;
  }
  else {
    int distance_cm = 0;
    return distance_cm;
  }
}

// 紅外線反射數位讀取程式(定義感測到物體時為:1)
int ir() {
  bool value = digitalRead(4);
  return !value;
}

// 按鈕讀取程式(定義按下按鈕時為:1)
int sw() {
  bool value = digitalRead(7);
  return !value;
}

// 控制第幾顆馬達(數量2), 以及控制轉速(速度正為正轉, 速度負為反轉)
void motor(int number, int Speed) {
  if (number == 1) {
    if (Speed >= 0 ) {
      if (Speed > 255)Speed = 255;
      analogWrite(M1A, Speed);
      analogWrite(M1B, 0);
    }
    else {
      Speed *= -1;
      if (Speed > 255)Speed = 255;
      analogWrite(M1A, 0);
      analogWrite(M1B, Speed);
    }
  }
  else if (number == 2) {
    if (Speed >= 0 ) {
      if (Speed > 255)Speed = 255;
      analogWrite(M2A, Speed);
      analogWrite(M2B, 0);
    }
    else {
      Speed *= -1;
      if (Speed > 255)Speed = 255;
      analogWrite(M2A, 0);
      analogWrite(M2B, Speed);
    }
  }
  else {
    Serial.println("Out of range.");
  }
}

void bluetooth() {
  if (mySerial.available()) {
    Text = mySerial.read();
    if (Text == 65) {
      Serial.print("A:");
      //delay(50);
      Text = mySerial.read();
      Serial.print(Text);
      motor1_speed = int(Text);
    }
    else if (Text == 66) {
      Serial.print("B:");
      //delay(50);
      Text = mySerial.read();
      Serial.print(Text);
      motor2_speed = int(Text);
    }
    else if (Text == 72) {
      byte HC = sonar();
      bool IR = ir();
      mySerial.write(72);
      mySerial.write(HC);
      mySerial.write(IR);
    }
    else if (Text == 83){
      app_button = 1;
    }
    else if (Text == 90) {
      app_button = 0;
    }
    Serial.println("");
  }
}

// 啟動時設定與執行(只在開機時執行一次的程式)
void setup() {
  Serial.begin (9600);
  mySerial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT_PULLUP);
  pinMode(infrared, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);
}

// 重複執行程式
void loop() {
  while ((ul > 10 || ul == 1 || ul == 0) && ir() == 0 && app_button == 0) {
    ul = sonar();
    bluetooth();
    Serial.print("Distance: " + String(ul) + "cm");  
    Serial.print(" / IR: " + String(ir()));
    Serial.println();
  }
  Serial.println("Start!");
  app_button = 1;
  while (sw() == 0 && app_button == 1) {
    ul = sonar();
    bluetooth();
    motor(1, motor1_speed);
    motor(2, motor2_speed);
    Serial.println("Wait button...");
    Serial.print("Distance: " + String(ul) + "cm");  
    Serial.println(" / IR: " + String(ir()));
    Serial.print("M1: " + String(motor1_speed));
    Serial.print(" / M2: " + String(motor2_speed));
    Serial.println();
  }
  Serial.println("End!");
  app_button = 0;
  motor(1, 0);
  motor(2, 0);
}
