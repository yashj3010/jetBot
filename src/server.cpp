
// Libraries
#include <iostream>
#include <string>

#include <WiFi.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>

// Defines

#define ENABLE 32
#define BIT0   33
#define BIT1   25

#define SERVO_DOWN_PIN     21
#define SERVO_UP_PIN       22
#define SERVO_DISTANCE_PIN 23

#define TRIGGER_PIN 2
#define ECHO_PIN 5

#define MSG_BUFFER_SIZE (50)

#define COLLISION_DISTANCE 45

#define LOOK_RIGHT_ANGLE    60
#define LOOK_STRAIGHT_ANGLE 90
#define LOOK_LEFT_ANGLE     120

#define MESSAGE_PUBLISH_TIME 2000
#define DISTANCE_SAMPLE_TIME 250

#define MOVE_FORWARD_CALLBACK        "1"
#define MOVE_BACKWARD_CALLBACK       "2"
#define MOVE_RIGHT_CALLBACK          "3"
#define MOVE_LEFT_CALLBACK           "4"
#define MOVE_STOP_CALLBACK           "5"
#define TOGGLE_GETDISTANCE_CALLBACK  "6"
#define TOGGLE_AUTOMOVEMENT_CALLBACK "7"

// consts

const char* ssid = "Node 0";
const char* password = "yashj1030";
const char *mqtt_server = "192.168.0.113";

// instances
WiFiClient espClient;
PubSubClient client(espClient);

Servo servoDown;
Servo servoUp;
Servo servoDistance;

// variables

unsigned long lastMsg = 0;
unsigned long lastMsgDistance = 0;
unsigned long initialTime = 0;
unsigned long endTime = 0;

int boundsDetectionStart = 0;
int xEnd = 0;


char lightchar[50];
char msg[MSG_BUFFER_SIZE];

int value = 0;
int pos = 0;

int distance;
int lastDistance;

int rightDistance;
int leftDistance;

long duration;

String inputMsg;

bool isAutoMovement = false;
bool getDistance = false;
bool isOnTimer = false;


// main code block

void moveBackward() {
  digitalWrite(BIT0, HIGH);
  digitalWrite(BIT1, HIGH);
  digitalWrite(ENABLE, HIGH);
}

void moveForward() {
  digitalWrite(BIT0, LOW);
  digitalWrite(BIT1, LOW);
  digitalWrite(ENABLE, HIGH);
}

void moveRight() {
  digitalWrite(BIT0, LOW);
  digitalWrite(BIT1, HIGH);
  digitalWrite(ENABLE, HIGH);
}

void moveLeft() {
  digitalWrite(BIT0, HIGH);
  digitalWrite(BIT1, LOW);
  digitalWrite(ENABLE, HIGH);
}

void moveStop() {
  digitalWrite(ENABLE, LOW);
}

void lookRight() {
  servoDistance.write(LOOK_RIGHT_ANGLE);
  delay(300);

}

void lookAtAngle(int angle) {
  servoDistance.write(angle);
}

void lookLeft() {
  servoDistance.write(LOOK_LEFT_ANGLE);
  delay(300);
}

void toggleGetDistance() {
  getDistance = !getDistance;

  if (getDistance) {
    client.publish("outTopic/Status/Distance", "1");
  }
  else {
    client.publish("outTopic/Status/Distance", "0");
  }
}

void toggleAutoMovement() {
  boundsDetectionStart = millis();
  isAutoMovement = !isAutoMovement;
  getDistance = (isAutoMovement == true) ? true : false;

  if (!isAutoMovement) {
    client.publish("outTopic/Status/AutoMovement", "0");
    client.publish("outTopic/Status/Distance", "0");
    moveStop();
  }
  else {
    client.publish("outTopic/Status/AutoMovement", "1");
    client.publish("outTopic/Status/Distance", "1");
  }
}
void powerStatus() {
  client.publish("outTopic/Status/Alive", "1");
}

void movementTimer() {
  unsigned long now = millis();
  if (now >= endTime)
  {
    Serial.println("STOPPED");
    // toggleAutoMovement();
    isOnTimer = false;
    Serial.println(isAutoMovement);
  }
  else {
    Serial.println("KEEP ON GOING");
    moveForward();
  }

}

int getDistanceValue(int angle) {
  // lookAtAngle(angle);
  servoDistance.write(angle);
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);

  lastDistance = distance;
  distance = duration * 0.034 / 2;

  // Serial.print("Distance: ");
  // Serial.println(distance);
  client.publish("outTopic/Distance", PackIntData(distance, lightchar));
  return distance;
}

char* PackIntData(int a , char b[]) {
  String pubString =  String(a);
  pubString.toCharArray(b, pubString.length() + 1);
  return b;
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte * payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]");
  Serial.print("\n");

  inputMsg = "";
  int lengthofmsg = (int)length;
  for (int i = 0; i < lengthofmsg; i++)
  {
    inputMsg += ((char)payload[i]);
  }

  Serial.println("Full Input Msg");
  Serial.print(inputMsg);

  if (strcmp(topic, "inTopic") == 0)
  {
    Serial.print((char)payload[0]);
    Serial.print("\n");

    if ((char)payload[0] == '1') {
      moveForward();
    }

    if ((char)payload[0] == '2') {
      moveBackward();
    }

    if ((char)payload[0] == '3') {
      moveRight();
    }

    if ((char)payload[0] == '4') {
      moveLeft();
    }

    if ((char)payload[0] == '5') {
      moveStop();
    }
    if ((char)payload[0] == '6') {
      toggleGetDistance();
    }
    if ((char)payload[0] == '7') {
      toggleAutoMovement();
    }
    if ((char)payload[0] == '8') {

      char dur[2];
      dur[0] = (char)payload[1];
      dur[1] = (char)payload[2];

      initialTime = millis();
      duration = atoi(dur);
      duration *= 1000;
      endTime = initialTime + duration;

      // Serial.print(duration);
      // Serial.println("duration");
      // Serial.print(endTime);
      // Serial.println("endTime");

      toggleAutoMovement();
      isOnTimer = true;
      movementTimer();
    }

  }

  if (strcmp(topic, "servoDown") == 0)
  {
    pos = inputMsg.toInt();
    Serial.println("ServoDown");
    servoDown.write(pos);
  }

  if (strcmp(topic, "servoUp") == 0)
  {
    pos = inputMsg.toInt();
    servoUp.write(pos);
  }

  if (strcmp(topic, "servoDistance") == 0)
  {
    pos = inputMsg.toInt();
    servoDistance.write(pos);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
      client.subscribe("servoDown");
      client.subscribe("servoUp");
      client.subscribe("servoDistance");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BIT0, OUTPUT);
  pinMode(BIT1, OUTPUT);
  pinMode(ENABLE, OUTPUT);

  pinMode(SERVO_DOWN_PIN, OUTPUT);
  pinMode(SERVO_UP_PIN, OUTPUT);
  pinMode(SERVO_DISTANCE_PIN, OUTPUT);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  servoDown.attach(SERVO_DOWN_PIN);
  servoUp.attach(SERVO_UP_PIN);
  servoDistance.attach(SERVO_DISTANCE_PIN);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();

  if (getDistance) {
    if (now - lastMsgDistance > DISTANCE_SAMPLE_TIME) {
      lastMsgDistance = now;
      getDistanceValue(LOOK_STRAIGHT_ANGLE);
    }
  }

  if (!isAutoMovement) {
    client.publish("outTopic/Status/AutoMovement", "0");
    client.publish("outTopic/Status/Distance", "0");
    moveStop();
  }
  else {
    client.publish("outTopic/Status/AutoMovement", "1");
    client.publish("outTopic/Status/Distance", "1");
  }

  if (isAutoMovement) {
    if (isOnTimer) {
      movementTimer();
    }

    if (distance < COLLISION_DISTANCE) {

      client.publish("outTopic/Collision/Status", "1");
      client.publish("outTopic/Collision/Distance", PackIntData(distance, lightchar));

      xEnd = millis() - boundsDetectionStart;
      client.publish("outTopic/Bounds/X", PackIntData(xEnd, lightchar));

      moveStop();
      rightDistance = getDistanceValue(LOOK_RIGHT_ANGLE);
      delay(250);
      leftDistance = getDistanceValue(LOOK_LEFT_ANGLE);
      delay(250);
      lookAtAngle(LOOK_STRAIGHT_ANGLE);

      if (rightDistance > leftDistance ) {
        moveRight();
        delay(1000);
        moveStop();
      }
      else if (rightDistance < leftDistance)
      {
        moveLeft();
        delay(1000);
        moveStop();
      }
      else {
        moveBackward();
        client.publish("outTopic/Collision/Distance", "ERROR 1");
      }
    }
    else if (distance >= COLLISION_DISTANCE)
    {
      lookAtAngle(LOOK_STRAIGHT_ANGLE);
      client.publish("outTopic/Collision/Status", "0");
      client.publish("outTopic/Collision/Distance", "");
      moveForward();
    }
  }
  else if (!isAutoMovement) {
    moveStop();
    lookAtAngle(LOOK_STRAIGHT_ANGLE);
    client.publish("outTopic/Collision/Distance", "ERROR 2");
  }
  else{
    client.publish("outTopic/Collision/Distance", "ERROR 3");
  }

  if (now - lastMsg > MESSAGE_PUBLISH_TIME) {
    lastMsg = now;
    powerStatus();
  }
}