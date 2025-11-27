#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"

WiFiClientSecure client;
PubSubClient mqtt(client);

// Pinos dos sensores ultrassônicos
const byte TRIGGER_PIN = 19;
const byte ECHO_PIN = 13;

const byte TRIGGER_PIN2 = 27;
const byte ECHO_PIN2 = 18;

// LED controlado pelo MQTT
const byte LED_PIN = 12;

unsigned long lastMsg = 0;
bool objetoProximoAnterior = false;

void conectaWiFi() {
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Aguarda até conectar ao WiFi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void conectaMQTT() {
  // Tenta conectar ao broker MQTT até conseguir
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");

    // Cria ID aleatório para o cliente
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Autenticação no servidor
    if (mqtt.connect(clientId.c_str(), BROKER_USER, BROKER_PASS)) {
      Serial.println("Conectado!");
      mqtt.subscribe(TOPIC_ILUM); // Ouve comandos para o LED
    } else {
      Serial.print("Falha (rc=");
      Serial.print(mqtt.state());
      Serial.println("), tentando novamente em 5s");
      delay(5000);
    }
  }
}

// Função que mede a distância usando um sensor ultrassônico
long lerDistancia(byte triggerPin, byte echoPin) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10); // Pulso para ativar o sensor
  digitalWrite(triggerPin, LOW);

  // Mede o tempo do pulso de retorno
  long duracao = pulseIn(echoPin, HIGH, 20000);
  if (duracao == 0) return -1; // Sem leitura (muito longe)

  // Converte o tempo para centímetros
  return (long) duracao * 0.034 / 2;
}

// Recebe mensagens do MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";

  // Converte payload para string
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("Mensagem recebida em ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);

  // Controle do LED pelo MQTT
  if (String(topic) == TOPIC_ILUM) {
    if (msg == "Acender") {
      digitalWrite(LED_PIN, HIGH);
    } else if (msg == "Apagar") {
      digitalWrite(LED_PIN, LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configura sensores e LED
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(TRIGGER_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  pinMode(LED_PIN, OUTPUT);

  client.setInsecure(); // Evita necessidade de certificado SSL
  conectaWiFi();

  mqtt.setServer(BROKER_URL, BROKER_PORT); // Configura servidor MQTT
  mqtt.setCallback(callback);               // Registra função de retorno
  conectaMQTT();
}

void loop() {
  // Reconecta MQTT caso perca conexão
  if (!mqtt.connected()) {
    conectaMQTT();
  }
  mqtt.loop(); // Mantém o MQTT ativo

  unsigned long agora = millis();
  if (agora - lastMsg > 2000) { // Executa a cada 2 segundos
    lastMsg = agora;

    // Lê os dois sensores ultrassônicos
    long distancia1 = lerDistancia(TRIGGER_PIN, ECHO_PIN);
    long distancia2 = lerDistancia(TRIGGER_PIN2, ECHO_PIN2);

    Serial.print("Sensor 1: ");
    Serial.print(distancia1);
    Serial.println(" cm");

    Serial.print("Sensor 2: ");
    Serial.print(distancia2);
    Serial.println(" cm");

    // Detecta objeto próximo e envia mensagem apenas uma vez
    if (distancia1 > 0 && distancia1 < 10) {
      if (!objetoProximoAnterior) {
        mqtt.publish(TOPIC_ILUM, "Objeto proximo no sensor 1");
        objetoProximoAnterior = true;
      }
    } else if (distancia2 > 0 && distancia2 < 10) {
      if (!objetoProximoAnterior) {
        mqtt.publish(TOPIC_ILUM, "Objeto proximo no sensor 2");
        objetoProximoAnterior = true;
      }
    } else {
      // Reseta para permitir novo envio depois que o objeto sai
      objetoProximoAnterior = false;
    }
  }
}