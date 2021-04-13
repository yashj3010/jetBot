import time
import machine
from machine import Pin

ENABLEPIN = 32
BIT0PIN   = 33
BIT1PIN   = 25

BIT0 = Pin(BIT0PIN, Pin.OUT)
BIT1 = Pin(BIT1PIN, Pin.OUT)
ENABLE = Pin(ENABLEPIN, Pin.OUT)


# isAutoMovement = main.isAutoMovement
# getDistance = main.getDistance

def moveBackward():
  BIT0.value(1)
  BIT1.value(1)
  ENABLE.value(1)
  # print("moveBackward")


def moveForward():
  BIT0.value(0)
  BIT1.value(0)
  ENABLE.value(1)
  # print("moveForward")


def moveRight():
  BIT0.value(0)
  BIT1.value(1)
  ENABLE.value(1)
  # print("moveRight")


def moveLeft():
  BIT0.value(1)
  BIT1.value(0)
  ENABLE.value(1)
  # print("moveLeft")

def moveStop():
  ENABLE.value(0)
  # print("moveStop")

def toggleGetDistance():
  import main
  main.getDistance = not main.getDistance
  print("T")  
  if main.getDistance:
    main.client.publish("outTopic/Status/Distance", "1")
  else:
    main.client.publish("outTopic/Status/Distance", "0")
  
def toggleAutoMovement():
  import main
  main.boundsDetectionStart = time.time()
  main.isAutoMovement = not main.isAutoMovement
  main.getDistance = True if (main.isAutoMovement == True) else False

  #print(main.isAutoMovement)
  #print(main.getDistance)
  #print(main.boundsDetectionStart)

  if (not main.isAutoMovement):
    main.client.publish("outTopic/Status/AutoMovement", "0")
    main.client.publish("outTopic/Status/Distance", "0")
    moveStop()
  else:
    main.client.publish("outTopic/Status/AutoMovement", "1")
    main.client.publish("outTopic/Status/Distance", "1")


