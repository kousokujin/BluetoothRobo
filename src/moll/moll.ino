#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <services.h>
#include <Servo.h> 

//Type
#define SET_UP 0
#define COMMAND 1
#define SET_LED 2

//Commands
#define STOP 0
#define FORWARD 1
#define BACK 2
#define TURN_LEFT 3
#define TURN_RIGHT 4
#define LEFT_FORWARD 5
#define RIGHT_FORWARD 6
#define LEFT_BACK 7
#define RIGHT_BACK 8

//Connection status
#define DISCONNECTED 0
#define CONNECTED 1

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

int ledRed = 0;
int ledGreen = 1;
int ledBlue = 2;

//速度
int velocity = DEFAULT_VELOCITY;
//センサの閾値
int sensorThreshold = DEFAULT_SENSOR_THRESHOLD;
//回避の後退時間
int backPeriod = DEFAULT_BACK_PERIOD;
//回避の転回時間
int turnPeriod = DEFAULT_TURN_PERIOD;

byte command = STOP;

void setup(){
  //ピンの初期化
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);

  //9600bpsらしいで
  Serial.begin(9600);

  //何故か起動直後に10番ピンに5Vが出力されるので強制停止
  _stop();

  ble_begin();
}

void loop(){
  int sensorL = analogRead(0);
  int sensorR = analogRead(1);

  if(ble_connected() == CONNECTED){
    //LED緑でいいかなめんどくさい
    setLed(LOW, HIGH, LOW);
  }
  else{
    //LEDを赤く
    setLed(HIGH, LOW, LOW);
    //切断した場合強制停止
    command = STOP;
    doCommand(command);
  }

  //センサL,R両方に反応があれば後退
  if(sensorR > sensorThreshold && sensorL > sensorThreshold){
    back();
    delay(backPeriod);
    doCommand(command);
  }
  else if(sensorL > sensorThreshold){
    //センサLにだけ反応があれば右旋回
    //右折後退
    rightBack();
    delay(turnPeriod);
    doCommand(command);
  }
  else if(sensorR > sensorThreshold){
    //センサRにのみ反応があれば左旋回
    //左折後退
    leftBack();
    delay(turnPeriod);
    doCommand(command);
  }
  else{
    while(ble_available()){
      byte value1 = ble_read();
      byte bytes[4];
      int red,green,blue;

      //value1で信号の種類を識別
      switch(value1){
      case SET_UP:
        //速度(最高でも255だから特に変換は必要ない)
        velocity = ble_read();
        for(int i = 0 ; i < 3 ;i++){
          //センサの閾値を変更
          //byte配列の取得
          for(int j = 0; j < 4 ; j++){
            bytes[j] = ble_read();
          }
          switch(i){
          case 0:
            sensorThreshold = bytesToInt(bytes);
            break;
          case 1:
            backPeriod = bytesToInt(bytes);
            break;
          case 2:
            turnPeriod = bytesToInt(bytes);
            break;
          }
        }
        break;
        //コマンドを実行
      case COMMAND:
        command = ble_read();
        doCommand(command);
        break;
      case SET_LED:
        red = ble_read();
        green  = ble_read();
        blue = ble_read();
        setLed(red,green,blue);
        break;
      }
    }
  }

  ble_do_events();
}

//LEDの設定
void setLed(int red,int green, int blue){
  digitalWrite(ledRed, red);
  digitalWrite(ledGreen, green);
  digitalWrite(ledBlue, blue);
}

//コマンドの実行
void doCommand(byte _command){
  switch(_command){
  case STOP:
    _stop();
    break;
  case FORWARD:
    forward();
    break;
  case BACK:
    back();
    break;
  case TURN_LEFT:
    turnLeft();
    break;
  case TURN_RIGHT:
    turnRight();
    break;
  case LEFT_FORWARD:
    leftForward();
    break;
  case RIGHT_FORWARD:
    rightForward();
    break;
  case LEFT_BACK:
    leftBack();
    break;
  case RIGHT_BACK:
    rightBack();
    break;
  }
}

//停止
void _stop(){
  //Serial.println("STOP");
  analogWrite(leftP, 0);
  analogWrite(leftN, 0);
  analogWrite(rightP, 0);
  analogWrite(rightN, 0);
}

//前進
void forward(){
  //Serial.println("FORWARD");
  analogWrite(leftP, velocity);
  analogWrite(leftN, 0);
  analogWrite(rightP, velocity);
  analogWrite(rightN, 0);
}

//後退
void back(){
  //Serial.println("BACK");
  analogWrite(leftP, 0);
  analogWrite(leftN, velocity);
  analogWrite(rightP, 0);
  analogWrite(rightN, velocity);
}

//左回転
void turnLeft(){
  //Serial.println("TURN LEFT");
  analogWrite(leftP, 0);
  analogWrite(leftN, velocity);
  analogWrite(rightP, velocity);
  analogWrite(rightN, 0);
}

//右回転
void turnRight(){
  //Serial.println("TURN RIGHT");
  analogWrite(leftP, velocity);
  analogWrite(leftN, 0);
  analogWrite(rightP, 0);
  analogWrite(rightN, velocity);
}

//左折前進
void leftForward(){
  //Serial.println("LEFT_FORWARD");
  analogWrite(leftP, 0);
  analogWrite(leftN, 0);
  analogWrite(rightP, velocity);
  analogWrite(rightN, 0);
}

//右折前進
void rightForward(){
  //Serial.println("RIGHT_FORWARD");
  analogWrite(leftP, velocity);
  analogWrite(leftN, 0);
  analogWrite(rightP, 0);
  analogWrite(rightN, 0);
}

//左折後退
void leftBack(){
  //Serial.println("LEFT_BACK");
  analogWrite(leftP, 0);
  analogWrite(leftN, 0);
  analogWrite(rightP, 0);
  analogWrite(rightN, velocity);
}

//右折前進
void rightBack(){
  //Serial.println("RIGHT_BACK");
  analogWrite(leftP, 0);
  analogWrite(leftN, velocity);
  analogWrite(rightP, 0);
  analogWrite(rightN, 0);
}

//int型に変換
int bytesToInt(byte bytes[]){
  int value;
  for(int i = 0 ; i < 4 ; i++){
    value = (value << 8) + bytes[i]; 
  }
  return value;
}

