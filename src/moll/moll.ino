#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <services.h>
#include <Servo.h> 

//Type
#define SET_UP 0
#define COMMAND 1

//Commands
#define STOP 0
#define FORWARD 1
#define BACK 2
#define LEFT 3
#define RIGHT 4
#define TURN_LEFT 5
#define TURN_RIGHT 6

//Connection status
#define DISCONNECTED 0
#define CONNECTED 1

//PWM
#define PWM_DEF 120

//デフォルトのセンサの閾値
#define SENSOR_THRESHOLD_DEF 500

//回避の転回時間
#define TURN_DELAY 700

//ピン番号
int leftP = 5;
int leftN = 6;
int rightP = 3;
int rightN = 10;

int ledRed = 0;
int ledGreen = 1;
int ledBlue = 2;

//センサの閾値
int sensorThreshold = SENSOR_THRESHOLD_DEF;

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
    delay(600);
    doCommand(command);
  }
  else if(sensorL > sensorThreshold){
    //センサLにだけ反応があれば右旋回
    //若干下がる
    back();
    delay(200);
    //右旋回
    turnRight();
    delay(TURN_DELAY);
    doCommand(command);
  }
  else if(sensorR > sensorThreshold){
    //センサRにのみ反応があれば左旋回
    //若干下がる
    back();
    delay(200);
    //左旋回
    turnLeft();
    delay(TURN_DELAY);
    doCommand(command);
  }
  else{
    while(ble_available()){
      byte value1 = ble_read();
      byte *value = new byte[4];
      int data = 0;

      //value1で信号の種類を識別
      switch(value1){
        //センサの閾値を変更
      case SET_UP:
        //byte配列の取得
        for(int i = 0; i < 4 ; i++){
          value[i] = ble_read();
        }
        //int型に変換
        for(int i = 0 ; i < 4 ; i++){
          data = (data << 8) + value[i]; 
        }
        sensorThreshold = data;
        Serial.println(sensorThreshold);
        break;
        //コマンドを実行
      case COMMAND:
        command = ble_read();
        doCommand(command);
        break;
      }
    }
  }

  ble_do_events();
}

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
  case LEFT:
    left();
    break;
  case RIGHT:
    right();
    break;
  case TURN_LEFT:
    turnLeft();
    break;
  case TURN_RIGHT:
    turnRight();
    break;
  }
}

//停止
void _stop(){
  Serial.println("STOP");
  analogWrite(leftP, 0);
  analogWrite(leftN, 0);
  analogWrite(rightP, 0);
  analogWrite(rightN, 0);
}

//前進
void forward(){
  Serial.println("FORWARD");
  analogWrite(leftP, PWM_DEF);
  analogWrite(leftN, 0);
  analogWrite(rightP, PWM_DEF);
  analogWrite(rightN, 0);
}

//後退
void back(){
  Serial.println("BACK");
  analogWrite(leftP, 0);
  analogWrite(leftN, PWM_DEF);
  analogWrite(rightP, 0);
  analogWrite(rightN, PWM_DEF);
}

//左折
void left(){
  Serial.println("LEFT");
  analogWrite(leftP, 0);
  analogWrite(leftN, 0);
  analogWrite(rightP, PWM_DEF);
  analogWrite(rightN, 0);
}

//右折
void right(){
  Serial.println("RIGHT");
  analogWrite(leftP, PWM_DEF);
  analogWrite(leftN, 0);
  analogWrite(rightP, 0);
  analogWrite(rightN, 0);
}

//左回転
void turnLeft(){
  Serial.println("TURN LEFT");
  analogWrite(leftP, 0);
  analogWrite(leftN, PWM_DEF);
  analogWrite(rightP, PWM_DEF);
  analogWrite(rightN, 0);
}

//右回転
void turnRight(){
  Serial.println("TURN RIGHT");
  analogWrite(leftP, PWM_DEF);
  analogWrite(leftN, 0);
  analogWrite(rightP, 0);
  analogWrite(rightN, PWM_DEF);
}

//LEDの設定
void setLed(int red,int green, int blue){
  digitalWrite(ledRed, red);
  digitalWrite(ledGreen, green);
  digitalWrite(ledBlue, blue);

}


