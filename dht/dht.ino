
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <DHT.h>


DHT dht0(5, DHT22); // D1 pin
DHT dht1(4, DHT21); // D2 pin

float dht0_t;
float dht0_h;

float dht1_t;
float dht1_h;


long lastTemp;
unsigned long previousMillis = 0;
const int measureTempFrequency = 5000; 

//------------------------------------------
//WIFI
const char* ssid = "SSID";
const char* password = "password";

//------------------------------------------
//HTTP
ESP8266WebServer server(80);


String GenerateMetrics() {
  String message = "";


  char dht0TString[6];
  char dht0HString[6];
  
  char dht1TString[6];
  char dht1HString[6];
  
  dtostrf(dht0_t, 2, 2, dht0TString);
  dtostrf(dht0_h, 2, 2, dht0HString);

  dtostrf(dht1_t, 2, 2, dht1TString);
  dtostrf(dht1_h, 2, 2, dht1HString);


  message += "# HELP temperature_dht0_inside Inside temperature.\n";
  message += "# TYPE temperature_dht0_inside gauge\n";
  message += "temperature_dht0_inside ";
  message += dht0TString;
  message += "\n";

  message += "# HELP temperature_dht1_outside Outside temperature.\n";
  message += "# TYPE temperature_dht1_outside gauge\n";
  message += "temperature_dht1_outside ";
  message += dht1TString;
  message += "\n";

  message += "# HELP humidity_dht0_inside Inside humidity.\n";
  message += "# TYPE humidity_dht0_inside gauge\n";
  message += "humidity_dht0_inside ";
  message += dht0HString;
  message += "\n";

  message += "# HELP humidity_dht1_outside  Outside humidity.\n";
  message += "# TYPE humidity_dht1_outside gauge\n";
  message += "humidity_dht1_outside ";
  message += dht1HString;
  message += "\n";

  return message;
}


void HandleRoot() {
  server.send(200, "text/plain", GenerateMetrics());
}

void HandleNotFound() {
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
  server.send(404, "text/html", message);
}

void MeasureTemperature(long now) {
  if ( now - lastTemp > measureTempFrequency ) { //Take a measurement at a fixed interval (durationTemp = 5000ms, 5s)
    

    // Save the measured value to device array
    dht0_t = dht0.readTemperature();
    dht0_h = dht0.readHumidity();

    dht1_t = dht1.readTemperature();
    dht1_h = dht1.readHumidity();

    lastTemp = millis();  //Remember the last time measurement
  }
}
 
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  //Setup WIFI
  WiFi.begin(ssid, password);
  Serial.println("");

  //Wait for WIFI connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  server.on("/", HandleRoot);
  server.on("/metrics", HandleRoot);
  server.onNotFound( HandleNotFound );

  server.begin();
  Serial.println("HTTP server started at ip " + WiFi.localIP().toString() );
  dht0.begin();
  dht1.begin();
}

void loop(){
  long t = millis();
  server.handleClient();
  MeasureTemperature(t);
}
