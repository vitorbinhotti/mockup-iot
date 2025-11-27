#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"
#include <ESP32Servo.h>

//CRIA CLIENTE  E USA PARA RECEBER E ENVIAR MENSAGEM
WiFiClientSecure client;
PubSubClient mqtt(client);

//lIGAR OS SERVOS NOS PINOS E NOMEAR
const byte SERVO_PIN = 18;
Servo Servo1;

const byte SERVO_PIN2 = 19;
Servo Servo2;

const byte TRIGGER_PIN = 25;
const byte ECHO_PIN = 26;



void setup() {
  Serial.begin(115200);


  Servo1.attach(SERVO_PIN);
  Servo2.attach(SERVO_PIN2);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

//DIZ AO CLIENTE HTTPS ACEITAR CONEXÃO ACEITAR CONEXÃO SEM VERIFICAÇÃO
  client.setInsecure();
  Serial.println("Conectando ao WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.print("\nConectado com Sucesso!");
  Serial.println("Conectando ao Broker...");

  //DEFINE ONDE ESTA O SERVIDOR MQTT E QUAL FUNÇÃO DEVE SER CHAMADO
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);
  String BoardID = "S2";
  BoardID += String(random(0xffff, HEX));
  mqtt.connect(BoardID.c_str(), BROKER_USER, BROKER_PASS);
  while (!mqtt.connected()) {
    Serial.println(".");
    delay(200);
  }
  
  Serial.println("\nConectado ao Broker!");

  //A PLACA DIZ QUE QUER OUVIR MENSAGEM DESSES CANAIS
  mqtt.subscribe(TOPIC_ILUM);
  mqtt.subscribe(TOPIC_PRESENCA1);
  mqtt.subscribe(TOPIC_PRESENCA2);
  mqtt.subscribe(TOPIC_PRESENCA3);
}



long lerDistancia() {

  //PINO NIVEL BAIXO, SILENCIAR POR 2 MICROSEGUNDOS
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  //MANDA IMPULSO DE 10 MICROSEGUNDOS E DEPOIS VOLTA PARA LOW
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

//A PLACA ESPERA O ECHO FICAR HIGH E QUANTO TEMPO LEVA PARA ISSO
  long duracao = pulseIn(ECHO_PIN, HIGH);
  long distancia = duracao * 349.24 / 2 / 10000;

  return distancia;
}



void loop() {

  mqtt.loop();

  long distancia = lerDistancia();

  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  if (distancia < 10) {
    Serial.println("Objeto próximo!");
    mqtt.publish(TOPIC_DISTANCIA, "Objeto próximo!");
  }

  delay(1000);

}

//INICIO DA FUNÇÃO QUANDO ESTÁ CONECTADO
void callback(char* topic, byte* payload, unsigned int length) {
  
  //MANDA DADOS EM BYTES E TRANSFORMA EM TEXTO LEGIVEL
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

//MOSTRA QUAL TOPICO CHEGOU E QUAL FOI A MENSAGEM
Serial.print("TOPICO: "); Serial.println(topic);
  Serial.print("MSG: "); Serial.println(msg);

//SE A MENSAGEM VEIO DO ILUMINAÇÃO, ACENDE PINO LED 2, SENAO DESLIGA
  if (strcmp(topic, TOPIC_ILUM) == 0) {
    if (msg == "ligar") digitalWrite(2, HIGH);
    if (msg == "desligar") digitalWrite(2, LOW);

    //SE A MENSAGEM VEIO DO TOPICO_PRE1, A MENSAGEM "SERVO1" FAZ O SERVO GIRAR PARA 0°
  }else if (strcmp(topic, TOPIC_PRESENCA1) == 0) {
    if (msg == "servo1") {
      Servo1.write(0);
      Serial.println("Servo1 → posição 1 (0°)");
    }

    //MESMA COISA DO ANTERIOR
  }else if (strcmp(topic, TOPIC_PRESENCA2) == 0) {
    if (msg == "servo2") {
      Servo2.write(0);
      Serial.println("Servo2 → posição 1 (0°)");
    }

    //SE A MENSAGEM CHEGOU NO TOPICO_PRE3, ELE GIRA O SERVO 1 E 2 PARA 90°
  }else if (strcmp(topic, TOPIC_PRESENCA3) == 0) {
    if (msg == "abrir") {
      Servo1.write(90);
      Servo2.write(90);
      Serial.println("Servo1 e Servo2 → posição 2 (90°)");
    }
  }
}
