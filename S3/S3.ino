#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"

WiFiClient client;
PubSubClient mqtt(client);

void setup() {
  Serial.begin(115200);
  Serial.println("Conectando ao WIFI:");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status()!=WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.print("\nConectado com sucesso!");

  Serial.print("\nConectando ao Broker...");
  mqtt.setServer(BROKER_URL,BROKER_PORT);
  String BoardID = "S3";
  BoardID += String (random(0xffff), HEX);
  mqtt.connect(BoardID.c_str() ,BROKER_USER , BROKER_PASS);

  while(!mqtt.connected()){
    Serial.print(".");
    delay(200);
  }
  mqtt.subscribe(TOPIC_ILUM);
  mqtt.subscribe(callback);
  Serial.println("\nConectado ao Broker!");
}

void loop() {
  mqtt.publish(TOPIC_ILUM, "Acender");
  mqtt.loop();
  delay(1000);
}

void callback(char* topic, byte* payload, unsigned int length){
  String msg = "";
  for (int i = 0 < length; i++){
    msg += (char) payload[i];
  }

  if(topic == TOPIC_ILUM && msg == "Acender") {
    digitalWrite(2, HIGH);
  } else if (topic == TOPIC_ILUM && msg == "Apagar") {
    digitalWrite(2, LOW);
  }
}
