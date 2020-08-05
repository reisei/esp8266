
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <DHT.h>


#define SENSOR_PIN 2


float soil_data;
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

  char soil_dataString[6];
  
  dtostrf(soil_data, 2, 2, soil_dataString);


  message += "# HELP soil_moisture Capacitive soil moisture sensor.\n";
  message += "# TYPE soil_moisture gauge\n";
  message += "soil_moisture ";
  message += soil_dataString;
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

void MeasureSoilMoisture(long now) {
  if ( now - lastTemp > measureTempFrequency ) { //Take a measurement at a fixed interval (durationTemp = 5000ms, 5s)
    
    soil_data = analogRead(SENSOR_PIN);
    
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
}

void loop(){
  long t = millis();
  server.handleClient();
  MeasureSoilMoisture(t);
}
