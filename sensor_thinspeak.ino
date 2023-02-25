#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h> 
 
#define DS18B20 5
#define REPORTING_PERIOD_MS     1000
 
float BPM, SpO2, bodytemperature;
 

const char* ssid = "1172 wifi sharks_5G";  // Enter SSID here
const char* password = "arni1309";  //Enter Password here
 
String serverName = "https://api.thingspeak.com/update?api_key=OH3YZO59WS302XOL";

PulseOximeter pox;
uint32_t tsLastReport = 0;
OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);
 
             
 
void onBeatDetected()
{
  Serial.println("Beat!");
}
 
void setup() {
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  Serial.begin(115200);
  pinMode(19, OUTPUT);
  delay(100);   
 
  Serial.println("Connecting to ");
  Serial.println(ssid);
 
  //connect to your local wi-fi network
  WiFi.begin(ssid, password);
 
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
 
 
  Serial.print("Initializing pulse oximeter..");
 
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
    pox.setOnBeatDetectedCallback(onBeatDetected);
  }
 
   pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
 
 
}

void loop() {


  HTTPClient http;
  pox.update();
  sensors.requestTemperatures();

  BPM = pox.getHeartRate();
  SpO2 = pox.getSpO2();
  bodytemperature = sensors.getTempCByIndex(0);
 
  
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) 
  {
    Serial.print("BPM: ");
    Serial.println(BPM);
    
    Serial.print("SpO2: ");
    Serial.print(SpO2);
    Serial.println("%");
 
    Serial.print("Body Temperature: ");
    Serial.print(bodytemperature);
    Serial.println("°C");
    
    Serial.println("*********************************");
    Serial.println();
 
    tsLastReport = millis();

    String url = serverName + "&field1=" + BPM + "&field2=" + SpO2 + "&field3=" + bodytemperature ; // Define our entire url
      http.begin(url.c_str()); 
      int httpResponseCode = http.GET(); 
      if (httpResponseCode > 0)
      { 
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      else
      {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
      delay(15000);  

    }
  }