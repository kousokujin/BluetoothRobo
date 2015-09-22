#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <services.h>
#include <Servo.h> 

#define STOP 0
#define FORWARD 1
#define BACK 2
#define LEFT 3
#define RIGHT 4
#define TURN_LEFT 5
#define TURN_RIGHT 6

int leftP = 5;
int leftN = 6;
int rightP = 3;
int rightN = 10;

void setup(){
  //9600bpsらしいで
  Serial.begin(9600);

  ble_begin();
  _stop();
}

void loop(){
  while(ble_available()){
    byte command = ble_read();
    switch(command){
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
  ble_do_events();
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
  analogWrite(leftP, 80);
  analogWrite(leftN, 0);
  analogWrite(rightP, 80);
  analogWrite(rightN, 0);
}

//後退
void back(){
  Serial.println("BACK");
  analogWrite(leftP, 0);
  analogWrite(leftN, 80);
  analogWrite(rightP, 0);
  analogWrite(rightN, 80);
}

//左折
void left(){
  Serial.println("LEFT");
  analogWrite(leftP, 0);
  analogWrite(leftN, 0);
  analogWrite(rightP, 80);
  analogWrite(rightN, 0);
}

//右折
void right(){
  Serial.println("RIGHT");
  analogWrite(leftP, 80);
  analogWrite(leftN, 0);
  analogWrite(rightP, 0);
  analogWrite(rightN, 0);
}

//左回転
void turnLeft(){
  Serial.println("TURN LEFT");
  analogWrite(leftP, 0);
  analogWrite(leftN, 80);
  analogWrite(rightP, 80);
  analogWrite(rightN, 0);
}

//右回転
void turnRight(){
  Serial.println("TURN RIGHT");
  analogWrite(leftP, 80);
  analogWrite(leftN, 0);
  analogWrite(rightP, 0);
  analogWrite(rightN, 80);
}


