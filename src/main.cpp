#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <FirebaseESP32.h>

// Credenciais Wi-Fi
#define WIFI_SSID = "TccSenai";
#define WIFI_SENHA = "testandoaqui";

// Configurações do Firebase
#define FB_HOST "https://iot-tcc-ef560-default-rtdb.firebaseio.com/"
#define FB_AUTH "14aO4okIScEw8Qlv6ojs009rX2Iv24jGxLEtkPxO" // Token de autenticação

// Servo
Servo meuservo; 
int angulo = 0;
String lastPlate = ""; 
int lastExists = 0;

//Variaveis Globais 
int FBExists;
char FBPlates;

void connect()
{
    WiFi.begin(WIFI_SSID, WIFI_SENHA);
    while (WiFi.status() != WL_CONNECTED);
}

void refresh() {
	connect();

	Firebase.get(firebaseData, "/plates/exists");
	FBExists = firebaseData.intData();

	Firebase.get(firebaseData, "/plates/exists");
	FBPlates = FirebaseData.charData();

	WiFi.disconnect();
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    connect();
    delay(100);
    Firebase.begin(FB_HOST, FB_AUTH);
    Firebase.reconnectWiFi(true);
    Firebase.setReadTimeout(firebaseData, 1000 * 60);
    Firebase.setwriteSizeLimit(firebaseData, "tiny");
    WiFi.disconnect();

    meuservo.attach(18);
    meuservo.write(0); 
    lastPlate = "";
    lastExists = 0;
}

void loop() {
    refresh(); 
    delay(2000); // Atualiza a cada 2 segundos
	Firebase.updateNode(firebaseData, "/plates", json);
}
