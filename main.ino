#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>                               //import all the libraries required for sensors to interface
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "twilio.hpp"

MAX30105 particleSensor;
#define DS18B20 5                                         //define PIN No. at which the Temperature sensor is connected
#define MAX_BRIGHTNESS 255

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100];  //red LED sensor data
#else
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
#endif

int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid
float bodytemperature; // constant for calculating body temperature

const char* ssid = "1172 wifi sharks";  // Enter your Wi-Fi SSID here
const char* password = "Arni1309";  //Enter Wi-Fi Password here

String serverName = "https://api.thingspeak.com/update?api_key=OH3YZO59WS302XOL";

byte pulseLED = 33; //Must be on PWM pin
byte readLED = 32; //Blinks with each data read

OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);
static const char *account_sid = "AC387fea29dd03869acbbc7054eaed1c22";  // Twilio connection details                  
static const char *auth_token = "1cee5cad911733e601d8550396a5d686";
// Phone number should start with "+<countrycode>"
static const char *from_number = "+13466603745";

// You choose!
// Phone number should start with "+<countrycode>"
static const char *to_number = "+918484034691";  // Registered mobile no. to which the message is to be sent
static char *critical = "Alert! Health is critical. Please consult a doctor!\n - ";  // Message being sent that the health condition is critical to the registered no.
static char *greet = "Welcome on board! Your device UID is ";
static const char *uid = "l33tc0de";
Twilio *twilio;

void setup()
{
  pinMode(14,OUTPUT);                //Three indicator LED pinouts
  pinMode(27,OUTPUT);
  pinMode(26,OUTPUT);
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:

  pinMode(pulseLED, OUTPUT);
  pinMode(readLED, OUTPUT);

  Serial.println("Connecting to ");               // Attempt connecting to WiFi
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
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());   //Local IP Address is printed


  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));       //Attempt to initialize MAX30102
    while (1);
  }

  Serial.println(F("Attach sensor to finger with rubber band. Press any key to start conversion")); 
  while (Serial.available() == 0) ; //wait until user presses a key
  Serial.read();
  
  // setting the default parameters for the IR and read light
  byte ledBrightness = 30; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
  sensors.begin();
}
void BODY_TEMPERATURE();
void HEART_RATE();
void SpO2_LEVEL();
void init_message(); 
void alert_message(); 

void loop()
{
  HTTPClient http;
  bufferLength = 100; 
  sensors.requestTemperatures();                   // Get temperature readings from the DS18B20
  bodytemperature = sensors.getTempCByIndex(0);
  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false)   //do we have new data? 
      particleSensor.check();                     //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample

    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data

      digitalWrite(readLED, !digitalRead(readLED)); 

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample
 

      Serial.print(F("red="));               // print the red LED value
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));              //print the IR LED Value
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));              //print Heart Rate
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));        // 1 if the Heart rate is valid
      Serial.print(validHeartRate, DEC);     

      Serial.print(F(", SPO2="));          //print SP02
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));     //1 if the calculated SP02 is valid
      Serial.println(validSPO2, DEC);

    Serial.print("Body Temperature: ");    // prints the Body temperature
    Serial.print(bodytemperature);
    Serial.println("°C");
    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
String url = serverName + "&field1=" + heartRate + "&field2=" + spo2 + "&field3=" + bodytemperature ; // Define our entire url
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
  BODY_TEMPERATURE();
  HEART_RATE();
  SpO2_LEVEL();
 
      delay(15000);  

    }
  }
  void HEART_RATE()
{
if(heartRate>=60 && heartRate<=100)
{
  Serial.println("Normal Heart Rate");
}
else if (heartRate < 60)
  {
  digitalWrite(27,HIGH);
  delay(1000);

  init_message();
  alert_message();
  Serial.println("Slow Heart Rate");
  }
else if (heartRate > 130)
  {
  digitalWrite(27,HIGH);
  delay(1000);
  init_message();
  alert_message();
  Serial.println("High Heart Rate"); 
  }
}
void SpO2_LEVEL()
{
if(spo2 >90 && spo2<100)
{
  Serial.println("Normal Oxygen Level");
}
else if(spo2<=90)
  {
  digitalWrite(26,HIGH);
  delay(1000);
  init_message();
  alert_message();
  Serial.println("Low Oxygen Level");
  }
}
  void BODY_TEMPERATURE()
{
  if (bodytemperature<34.8)
  {
    digitalWrite(14,HIGH);
    delay(1000);
    init_message();
    alert_message();
    Serial.println("Low Temperature");
  }
else if(bodytemperature>=36.5 && bodytemperature<=37.2)
{
  digitalWrite(14,LOW);
  delay(1000);
  Serial.println("Normal Temperature");
}
else if(bodytemperature>=37.8)
  {
  digitalWrite(14,HIGH);
  delay(1000);
  init_message();
  alert_message();
  Serial.println("High Temperature");
  }
}

  void init_message() 
{
    twilio = new Twilio(account_sid, auth_token);

  delay(1000);
  String response;
  strcat(greet, uid);
  bool success = twilio->send_message(to_number, from_number, greet, response);
  if (success) {
    Serial.println("Sent message successfully!");
    free(greet);
  } else {
    Serial.println(response);
  }
}
void alert_message() 
{
  int alert = 1;
  strcat(critical, uid);
  if (alert) {
    String response;
    bool success = twilio->send_message(to_number, from_number, critical, response);
    if (success) {
      Serial.println("Sent message successfully!");
      // free(critical);
    } else {
      Serial.println(response);
    }
  } else {
    Serial.println("everything is normal!!!");
  }
}  

