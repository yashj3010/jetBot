#Defines
import time
import movement
import machine 

from umqttsimple import MQTTClient

from sensors import HCSR04
from sensors import Servo

from boot import client_id
from boot import ssid
from boot import password
from boot import mqtt_server
from boot import topic_sub
from sensors import Servo

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
boundsDetectionStart = False

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
    time.sleep(.5)

def lookStraight():
    distanceServo.write_angle(90)
    time.sleep(.5)

def lookRight():
    distanceServo.write_angle(120)
    time.sleep(.5)

def sub_cb(topic, msg):
  print((topic, msg))

  if topic == b'inTopic' and msg == MOVE_FORWARD_CALLBACK:
    print("1 Recieved")
    movement.moveForward()

  if topic == b'inTopic' and msg == MOVE_BACKWARD_CALLBACK:
    movement.moveBackward()
    print("2 Recieved")

  if topic == b'inTopic' and msg == MOVE_RIGHT_CALLBACK:
    movement.moveRight()
    print("3 Recieved")

  if topic == b'inTopic' and msg == MOVE_LEFT_CALLBACK:
    movement.moveLeft()
    print("4 Recieved")

  if topic == b'inTopic' and msg == MOVE_STOP_CALLBACK:
    movement.moveStop()
    print("5 Recieved")

  if topic == b'inTopic' and msg == TOGGLE_GETDISTANCE_CALLBACK:
    movement.toggleGetDistance()
    print("6 Recieved")

  if topic == b'inTopic' and msg == TOGGLE_AUTOMOVEMENT_CALLBACK:
    movement.toggleAutoMovement()
    print("7 Recieved")

  if topic == b'inTopic' and msg == TIMER_MOVEMENT_CALLBACK:
    #movement.moveForward()
    print("8 Recieved: Movement On Timer")

  if topic == b'inTopic/Router/Time':
    print(msg)

  if topic == b'servoDown':
    print(msg)

  if topic == b'servoUp':
    print(msg)

  if topic == b'servoDistance':
    print(int(msg))
    distanceServo.write_angle(int(msg))
    time.sleep(0.5)


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
    DISTANCE_SAMPLE_TIME = 1
    MESSAGE_PUBLISH_TIME = 2
    COLLISION_DISTANCE = 20

    now = time.time()
    client.check_msg()

    if getDistance:
      if now - lastMsgDistance > DISTANCE_SAMPLE_TIME:
        lookStraight()
        time.sleep(0.5)

        distance = distanceSensor.distance_cm()
        client.publish("outTopic/Distance", str(distance))

        lastMsgDistance = time.time()

    if isAutoMovement:
      # if now - lastMsgDistanceMovement > DISTANCE_SAMPLE_TIME:
      lastMsgDistanceMovement = time.time()
      lookStraight()
      time.sleep(0.5)

      distance = distanceSensor.distance_cm()
      client.publish("outTopic/Distance", str(distance))

      if distance < COLLISION_DISTANCE:

        client.publish("outTopic/Collision/Status", "1")
        client.publish("outTopic/Collision/Distance", str(distance))

        xEnd = now - boundsDetectionStart
        client.publish("outTopic/Bounds/X", str(xEnd))

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

      elif (distance >= COLLISION_DISTANCE):
        client.publish("outTopic/Collision/Status", "0")
        client.publish("outTopic/Collision/Distance", "")
        movement.moveForward()

    elif isAutoMovement == False:
      movement.moveStop()
      lookStraight()
      time.sleep(0.5)
      client.publish("outTopic/Collision/Distance", "ERROR 1: Please Restart")
    else:
      client.publish("outTopic/Collision/Distance", "ERROR 2: Please Restart")

      
    if now - last_message > MESSAGE_PUBLISH_TIME:
      msg = b'Hello #%d' % counter
      client.publish(b'outTopic/Status/Alive',msg)
      last_message = time.time()
      
      counter += 1
      
  except OSError as e:
    restart_and_reconnect()



