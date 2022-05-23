#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>
#define DHTTYPE DHT11
#define dht_dpin D4
DHT dht(dht_dpin, DHTTYPE);

// EN EL COM7

// Set WiFi credentials
#define WIFI_SSID "ESPsoftAP_01"
#define WIFI_PASS "MVPpassword1"

//Your Domain name with URL path or IP address with path
String mainServer = "http://192.168.1.1/";
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long dataSendInterval = 2000;
//Static ip adress

IPAddress local_ip(192, 168, 1, 2);                           //Modifica la dirección IP 
IPAddress gateway(192, 168, 1, 10);   
IPAddress subnet(255, 255, 0, 0);

void setup()
{
  pinMode(LED_BUILTIN,OUTPUT);
  Serial.begin(115200);
  if (!WiFi.config(local_ip, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a la red WIFI con IP: ");
  Serial.println(WiFi.localIP());

  dht.begin();


}

void loop(void) {
  if ((millis() - lastTime) > dataSendInterval) {
    if (isWifiConnected()) {
      reportToMain();
    }
    lastTime = millis();
  }
}

bool isWifiConnected(void) {
  int status = WiFi.status();
  if (status != WL_CONNECTED) {
    Serial.println("Se desconectó el WIFI :(");
  }
  return status == WL_CONNECTED;
}

void reportToMain() {
  String data = readSensor();
  postToMainServer(data);
}

String readSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  String data = String("{ \"temperature\": ") + t + String(", \"humidity\": ") + h + String("}");
  Serial.println(data);
  return data;
}


void postToMainServer(String data) {
  WiFiClient client;
  HTTPClient http;

  String reportUrl = mainServer + "data";

  // Your Domain name with URL path or IP address with path
  http.begin(client, reportUrl.c_str());

  // Send HTTP POST request
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(data);
  
  if (httpResponseCode > 0) { 
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}
