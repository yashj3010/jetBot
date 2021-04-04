
// Libraries
#include <WiFi.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>

// Defines

#define enable 32
#define bit0 33
#define bit1 25

#define servoDownPin 21
#define servoUpPin 22
#define servoDistancePin 23

#define trigPin 2
#define echoPin 5

#define MSG_BUFFER_SIZE  (50)


// consts

const char* ssid = "Node 0";
const char* password = "yashj1030";
const char *mqtt_server = "192.168.0.106";

// instances
WiFiClient espClient;
PubSubClient client(espClient);

Servo servoDown;
Servo servoUp;
Servo servoDistance;

// variables

unsigned long lastMsg = 0;
unsigned long lastMsgDistance = 0;

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

// main code block

void moveBackward() {
  digitalWrite(bit0, HIGH);
  digitalWrite(bit1, HIGH);
  digitalWrite(enable, HIGH);
}

void moveForward() {
  digitalWrite(bit0, LOW);
  digitalWrite(bit1, LOW);
  digitalWrite(enable, HIGH);
}

void moveRight() {
  digitalWrite(bit0, LOW);
  digitalWrite(bit1, HIGH);
  digitalWrite(enable, HIGH);
}

void moveLeft() {
  digitalWrite(bit0, HIGH);
  digitalWrite(bit1, LOW);
  digitalWrite(enable, HIGH);
}

void moveStop() {
  digitalWrite(enable, LOW);
}

void lookRight() {
  servoDistance.write(60);
  delay(300);

}

void look90() {
  servoDistance.write(90);
}

void lookLeft() {
  servoDistance.write(120);
  delay(300);
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

void callback(char* topic, byte* payload, unsigned int length) {
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
      getDistance = !getDistance;
    }
    if ((char)payload[0] == '7') {
      isAutoMovement = !isAutoMovement;
      getDistance = (isAutoMovement == true) ? true : false;

      if (!isAutoMovement) {
        moveStop();
      }

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

  pinMode(bit0, OUTPUT);
  pinMode(bit1, OUTPUT);
  pinMode(enable, OUTPUT);

  pinMode(servoDownPin, OUTPUT);
  pinMode(servoUpPin, OUTPUT);
  pinMode(servoDistancePin, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  servoDown.attach(servoDownPin);
  servoUp.attach(servoUpPin);
  servoDistance.attach(servoDistancePin);

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
    if (now - lastMsgDistance > 250) {
      lastMsgDistance = now;
      look90();
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      duration = pulseIn(echoPin, HIGH);

      lastDistance = distance;
      distance = duration * 0.034 / 2;

      Serial.print("Distance: ");
      Serial.println(distance);
      client.publish("outTopic/Distance", PackIntData(distance, lightchar));
    }
  }

  if (isAutoMovement) {
    if (distance < 35) {
      moveStop();
      lookRight();
      lastMsgDistance = now;
      Serial.print("inside less than 12: ");
      Serial.println(distance);
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      duration = pulseIn(echoPin, HIGH);

      rightDistance = duration * 0.034 / 2;

      lookLeft();

      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      duration = pulseIn(echoPin, HIGH);

      leftDistance = duration * 0.034 / 2;

      Serial.println("rightDistance:");
      Serial.println(rightDistance);

      Serial.println("leftDistance:");
      Serial.println(leftDistance);

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
      }
    }
    else if (distance > 35)
    {
      moveForward();
    }
  }


  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    client.publish("outTopic", msg);
  }
}
