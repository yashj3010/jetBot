
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

char lightchar[50];
char msg[MSG_BUFFER_SIZE];

int value = 0;
int pos = 0;
int distance;

long duration;

String inputMsg;


// main code block

char* PackIntData(int a , char b[]) {
  String pubString =  String(a);
  pubString.toCharArray(b, pubString.length() + 1);
  return b;
}

void getDistance() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance = duration * 0.034 / 2;

  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  client.publish("outTopic/Distance", PackIntData(distance, lightchar));

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
      Serial.print("1");
      digitalWrite(bit0, HIGH);
      digitalWrite(bit1, HIGH);
      digitalWrite(enable, HIGH);

    }

    if ((char)payload[0] == '2') {
      Serial.print("2");
      digitalWrite(bit0, LOW);
      digitalWrite(bit1, LOW);
      digitalWrite(enable, HIGH);
    }

    if ((char)payload[0] == '3') {
      Serial.print("3");
      digitalWrite(bit0, LOW);
      digitalWrite(bit1, HIGH);
      digitalWrite(enable, HIGH);

    }

    if ((char)payload[0] == '4') {
      Serial.print("4");
      digitalWrite(bit0, HIGH);
      digitalWrite(bit1, LOW);
      digitalWrite(enable, HIGH);
    }

    if ((char)payload[0] == '5') {
      Serial.print("5");
      digitalWrite(enable, LOW);
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

  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    client.publish("outTopic", msg);
    getDistance();
  }
}
