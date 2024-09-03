#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "TccSenai"
#define WIFI_SENHA "testandoaqui"

#define FB_HOST "https://iot-tcc-ef560-default-rtdb.firebaseio.com/"
#define FB_AUTH "14aO4okIScEw8Qlv6ojs009rX2Iv24jGxLEtkPxO" // Token de autenticação

Servo meuservo; 
int angulo = 0;
String lastPlate = ""; 
int lastExists = 0;
unsigned long lastMoveTime = 0; 

int FBExists;
String FBPlates;

FirebaseJson json;      
FirebaseData firebaseData;

void connect() {
    WiFi.begin(WIFI_SSID, WIFI_SENHA);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000); 
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
}

void refresh() {
    connect();

    Firebase.get(firebaseData, "/plates/exists");
    FBExists = firebaseData.intData();

    Firebase.get(firebaseData, "/plates/plate");
    FBPlates = firebaseData.stringData();

    WiFi.disconnect();
}

void controlarServo() {
    unsigned long currentTime = millis();
    
    if (FBExists == 1) {
        if (FBPlates != lastPlate) {
            meuservo.write(180);
            lastMoveTime = currentTime;
            lastPlate = FBPlates;
        } else {
            meuservo.write(0);
        }
    } else {
        meuservo.write(0);
        lastPlate = ""; 
    }

    if (meuservo.read() == 180 && (currentTime - lastMoveTime >= 3000)) {
        meuservo.write(0);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    connect();
    delay(100);
    Firebase.begin(FB_HOST, FB_AUTH);
    Firebase.reconnectWiFi(true);
    Firebase.setReadTimeout(firebaseData, 1000 * 60);
    Firebase.setWriteSizeLimit(firebaseData, "tiny");
    WiFi.disconnect();

    meuservo.attach(18);
    meuservo.write(0); 
    lastPlate = "";
    lastExists = 0;
    lastMoveTime = 0;
}

void loop() {
    refresh(); 
    controlarServo(); 
    delay(2000); // Atualiza a cada 2 segundos
}
