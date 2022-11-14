#include <PubSubClient.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>

#define OUTER_PIR_PIN 34
#define INNER_PIR_PIN 35

#define WIFI_SSID "suastuti_3"
#define WIFI_PASS "notaristutiek"

#define MQTT_BROKER "192.168.1.4"
#define MQTT_PORT 1883
#define MQTT_TOPIC "capstoneA16/visitor"

int outerPirState = 0;
int innerPirState = 0;

unsigned int outerPirTime = 0;
unsigned int innerPirTime = 0;
unsigned int lastOuterPirTime = 0;
unsigned int lastInnerPirTime = 0;

unsigned int visitorCount = 0;
unsigned int lastVisitorCount = 0;

bool wifiIndState = false;

WiFiClient espClient;
PubSubClient client(espClient);

void handleOuterPir(void parameters) {
  for (;;) {
    outerPirState = digitalRead(OUTER_PIR_PIN);

    if (!outerPirState) {
      Serial.println("Motion detected on OUTER SENSOR");
      outerPirTime = millis();

      outerPirState = 1;
      vTaskDelay(500);
    }
  }
}

void handleInnerPir(void *parameters) {
  for (;;) {
    innerPirState = digitalRead(INNER_PIR_PIN);
    //            Serial.println(innerPirState);

    if (!innerPirState) {
      Serial.println("Motion detected on INNER SENSOR");
      innerPirTime = millis();

      innerPirState = 1;
      vTaskDelay(500);
    }
  }
}

void handleVisitorCount(void *parameters) {
  for (;;) {
    Serial.print("");
    if (innerPirTime != 0 && outerPirTime != 0) {
      int visitorState = innerPirTime - outerPirTime;
      if (visitorState > 0) {
        visitorCount++;

        Serial.print("Visitor count: ");
        Serial.println(visitorCount);
        Serial.println("Visitor in");
      } else {
        if (visitorCount > 0) {
          visitorCount--;

          Serial.print("Visitor count: ");
          Serial.println(visitorCount);
          Serial.println("Visitor out");
        }
      }
      vTaskDelay(500);
      innerPirTime = 0;
      outerPirTime = 0;
    }
  }
}

void publishToTopic(void *parameters) {
  for (;;) {
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect("ESP8266Client")) {
        if (lastVisitorCount != visitorCount) {
          while (!client.connected()) {
            reconnect();
          }
          client.publish("capstoneA16/visitor", "x");
          //                        vTaskDelay(1000);
          Serial.println("Published");
          lastVisitorCount = visitorCount;
        }
        Serial.println("connected");
      }
    }
    if (lastVisitorCount != visitorCount) {
      while (!client.connected()) {
        reconnect();
      }
      char msg_out[20];
      sprintf(msg_out, "%d", visitorCount);
      client.publish("capstoneA16/visitor", msg_out);
      //                vTaskDelay(1000);
      Serial.println("Published");
      lastVisitorCount = visitorCount;
    }
  }
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
      vTaskDelay(500);
    }
  }
}

void callback(char *topic, byte *message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  pinMode(OUTER_PIR_PIN, INPUT);
  pinMode(INNER_PIR_PIN, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  //
  //  pinMode(OUTER_PIR_PIN, INPUT);
  //  pinMode(INNER_PIR_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    wifiIndState = !wifiIndState;
    digitalWrite(LED_BUILTIN, wifiIndState);
    delay(100);
  }

  digitalWrite(LED_BUILTIN, HIGH);

  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);

  xTaskCreatePinnedToCore(handleOuterPir, "Handle outside sensor", 1000, NULL,
                          1, NULL, 1);

  xTaskCreatePinnedToCore(handleInnerPir, "Handle inside sensor", 1000, NULL, 1,
                          NULL, 1);

  xTaskCreatePinnedToCore(handleVisitorCount, "Handle visitor", 1000, NULL, 1,
                          NULL, 1);
  xTaskCreatePinnedToCore(publishToTopic, "Send data to mqtt", 5000, NULL, 1,
                          NULL, 1);
}

void loop() {}
