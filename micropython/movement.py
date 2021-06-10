import time
import machine
from machine import Pin

ENABLEPIN = 32
BIT0PIN   = 33
BIT1PIN   = 25

BIT0 = Pin(BIT0PIN, Pin.OUT)
BIT1 = Pin(BIT1PIN, Pin.OUT)
ENABLE = Pin(ENABLEPIN, Pin.OUT)

def moveBackward():
  BIT0.value(1)
  BIT1.value(1)
  ENABLE.value(1)

def moveForward():
  BIT0.value(0)
  BIT1.value(0)
  ENABLE.value(1)

def moveRight():
  BIT0.value(0)
  BIT1.value(1)
  ENABLE.value(1)

def moveLeft():
  BIT0.value(1)
  BIT1.value(0)
  ENABLE.value(1)

def moveStop():
  ENABLE.value(0)

def moveRouter(route,client):
    seperated = route.split(":")
    timeToRun = seperated[0].split(",")
    turnsToTake = seperated[1].split(",")
    
    timeToRun   = [int(i) for i in timeToRun]
    turnsToTake = [int(i) for i in turnsToTake]
    
    commandList = []
    
    for i in turnsToTake:
        if i == 3 or i == 4:
            pass
        else:
            client.publish("outTopic/Errors/Router", "Turns Must Be 3 or 4")
            return
        
    if len(timeToRun) == len(turnsToTake):
        for i in range(len(timeToRun)):
            commandList.append([timeToRun[i],turnsToTake[i]])
            client.publish("outTopic/Errors/Router", "SuccesFully Parsed")
    else:
        client.publish("outTopic/Errors/Router", "Unequal Lengths Of Commands")
        return
    
    for i in range(len(commandList)):
        toRun = commandList[i][0]
        start = round(time.time() * 1000)
        end   = start + toRun
        
        toTurn = commandList[i][1]
        
        print("Move For:",toRun,"Milliseconds", "Started At:",start,"End At:",end,"Then Turn:",toTurn)
        
        while(round(time.time() * 1000) < end):
            moveForward()
            
        if toTurn == 3:
          moveRight()
          time.sleep_ms(1000)
          moveStop()
        else:
          moveLeft()
          time.sleep_ms(1000)
          moveStop()
        
        moveStop()

def toggleGetDistance(getDistance):
  getDistance = not getDistance
  return getDistance

def toggleAutoMovement(isAutoMovement,boundsDetectionStart):
  boundsDetectionStart = time.time()
  isAutoMovement = not isAutoMovement
  return isAutoMovement, boundsDetectionStart

