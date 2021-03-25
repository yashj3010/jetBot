#include <AFMotor.h>

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Motor test!");

  // turn on motor
  motor1.setSpeed(255);
  motor2.setSpeed(255);
  motor3.setSpeed(255);
  motor4.setSpeed(255);


  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
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


void loop() {
  moveForward();
  delay(5000);
  moveBackward();
}