#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "realme";
const char* password = "@1234567";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

const int ldrPin = A0;           // LDR connected to analog pin A0
const int trigPin = D1;          // Ultrasonic Sensor Trig pin
const int echoPin = D2;          // Ultrasonic Sensor Echo pin
const int ledPin = D5;           // LED connected to digital pin D5
const int ldrThreshold = 500;    // Threshold for LDR
const int distanceThreshold = 400; // Threshold for ultrasonic distance in cm

long duration;
int distance;

void setup() {
  Serial.begin(115200);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int ldrValue = analogRead(ldrPin);
  int distance = measureDistance();
  
  // Print LDR value and distance to Serial Monitor
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  // Publish LDR and distance readings to MQTT
  String ldrValueStr = String(ldrValue);
  client.publish("sensor/ldr", ldrValueStr.c_str());
  
  String distanceStr = String(distance);
  client.publish("sensor/ultrasonic", distanceStr.c_str());

  // LED control logic based on new condition
  if (distance > distanceThreshold) {
    digitalWrite(ledPin, HIGH);   // Turn LED ON
    client.publish("device/ledStatus", "ON");
    Serial.println("LED Status: ON");
  } 
  else if (distance > distanceThreshold) {
    digitalWrite(ledPin, HIGH);   // Turn LED ON
    client.publish("device/ledStatus", "ON");
    Serial.println("LED Status: ON");
  }
  else if (ldrValue > ldrThreshold) {
    digitalWrite(ledPin, LOW);    // Turn LED OFF
    client.publish("device/ledStatus", "OFF");
    Serial.println("LED Status: OFF");
  }
  else if (distance < distanceThreshold) {
    digitalWrite(ledPin, LOW);    // Turn LED OFF
    client.publish("device/ledStatus", "OFF");
    Serial.println("LED Status: OFF");
  }
  
  
  delay(2000);  // Delay for stability
}

int measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;  // Convert to cm
  
  return distance;
}
