#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"
#include "DHT.h"

// ---------------------
// DEFINIÇÕES DE PINOS
// ---------------------
#define LDR_PIN 34
#define LED_PIN 19  
#define DHTPIN 4
#define DHTTYPE DHT11
#define TRIGGER_PIN 22
#define ECHO_PIN 23

// ---------------------
// OBJETOS
// ---------------------
WiFiClientSecure client;
PubSubClient mqtt(client);
DHT dht(DHTPIN, DHTTYPE);

// ---------------------
// WIFI
// ---------------------
void conectaWiFi() {
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ---------------------
// MQTT
// ---------------------
void conectaMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), BROKER_USER, BROKER_PASS)) {
      Serial.println("Conectado!");
      mqtt.subscribe(TOPIC_ILUM);
    } else {
      Serial.print("Falha (rc=");
      Serial.print(mqtt.state());
      Serial.println("), tentando novamente em 5s");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// ---------------------
// SENSOR ULTRASSÔNICO
// ---------------------
long lerDistancia() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH);
  long distancia = duracao * 349.24 / 2 / 10000;

  return distancia;
}

// ---------------------
// SETUP
// ---------------------
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  dht.begin();

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  client.setInsecure();
  conectaWiFi();

  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);
  conectaMQTT();
}

// ---------------------
// LOOP
// ---------------------
void loop() {
  if (!mqtt.connected()) {
    conectaMQTT();
  }
  mqtt.loop();

  // ---------------------
  // LDR + LED
  // ---------------------
  int valorLDR = analogRead(LDR_PIN);
  int limite = 3000;

  Serial.println("========== LDR ==========");
  Serial.print("Valor: ");
  Serial.println(valorLDR);

  if (valorLDR > limite) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Status: AMBIENTE ESCURO");
      mqtt.publish(TOPIC_ILUM, "Status: AMBIENTE ESCURO");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Status: AMBIENTE CLARO");
      mqtt.publish(TOPIC_ILUM, "Status: AMBIENTE CLARO");
  }
  Serial.println();

  delay(800);

  // ---------------------
  // DHT11
  // ---------------------
  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();

  Serial.println("======== DHT11 ==========");
  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("ERRO na leitura");
  } else {
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println("%");

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");
    
  }
  Serial.println();
  mqtt.publish(TOPIC_UMID, String(umidade).c_str());
  mqtt.publish(TOPIC_TEMP, String(temperatura).c_str());

  delay(800);

  // ---------------------
  // ULTRASSÔNICO
  // ---------------------
  long distancia = lerDistancia();

  Serial.println("====== ULTRASSÔNICO ======");
  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  if (distancia < 10) {
    Serial.println("Objeto próximo!");
  }
  Serial.println("\n---------------------------\n");
  mqtt.publish(TOPIC_DIST, String(distancia).c_str());

  delay(800);


}

