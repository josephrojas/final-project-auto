#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <FS.h>

IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 4, 9);
IPAddress subnet(255, 255, 255, 0);
String SSIDname = "ESPsoftAP_01";
String password = "MVPpassword1";
ESP8266WebServer webserver(80);
String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
String currentSensorData="";

//EN EL COM8

// Handle 404
void notfoundPage() {
  webserver.send(404, "text/plain", "404: Not found");
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  Serial.print("Estableciendo configuración Soft-AP... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Listo" : "Falló!");

  Serial.print("Estableciendo modo Soft-AP... ");
  Serial.println(WiFi.softAP(SSIDname, password, 6, false, 8) ? "Listo" : "Falló!");

  Serial.print("Dirección IP Soft-AP = ");
  Serial.println(WiFi.softAPIP());

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Start Web Server
  webserver.on("/data", HTTP_POST, handleSensorData);
  webserver.on("/", []() {                              // If the client requests any URI
    handleFileRead("/index.html");
  });
  webserver.on("/sensor", HTTP_GET, handleGetSensorData);
  webserver.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(webserver.uri()))                  // send it if it exists
      notfoundPage();// otherwise, respond with a 404 (Not Found) error
  });
  webserver.begin();

}

// Listen for HTTP requests
void loop(void) {
  webserver.handleClient();
}

void handleGetSensorData(void) {
  webserver.send(200, F("application/json"), currentSensorData);
}

void handleSensorData(void) {
  String postBody = webserver.arg("plain");
  Serial.println(postBody);

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, postBody);
  if (error) {
    // if the file didn't open, print an error:
    Serial.print(F("Error obteniendo JSON "));
    Serial.println(error.c_str());

    String msg = error.c_str();

    webserver.send(400, F("text/html"),
                   "Error in parsin json body! <br>" + msg);

  } else {
    JsonObject postObj = doc.as<JsonObject>();

    Serial.print(F("HTTP Method: "));
    Serial.println(webserver.method());

    if (webserver.method() == HTTP_POST) {
      if (postObj.containsKey("temperature") && postObj.containsKey("humidity")) {

        // Here store data or doing operation
        currentSensorData=postBody;
        // Create the response
        // To get the status of the result you can get the http status so
        // this part can be unusefully
        DynamicJsonDocument doc(512);
        doc["status"] = "OK";

        Serial.print(F("Stream..."));
        String buf;
        serializeJson(doc, buf);

        webserver.send(201, F("application/json"), buf);
        Serial.print(F("done."));

      } else {
        DynamicJsonDocument doc(512);
        doc["status"] = "KO";
        doc["message"] = F("No data found, or incorrect!");

        Serial.print(F("Stream..."));
        String buf;
        serializeJson(doc, buf);

        webserver.send(400, F("application/json"), buf);
        Serial.print(F("done."));
      }
    }
  }
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  Serial.print("El archivo existe?: ");
  Serial.println(SPIFFS.exists(path));
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = webserver.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}
