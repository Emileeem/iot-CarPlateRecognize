#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <FirebaseClient.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <Ultrasonic.h> // Biblioteca Ultrasonic

#define WIFI_SSID "TccSenai"
#define WIFI_PASSWORD "testandoaqui"

#define DATABASE_SECRET "14aO4okIScEw8Qlv6ojs009rX2Iv24jGxLEtkPxO" // Token de autenticação
#define DATABASE_URL "https://iot-tcc-ef560-default-rtdb.firebaseio.com/"

#define LED_VERDE 14
#define LED_VERM 12

Ultrasonic ultrassom(5, 4); // Pinos TRIG e ECHO
long distancia;
Servo meuservo;
String lastPlate = "";
int lastExists = 0;
unsigned long lastMoveTime = 0;

FirebaseApp app;
RealtimeDatabase Database;
AsyncResult result;
LegacyToken dbSecret(DATABASE_SECRET);
WiFiClientSecure ssl;
DefaultNetwork network;
AsyncClientClass client(ssl, getNetwork(network));

// Função para exibir erros
void printError(int code, const String &msg)
{
    Serial.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}

// Função para conectar ao Wi-Fi
void connect()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}

// Função para inicializar o Firebase
void initializeFirebase()
{
    // Initialize Firebase
    initializeApp(client, app, getAuth(dbSecret));
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);
    client.setAsyncResult(result);
}

// Função para atualizar os dados do Firebase
void refresh()
{
    connect();
    initializeFirebase();

    // Get values from Firebase
    Serial.print("Getting /plates/exists... ");
    bool status = Database.get<int>(client, "/plates/exists");
    if (status)
    {
        lastExists = Database.get<int>(client, "/plates/exists");
    }
    else
    {
        printError(client.lastError().code(), client.lastError().message());
    }

    Serial.print("Getting /plates/plate... ");
    String plateData = Database.get<String>(client, "/plates/plate");
    if (client.lastError().code() == 0)
    {
        lastPlate = plateData;
    }
    else
    {
        printError(client.lastError().code(), client.lastError().message());
    }

    WiFi.disconnect();
}

// Função para controlar o servo motor
void controlarServo()
{
    unsigned long currentTime = millis();

    if (lastExists == 1)
    {
        if (lastPlate != "")
        {
            meuservo.write(180);
            lastMoveTime = currentTime;
        }
        else
        {
            meuservo.write(0);
        }
    }
    else
    {
        meuservo.write(0);
        lastPlate = "";
    }

    if (meuservo.read() == 180 && (currentTime - lastMoveTime >= 3000))
    {
        meuservo.write(0);
    }
}

// Função para verificar a disponibilidade da vaga usando ultrassônico
void vagaDisponivel()
{
    delay(1000);
    distancia = ultrassom.read(); // Supondo que 'read' seja o método correto
    Serial.print("Distância = ");
    Serial.print(distancia);
    Serial.println("cm");
    if (distancia > 5)
    {
        digitalWrite(LED_VERDE, HIGH);
        digitalWrite(LED_VERM, LOW);
    }
    else
    {
        digitalWrite(LED_VERDE, LOW);
        digitalWrite(LED_VERM, HIGH);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    connect();
    delay(100);

    // Initialize Firebase
    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
    ssl.setInsecure();
    #if defined(ESP8266)
    ssl.setBufferSizes(1024, 1024);
    #endif

    initializeFirebase();

    meuservo.attach(23);
    meuservo.write(0);
    lastPlate = "";
    lastExists = 0;
    lastMoveTime = 0;
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_VERM, OUTPUT);
}

void loop()
{
    refresh();
    controlarServo();
    vagaDisponivel();
    delay(2000); // Atualiza a cada 2 segundos
}
