import time
import machine
from sensors import Servo

DISTANCE_SERVO_PIN = 23

distanceServoPin = machine.Pin(DISTANCE_SERVO_PIN)
distanceServo = Servo(distanceServoPin)

def lookLeft():
    distanceServo.write_angle(60)
    time.sleep(.5)

def lookStraight():
    distanceServo.write_angle(90)
    time.sleep(.5)

def lookRight():
    distanceServo.write_angle(120)
    time.sleep(.5)
    
while True:
    lookLeft()
    lookStraight()
    lookRight()

