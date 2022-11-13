#include <WiFi.h>
#include <PubSubClient.h>

#define LDR_PIN 36 //LDR pada pin A0

const char* ssid = "REPLACE_WITH_SSID";
const char* pass = "REPLACE_WITH_PASS";

const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);

int ledPin = 0;
double lightIntensity = 0;
double averageLightIntensity = 0;

char messageData[20];

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

double readLDRData() {
  int ldrRawData = 4095 - analogRead(LDR_PIN);
  delay(1000);
  
  return (0.28661 * ldrRawData) - 201.27866;
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  
  for (int i=0; i<5; i++) {
    lightIntensity = lightIntensity + readLDRData();
  }

  averageLightIntensity = lightIntensity / 5;
//  messageData = String(averageLightIntensity, 4);
  sprintf(messageData, "%f", averageLightIntensity);
  client.publish("capstoneA16/lightIntensity", messageData);
}
