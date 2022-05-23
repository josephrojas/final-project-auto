#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


IPAddress local_ip(192, 168, 1, 3);                           //Modifica la dirección IP 
IPAddress gateway(192, 168, 1, 1);   
IPAddress subnet(255, 255, 0, 0);

String SSIDname = "ESPsoftAP_01";
String password = "MVPpassword1";
ESP8266WebServer webserver(80);
int ledPin = D6;

// Handle Root
void ledOn() {
  digitalWrite(ledPin, HIGH);   // Arduino: turn the LED on (HIGH)
  webserver.send(200, "text/plain", "ON");
}

void ledOff() {
 digitalWrite(ledPin, LOW);    // Arduino: turn the LED off (LOW)
 webserver.send(200, "text/plain", "OFF");
  }


// Handle 404
void notfoundPage() {
  webserver.send(404, "text/plain", "404: Not found");
}

void setup()
{
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println();
  if (!WiFi.config(local_ip, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(SSIDname, password);
  while (WiFi.status() != WL_CONNECTED)
   {   delay(500);
       Serial.print(".");
   }
   Serial.println();
   Serial.println("IP local:");
  Serial.println(WiFi.localIP());

   // Start Web Server
  webserver.on("/fan=on",ledOn);
  webserver.on("/fan=off",ledOff);
  webserver.onNotFound(notfoundPage);
  webserver.begin();

}

// Listen for HTTP requests
void loop(void) {
  webserver.handleClient();
}
