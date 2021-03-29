
/////////////////  LIBRARIES ////////////////////////
#include <Arduino.h>
#include<HardwareSerial.h>
#include <AFMotor.h>

///////////////// CONSTANTS ////////////////////////
#define servoDownPin 9
#define servoUpPin 10

#define motorPin1 1
#define motorPin2 2
#define motorPin3 3
#define motorPin4 4

#define motorSpeed 255

#define enable A3
#define bit0 A4
#define bit1 A5

/////////////////  INSTANCES ////////////////////////
AF_DCMotor motor1(motorPin1);
AF_DCMotor motor2(motorPin2);
AF_DCMotor motor3(motorPin3);
AF_DCMotor motor4(motorPin4);

/////////////////  Variables ////////////////////////
AF_DCMotor motors[4] = {motor1,motor2,motor3,motor4};

void resetMotor(){
  for (int i = 0; i < 4; i++) {
    motors[i].setSpeed(motorSpeed);
    motors[i].run(RELEASE);
  }
}

void setup() {
  Serial.begin(9600);  
  pinMode(enable,INPUT);
  pinMode(bit0,INPUT);
  pinMode(bit1,INPUT);
  
  // Initialize all the motors        
  resetMotor();
}

void rightForward(){
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void leftForward(){
  motor1.run(FORWARD);
  motor2.run(FORWARD);
}

void rightBackward(){
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void leftBackward(){
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
}

void moveForward(){
  rightForward();
  leftForward();
}

void moveBackward(){
  rightBackward();
  leftBackward();
}

void moveRight(){
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  rightForward();
}

void moveLeft(){
  motor3.run(RELEASE);
  motor4.run(RELEASE);
  leftForward();
}

String readProtocol(){
  int bit0Sts = digitalRead(bit0);
  int bit1Sts = digitalRead(bit1);

  String status = String(bit0Sts) + String(bit1Sts); 

  return status;
}

void move(){

  bool enableSts = digitalRead(enable) == 1 ? true : false;

  if (enableSts){
    
    String protocolStatus = readProtocol();

    if (protocolStatus == "11"){
      moveForward();
    }
    else if (protocolStatus == "01"){
      moveRight();
    }
    else if (protocolStatus == "10"){
      moveLeft();
    }
    else if (protocolStatus == "00"){
      moveBackward();
    }
  }
  else if (!enableSts){
    resetMotor();
  }

}
void loop() {
  move();
}