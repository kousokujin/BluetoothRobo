
#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <services.h>
#include <Servo.h> 

//Type
#define SET_UP 0
#define MOVE 1
#define SET_LED 2

//Commands
#define STOP 0,0,0,0
#define FORWARD defVelocityL,0,defVelocityR,0
#define BACK 0,defVelocityL,0,defVelocityR
#define TURN_LEFT 0,defVelovityL,defVelocityR,0
#define TURN_RIGHT defVelocityL,0,0,defVelocityR
#define LEFT_FORWARD 0,0,defVelocityR,0
#define RIGHT_FORWARD defVelocityL,0,0,0
#define LEFT_BACK 0,0,0,defVelocityR
#define RIGHT_BACK 0,defVelocityL,0,0

//Connection status
#define DISCONNECTED 0
#define CONNECTED 1

//LED
#define RED HIGH,LOW,LOW
#define GREEN LOW,HIGH,LOW

//デフォルトの速度(PWM)
#define DEFAULT_VELOCITY 120
//デフォルトのセンサの閾値
#define DEFAULT_SENSOR_THRESHOLD 500
//デフォルトの回避の後退時間
#define DEFAULT_BACK_PERIOD 500
//デフォルトの回避の転回時間
#define DEFAULT_TURN_PERIOD 500

//ピン番号
int leftP = 5;
int leftN = 6;
int rightP = 3;
int rightN = 10;

int ledRed = 19;
int ledGreen = 18;
int ledBlue = 17;

//速度
int defVelocityL = DEFAULT_VELOCITY;
int defVelocityR = DEFAULT_VELOCITY;

int velocityLP,velocityLN,velocityRP,velocityRN;

//センサの閾値
int sensorThreshold = DEFAULT_SENSOR_THRESHOLD;
//回避の後退時間
int backPeriod = DEFAULT_BACK_PERIOD;
//回避の転回時間
int turnPeriod = DEFAULT_TURN_PERIOD;

int connectionStatus = DISCONNECTED;

void setup(){
  //ピンの初期化
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);

  //9600bpsらしいで
  Serial.begin(9600);

  //何故か起動直後に10番ピンに5Vが出力されるので強制停止
  _move(STOP);
  //LEDの初期化
  setLed(RED);

  ble_begin();
}

void loop(){
  int sensorL = analogRead(0);
  int sensorR = analogRead(1);

  if(ble_connected() != connectionStatus){
    connectionStatus = ble_connected();
    if(connectionStatus == CONNECTED){
      //LED緑でいいかなめんどくさい
      setLed(GREEN);
    }
    else{
      //LEDを赤く
      setLed(RED);
      //切断した場合強制停止
      _move(velocityLP = 0, velocityLN = 0, velocityRP = 0, velocityRN = 0); 
    }  
  }

  //センサL,R両方に反応があれば後退
  if(sensorR > sensorThreshold && sensorL > sensorThreshold){
    _move(BACK);
    delay(backPeriod);
    _move(velocityLP, velocityLN, velocityRP, velocityRN);
  }
  else if(sensorL > sensorThreshold){
    //sensorLにだけ反応があれば右折後退
    _move(LEFT_BACK);
    delay(turnPeriod);
    _move(velocityLP, velocityLN, velocityRP, velocityRN);
  }
  else if(sensorR > sensorThreshold){
    //sensorRにのみ反応があれば左折後退
    _move(RIGHT_BACK);
    delay(turnPeriod);
    _move(velocityLP, velocityLN, velocityRP, velocityRN);
  }
  else{
    while(ble_available()){
      byte value = ble_read();

      //valueで信号の種類を識別
      switch(value){
      case SET_UP:
        //速度(最高でも255だから特に変換は必要ない)
        defVelocityL = ble_read();
        defVelocityR = ble_read();
        //Intの取得
        sensorThreshold = ble_read_int();
        backPeriod = ble_read_int();
        turnPeriod = ble_read_int();
        break;
        //走行
      case MOVE:
        velocityLP = ble_read();
        velocityLN = ble_read();
        velocityRP = ble_read();
        velocityRN = ble_read();

        _move(velocityLP, velocityLN, velocityRP, velocityRN);
        break;
        //LEDの色を変更
      case SET_LED:
        setLed(ble_read(), ble_read(), ble_read());
        break;
      }
    }
  }
  ble_do_events();
}

//LEDの設定
void setLed(int red,int green, int blue){
  //Serial.println("LED");
  digitalWrite(ledRed, red);
  digitalWrite(ledGreen, green);
  digitalWrite(ledBlue, blue);
}

//走行
void _move(int velocityLP, int velocityLN, int velocityRP, int velocityRN){
  //左
  analogWrite(leftP, velocityLP);
  analogWrite(leftN, velocityLN);
  //右
  analogWrite(rightP, velocityRP);
  analogWrite(rightN, velocityRN);
}

//int型のデータを取得
int ble_read_int(){
  byte bytes[4];
  for(int i = 0 ; i < 4 ; i++){
    bytes[i] = ble_read();
  }

  int value;
  for(int i = 0 ; i < 4 ; i++){
    value = (value << 8) + bytes[i]; 
  }
  return value;
}




