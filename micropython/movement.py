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

def moveRouter(route):
    import main
    #moveStop()
    
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
            main.client.publish("outTopic/Errors/Router", "Turns Must Be 3 or 4")
            return
        
    if len(timeToRun) == len(turnsToTake):
        for i in range(len(timeToRun)):
            commandList.append([timeToRun[i],turnsToTake[i]])
        main.client.publish("outTopic/Errors/Router", "SuccesFully Parsed")
    else:
        main.client.publish("outTopic/Errors/Router", "Unequal Lengths Of Commands")
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

def toggleGetDistance():
  import main
  main.getDistance = not main.getDistance
  if main.getDistance:
    main.client.publish("outTopic/Status/Distance", "1")
  else:
    main.client.publish("outTopic/Status/Distance", "0")
  
def toggleAutoMovement():
  import main
  main.boundsDetectionStart = time.time()
  main.isAutoMovement = not main.isAutoMovement

  if (not main.isAutoMovement):
    main.client.publish("outTopic/Status/AutoMovement", "0")
    moveStop()
  else:
    main.client.publish("outTopic/Status/AutoMovement", "1")
