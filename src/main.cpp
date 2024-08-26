#include <WiFi.h>
#include <HTTPClient.h>
#include <Servo.h>

const char* ssid = "teste";
const char* password = "testandoaq";

const char* host = "http://127.0.0.1:5000/dados.json";

Servo meuservo;
int angulo = 0;
String lastPlate = ""; 
int lastExists = 0;    

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando...");
  }

  Serial.println("Conectado ao Wi-Fi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  meuservo.attach(9);
  meuservo.write(0); 
  lastPlate = "";
  lastExists = 0;
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) { // Verifica se está conectado ao Wi-Fi
    HTTPClient http;

    http.begin(host);  
    int httpCode = http.GET();  

    if (httpCode > 0) {  // Verifica se o pedido foi bem-sucedido
      String response = http.getString();

      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, response);

      if (error) {
        Serial.print("Falha ao analisar JSON: ");
        Serial.println(error.c_str());
        delay(10000); 
        return;
      }

      int exists = doc["exists"];
      const char* plate = doc["plate"];

      Serial.print("Exists: ");
      Serial.println(exists);
      Serial.print("Plate: ");
      if (plate == nullptr) {
        Serial.println("null");
      } else {
        Serial.println(plate);
      }

      if (String(plate) != lastPlate && exists == 1) {
        angulo = 180;  
        delay(8000);
        angulo = 0;
      } else {
        angulo = 0;    
      }

      meuservo.write(angulo);

      lastPlate = plate ? String(plate) : "";
      lastExists = exists;
    } else {
      Serial.print("Erro no HTTP: ");
      Serial.println(httpCode);
    }

    http.end();  
  } else {
    Serial.println("Desconectado do Wi-Fi");
  }

  delay(10000);
}
