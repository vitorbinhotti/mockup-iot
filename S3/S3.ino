#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"


WiFiClientSecure client;
PubSubClient mqtt(client);


void setup() {
  Serial.begin(115200);
  client.setInsecure();
  Serial.println("Conectando ao WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.print("\nConectado com Sucesso!");

  Serial.println("Conectando ao Broker...");
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  String BoardID = "S2";
  BoardID += String(random(0xffff, HEX));
  mqtt.connect(BoardID.c_str(), BROKER_USER, BROKER_PASS);
  while (!mqtt.connected()) {
    Serial.println(".");
    delay(200);
  }
  mqtt.subscribe(TOPIC_ILUM);
  mqtt.setCallback(callback);
  Serial.println("\nConectado ao Broker!");
}

void loop() {
  mqtt.publish(TOPIC_ILUM, "Acender");
  mqtt.loop();

  delay(1000);
}

void callback(char* topic, byte* payload, unsigned int length) {
    String msg = "";
    for (int i = 0;i < length; i++) {
    msg += (char) payload[i];
  }

  if (topic == TOPIC_ILUM && msg == "Acender") {
    digitalWrite(2, HIGH);
  } else if (topic == TOPIC_ILUM && msg == "Apagar") {
    digitalWrite(2,LOW);
  }
}