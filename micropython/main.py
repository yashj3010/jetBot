#Defines
import time
import utime
import movement
import machine 
from machine import I2C, Pin
from time import sleep
import dht 
from ds1307 import DS1307
import machine

sensor = dht.DHT11(Pin(14))

sclPin = Pin(22) # serial clock pin
sdaPin = Pin(21) # serial data pin

i2c_object = I2C(0,              # positional argument - I2C id
                  scl = sclPin,  # named argument - serial clock pin
                  sda = sdaPin,  # named argument - serial data pin
                  freq = 400000 )# named argument - i2c frequency

result = I2C.scan(i2c_object) # scan i2c bus for available devices

print("I2C scan result : ", result)
if result != []:
    print("I2C connection successfull")
else:
    print("retry")


# clock object at the dedicated i2c port
clockObject = DS1307(i2c_object)

#  enable the RTC module
clockObject.halt(False) # 32 khz crystal enable

from umqttsimple import MQTTClient

from sensors import HCSR04
from sensors import Servo
from sensors import Servo

from boot import client_id
from boot import ssid
from boot import password
from boot import mqtt_server
from boot import topic_sub

SERVO_DOWN_PIN     = 21
SERVO_UP_PIN       = 22
DISTANCE_SERVO_PIN = 23

TRIGGER_PIN = 2
ECHO_PIN = 5

COLLISION_DISTANCE   = 20

LOOK_RIGHT_ANGLE     = 90
LOOK_STRAIGHT_ANGLE  = 70
LOOK_LEFT_ANGLE      = 60

MESSAGE_PUBLISH_TIME = 2000

MOVE_FORWARD_CALLBACK        = b"1"
MOVE_BACKWARD_CALLBACK       = b"2"
MOVE_RIGHT_CALLBACK          = b"3"
MOVE_LEFT_CALLBACK           = b"4"
MOVE_STOP_CALLBACK           = b"5"
TOGGLE_GETDISTANCE_CALLBACK  = b"6"
TOGGLE_AUTOMOVEMENT_CALLBACK = b"7"
TIMER_MOVEMENT_CALLBACK      = b"8"

servoDownPin = machine.Pin(SERVO_DOWN_PIN)
servoUpPin = machine.Pin(SERVO_UP_PIN)
distanceServoPin = machine.Pin(DISTANCE_SERVO_PIN)


isAutoMovement = False
getDistance = False

boundsDetectionStart = 0
last_message = 0
lastMsgDistance = 0
lastMsgDistanceMovement = 0
counter = 0

servoDown     = Servo(servoDownPin)
servoUp       = Servo(servoUpPin)
distanceServo = Servo(distanceServoPin)

distanceSensor = HCSR04(trigger_pin=TRIGGER_PIN, echo_pin=ECHO_PIN,echo_timeout_us=1000000)

def lookLeft():
    distanceServo.write_angle(60)
    time.sleep_ms(200)

def lookStraight():
    distanceServo.write_angle(90)
    time.sleep_ms(200)

def lookRight():
    distanceServo.write_angle(120)
    #time.sleep(.5)

def getTemp123():
    sleep(2)
    sensor.measure()
    temp = sensor.temperature()
    hum = sensor.humidity()
    client.publish("outTopic/Temp", str(temp))
    client.publish("outTopic/Humidity", str(hum))
    print(temp)
    print(hum)

def getTime():
    print(ds.datetime(now))
    
    
def sub_cb(topic, msg):
  print((topic, msg))

  if topic == b'inTopic' and msg == MOVE_FORWARD_CALLBACK:
    movement.moveForward()

  if topic == b'inTopic' and msg == MOVE_BACKWARD_CALLBACK:
    movement.moveBackward()

  if topic == b'inTopic' and msg == MOVE_RIGHT_CALLBACK:
    movement.moveRight()

  if topic == b'inTopic' and msg == MOVE_LEFT_CALLBACK:
    movement.moveLeft()

  if topic == b'inTopic' and msg == MOVE_STOP_CALLBACK:
    movement.moveStop()

  if topic == b'inTopic' and msg == TOGGLE_GETDISTANCE_CALLBACK:
    movement.toggleGetDistance()

  if topic == b'inTopic' and msg == TOGGLE_AUTOMOVEMENT_CALLBACK:
    movement.toggleAutoMovement()

  if topic == b'inTopic/MovementTimer':
    toRun = int(msg.decode("utf-8")[1:])
    start = round(time.time() * 1000)
    end = start + toRun
    
    print("Move For:",toRun,"Milliseconds", "Started At:",start,"End At:",end)

    if msg[0] == MOVE_FORWARD_CALLBACK[0]:
      while(round(time.time() * 1000) < end):
            movement.moveForward()

    elif msg[0] == MOVE_BACKWARD_CALLBACK[0]:
      while(round(time.time() * 1000) < end):
            movement.moveBackward()
      
  if topic == b'inTopic/Router/Time':
    movement.moveRouter(msg.decode("utf-8"))

  if topic == b'servoDown':
    print(msg)

  if topic == b'servoUp':
    print(msg)

  if topic == b'servoDistance':
    print(int(msg))
    distanceServo.write_angle(int(msg))
    time.sleep_ms(250)


def connect_and_subscribe():
  global client_id, mqtt_server, topic_sub
  client = MQTTClient(client_id, mqtt_server)
  client.set_callback(sub_cb)
  client.connect()
  
  client.subscribe(topic_sub)
  client.subscribe("inTopic")
  client.subscribe("servoDown")
  client.subscribe("servoUp")
  client.subscribe("servoDistance")
  client.subscribe("inTopic/Router/Time")
  client.subscribe("inTopic/MovementTimer")

  print('Connected to %s MQTT broker, subscribed to %s topic' % (mqtt_server, topic_sub))
  return client

def restart_and_reconnect():
  print('Failed to connect to MQTT broker. Reconnecting...')
  time.sleep(10)
  machine.reset()
  
try:
  client = connect_and_subscribe()
except OSError as e:
  restart_and_reconnect()

while True:
  try:
    now = time.time()
    client.check_msg()
    if getDistance:
        lookStraight()
        distance = distanceSensor.distance_cm()
        client.publish("outTopic/Distance", str(distance))

    if isAutoMovement:
      distance = distanceSensor.distance_cm()
      client.publish("outTopic/Distance", str(distance))
      #getTemp123()
      print(clockObject.datetime())
      if distance < 50:

        client.publish("outTopic/Collision/Status", "1")
        client.publish("outTopic/Collision/Distance", str(distance))

        client.publish("outTopic/Bounds/X", str(now - boundsDetectionStart))

        movement.moveStop()

        lookRight()
        rightDistance = distanceSensor.distance_cm()
        
        lookLeft()
        leftDistance = distanceSensor.distance_cm()
        
        lookStraight()

        if rightDistance > leftDistance:
          movement.moveRight()
          time.sleep_ms(1000)
          movement.moveStop()
          client.publish("outTopic/Collision/Decision", "3")

        elif rightDistance < leftDistance:
          movement.moveLeft()
          time.sleep_ms(1000)
          movement.moveStop()
          client.publish("outTopic/Collision/Decision", "4")

        else:
          movement.moveBackward()
          client.publish("outTopic/Collision/Distance", "Right Left Same")

      elif (distance >= 50):
        client.publish("outTopic/Collision/Status", "0")
        client.publish("outTopic/Collision/Distance", "")
        movement.moveForward()

    elif isAutoMovement == False:
      movement.moveStop()
      lookStraight()
      time.sleep_ms(100)
      client.publish("outTopic/Collision/Distance", "ERROR 1: Please Restart")
    else:
      client.publish("outTopic/Collision/Distance", "ERROR 2: Please Restart")

      
    if now - last_message > 2:
      msg = b'Hello #%d' % counter
      client.publish(b'outTopic/Status/Alive',msg)
      last_message = time.time()
      
      counter += 1
      
  except OSError as e:
    restart_and_reconnect()

