#include <PubSubClient.h>
#include <WiFi.h>

#define LDR_PIN 36  // LDR pada pin A0

const char* ssid = "suastuti_3";
const char* pass = "notaristutiek";

const char* mqtt_server = "192.168.1.4";

WiFiClient espClient;
PubSubClient client(espClient);

double lightIntensity = 0;
double averageLightIntensity = 0;

bool wifiIndState = false;

String messageData;

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

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    wifiIndState = !wifiIndState;
    digitalWrite(LED_BUILTIN, wifiIndState);
    delay(100);
  }

  digitalWrite(LED_BUILTIN, HIGH);
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
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
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

  return (0.1784 * ldrRawData) + 93.93325;
}
void loop() {
  for (int i = 0; i < 5; i++) {
    lightIntensity = lightIntensity + readLDRData();
  }

  averageLightIntensity = lightIntensity / 5;
  messageData = String(averageLightIntensity, 4);

  client.publish("capstoneA16/lightIntensity", messageData);
}
