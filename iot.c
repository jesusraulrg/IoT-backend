#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const int ledPin = 2;
const int potPin = 34;

String firebaseHost = "iot-pro-22c31-default-rtdb.firebaseio.com";
String apiKey = "qz7ciSkViSwFwyiq9QyM4M7j5R90El5HeJCsQdAT";
String herokuServer = "https://iot-backend-restapi-458a9520ec9a.herokuapp.com/devices";
String serverNameLed = "https://" + firebaseHost + "/iot/1/value.json?auth=" + apiKey;
String serverNamePot = "https://" + firebaseHost + "/iot/2/value.json?auth=" + apiKey;

unsigned long lastTime = 0;
unsigned long timerDelay = 100;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  connectToWiFi();
  Serial.println("Actualización en tiempo real del potenciómetro.");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    int potValue = map(analogRead(potPin), 0, 4095, 0, 100);
    updateFirebase(serverNamePot, potValue);
    int ledState = (potValue > 0) ? 1 : 0;
    digitalWrite(ledPin, ledState);
    updateFirebase(serverNameLed, ledState);
    actualizarHeroku(1, ledState);
    actualizarHeroku(2, potValue);
    Serial.println("Potenciómetro: " + String(potValue) + "% | LED: " + String(ledState == 1 ? "encendido" : "apagado"));
  } else {
    Serial.println("WiFi Desconectado");
    connectToWiFi();
  }

  delay(100);
}

void connectToWiFi() {
  Serial.print("Conectando a WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    WiFi.begin(ssid, password);
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado a la red WiFi con dirección IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nNo se pudo conectar a la red WiFi después de varios intentos.");
  }
}

void updateFirebase(String serverName, int value) {
  HTTPClient http;
  http.begin(serverName.c_str());
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"value\":" + String(value) + "}";
  int httpResponseCode = http.PATCH(payload);
  Serial.print("HTTP Response code (Firebase): ");
  Serial.println(httpResponseCode);
  http.end();
}

void actualizarHeroku(int id, int value) {
  String herokuPath;
  if (id == 1) {
    herokuPath = herokuServer + "/1/" + value;
  } else if (id == 2) {
    herokuPath = herokuServer + "/2/" + value;
  } else {
    Serial.println("ID de dispositivo no reconocido");
    return;
  }

  HTTPClient http;
  http.begin(herokuPath.c_str());
  http.addHeader("Content-Type", "application/json");
  String payload = String(value);
  int httpResponseCode = http.PATCH(payload);
  Serial.print("HTTP Response code (Heroku): ");
  Serial.println(httpResponseCode);
  http.end();
}
