#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define SDA_PIN D4 // Pino SDA
#define SCL_PIN D5 // Pino SCL

#ifndef STASSID
#define STASSID "ZORO_2G"
#define STAPSK "senha123"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

const IPAddress local_IP(192, 168, 0, 50);
const IPAddress gateway(192, 168, 0, 1);
const IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(59999);
Adafruit_BMP280 bmp; // Criar objeto do sensor

void retornaTemperaturaEmJson() {
  float temperatura = lerTemperatura();
  String jsonResponse = "{";
  jsonResponse += "\"temperature\": \"" + String(temperatura) + "\"";
  jsonResponse += "}";
  server.send(200, "application/json", jsonResponse);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

float lerTemperatura() {
  return bmp.readTemperature();
}

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.println("");

  Wire.begin(SDA_PIN, SCL_PIN);
  
  if (!bmp.begin(0x76)) {
    Serial.println("Erro ao encontrar o BMP280. Verifique a conexão!");
    while (1);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/temperatura", retornaTemperaturaEmJson);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
