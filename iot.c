#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const int ledPin = 2;  // Pin para controlar el LED
const int potPin = 34; // Pin para leer el potenciómetro

String firebaseHost = "iot-pro-22c31-default-rtdb.firebaseio.com";
String apiKey = "qz7ciSkViSwFwyiq9QyM4M7j5R90El5HeJCsQdAT";
String herokuServer = "https://iot-back-a2f4e3309e8d.herokuapp.com/devices";
String serverNameLed = "https://" + firebaseHost + "/iot/1/value.json?auth=" + apiKey;
String serverNamePot = "https://" + firebaseHost + "/iot/2/value.json?auth=" + apiKey;

unsigned long lastTime = 0;
unsigned long timerDelay = 100; // Intervalo de tiempo para consultar el estado en milisegundos (100 milisegundos)

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  connectToWiFi();
  Serial.println("Actualización en tiempo real del potenciómetro.");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Consulta el valor del potenciómetro
    int potValue = map(analogRead(potPin), 0, 4095, 0, 100);
    updateFirebase(serverNamePot, potValue);

    // Actualiza el estado del LED basado en el potenciómetro
    int ledState = (potValue > 0) ? 1 : 0;
    digitalWrite(ledPin, ledState);
    updateFirebase(serverNameLed, ledState);

    // Después de actualizar en Firebase, también actualiza en Heroku
    actualizarHeroku(1, ledState);
    actualizarHeroku(2, potValue);

    Serial.println("Potenciómetro: " + String(potValue) + "% | LED: " + String(ledState == 1 ? "encendido" : "apagado"));
  } else {
    Serial.println("WiFi Desconectado");
    // Intentar reconexión si se pierde la conexión
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

  // Crea la carga útil JSON
  String payload = "{\"value\":" + String(value) + "}"; // Utiliza un objeto JSON con una clave "value"

  // Realiza la solicitud PATCH con la carga útil JSON
  int httpResponseCode = http.PATCH(payload);

  // Muestra el resultado de la solicitud HTTP
  Serial.print("HTTP Response code (Firebase): ");
  Serial.println(httpResponseCode);

  // Cierra la conexión HTTP
  http.end();
}

void actualizarHeroku(int id, int value) {
  String herokuPath;

  // Verifica el ID del dispositivo y construye la ruta correspondiente
  if (id == 1) {
    // Ruta para actualizar el valor del LED
    herokuPath = herokuServer + "/1/" + value;
  } else if (id == 2) {
    // Ruta para actualizar el valor del potenciómetro
    herokuPath = herokuServer + "/2/" + value;
  } else {
    // Manejar cualquier otro caso según tus necesidades
    Serial.println("ID de dispositivo no reconocido");
    return;
  }

  HTTPClient http;
  http.begin(herokuPath.c_str());

  // Configura el tipo de contenido para la carga útil JSON
  http.addHeader("Content-Type", "application/json");

  // Crea la carga útil JSON
  String payload = String(value);

  // Realiza la solicitud PATCH con la carga útil JSON
  int httpResponseCode = http.PATCH(payload);

  // Muestra el resultado de la solicitud HTTP
  Serial.print("HTTP Response code (Heroku): ");
  Serial.println(httpResponseCode);

  // Cierra la conexión HTTP
  http.end();
}
