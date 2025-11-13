#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"

#define LDR_PIN 34
#define LED_PIN 19  

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
}

void loop() {
  int valorLDR = analogRead(LDR_PIN);
  Serial.print("Valor do LDR: ");
  Serial.println(valorLDR);

  int limite = 3000; 

  if (valorLDR > limite) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("ambiente escuro");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("ambiente claro");
  }

  delay(1000);
}
