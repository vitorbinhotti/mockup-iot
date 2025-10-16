#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient client;
PubSubClient mqtt(client);

const String BrokerURL = "test.mosquitto.org";     //endereço do broker público
const int BrokerPort = 1883;                       //porta do broker púlbico
const String BrokerUser = "";                      //usuário do servidor
const String BrokerPass = "";                      //senha do usuário

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

void setup() {
  Serial.begin(115200);
  Serial.println("Conectando ao WiFi");  //apresanta essa msg na tela
  WiFi.begin(SSID, PASS);                //tenta conectar na rede
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");  //mostra ".........."
    delay(200);
  }
  Serial.println("\nConectado com Sucesso!");

  Serial.println("Conectando ao Broker...");
  mqtt.setServer(BrokerURL.c_str(),BrokerPort);
  String BoardID = "Trem";
  BoardID += String(random(0xffff), HEX);
  mqtt.connect(BoardID.c_str() , BrokerUser.c_str() , BrokerPass.c_str());
  while(!mqtt.connected()){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado ao Broker!");
}

void loop() {
}
