#include <WiFi.h>
#include <PubSubClient.h>

#define LDR_PIN 36 //LDR pada pin A0

const char* ssid = "REPLACE_WITH_SSID";
const char* pass = "REPLACE_WITH_PASS";

const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
}

double readLDRData() {
  int ldrRawData = 4095 - analogRead(LDR_PIN);
  delay(1000);
  
  return (0.1784 * ldrRawData) + 93.93325;
}

void loop() {
 readLDRData();
}
