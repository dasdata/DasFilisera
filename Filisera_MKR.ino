/*
  This a simple example of the aREST Library for the Arduino/Genuino MKR1000 board.
  See the README file for more details.

  Written in 2016 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <SPI.h>
#include <WiFi101.h>
#include <aREST.h>
#include "DHT.h"
#include <OneWire.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <SHT2x.h>

// *************************
// ** Senzori Temperatura **
// *************************
#define DHTPIN 0     //  Inside humidity/temp/heat index
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define ONE_WIRE_BUS 1

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);



float umidSol = 0; // Soil humidity
float umidAer = 0; // Air humidity
float tempIn = 0; // Temperature in
float tempOut = 0; // Temperature out
float tempSol = 0; // Container volume
float umidexIn = 0; // Feels like temp

//VOLUM 
#define echoPin A0 // Echo pin A0
#define trigPin A1 // Trigger pin A1

long duration ; // duration to calculate distance
int HR_dist = 0; // Calculated distance

int minimumRange=5; //Min Sonar range
int maximumRange=200; //Max Sonar range
unsigned long pulseduration=0;

int nivelCm, nivelRamasCm;
int nivelProc, nivelVol;
int empty = 100;
int full = 100;
int sqrLevel, sqrInvertLevel ;
 

// Status
int status = WL_IDLE_STATUS;

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
char ssid[] = "WIFI_AP";
char password[] = "WIFI_PASSW";

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

//INITIALIZARE SENZORI 
DHT dht(DHTPIN, DHTTYPE);

// RELEE RELEEE
int aVentil, aLateral;
int inReleu1Ventil = 6;
int inReleu2Ventil = 7;
int inReleu3Ventil = 8;
int inReleu4Ventil = 9;
int inReleu1Lateral = 10;
int inReleu2Lateral = 11;

// Declare functions to be exposed to the API
int ledControl(String command);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Init variables and expose them to REST API
 // rest.variable("temperature",&temperature);
//  rest.variable("humidity",&humidity);

  // Function to be exposed
  rest.function("led",ledControl);

  // Give name and ID to device
  rest.set_id("1");
  rest.set_name("mkr1000");

  // Connect to WiFi
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);

    // Wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

   // SETUP DHT - INSIDE  
 DHT dht(DHTPIN, DHTTYPE);
// sensors.begin(); // Pornire senzor temperatura exterioara DS-16B20

 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 
  // Init variables and expose them to REST API

  rest.variable("Vol_container",&sqrLevel );
  rest.variable("Umiditate_sol",&umidSol );
  rest.variable("Umiditate_aer",&umidAer );
  rest.variable("Temp_in",&tempIn );
  rest.variable("Temp_out",&tempOut );
  rest.variable("Temp_sol",&tempSol ); 
 // rest.variable("Umidex_in",&umidexIn );
  
//  sensors.requestTemperatures(); // Temperatura Out
  
}

void loop() {
 getmyDistance();
 // getSoilVals();
  getInsideVals();
//  getOutSideVals();


  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);

}

void getmyDistance() {
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);

  duration = pulseIn(echoPin,HIGH);
  //calculate distance
  HR_dist = duration/58.2;
//  sqrLevel = empty - HR_dist;
  
  if (HR_dist >= maximumRange || HR_dist <= minimumRange) {
     sqrLevel = 0;
    }
    else {
       sqrLevel = empty - HR_dist;
  Serial.println(HR_dist); 
      }

      delay(100);
  }


void getDistance() {
digitalWrite(trigPin, LOW);
delayMicroseconds(10);
// now send the 10uS pulse out to activate Ping 
digitalWrite(trigPin, HIGH); delayMicroseconds(10); digitalWrite(trigPin, LOW);
// finally, measure the length of the incoming pulse 
pulseduration = pulseIn(echoPin, HIGH);
// divide the pulse length by half
pulseduration = pulseduration/2;
// convert to centimetres.
nivelRamasCm = int(pulseduration/29);
nivelCm = empty - nivelRamasCm;
nivelVol = map(nivelRamasCm, full, empty, 100, 0);

// Tipareste nivelul in centimetri
   if (nivelCm >= -5 && nivelCm <=135) {
     double a = nivelCm / 100 * nivelVol;
  int sqrLevel = 240 - nivelCm;
  int sqrInvertLevel = 120 + nivelRamasCm;

      if (nivelCm >65 && nivelCm <=85){
         }
      if (nivelCm >85 && nivelCm <=100){
         }
   }
  }

long microsecondsToCentimeters (long microseconds) {
 // The speed of sound is 340 m/s or 29 microseconds per centimeter
 // The ping travels forth and back, so, the distance is half the distance traveled
 return microseconds / 29 / 2;
}

void getInsideVals() {
    
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
//  float f = dht.readTemperature(true);

  umidAer = h;
  tempIn = t;

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  //float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
 // float hic = dht.computeHeatIndex(t, h, false); 
//  umidexIn = hic; 

    }

void getOutSideVals() {
       tempOut = sensors.getTempCByIndex(0) ; 
         Serial.println("Temperature Out: ");
         Serial.print(tempOut);
       }    


void getSoilVals() {
      umidSol = SHT2x.GetHumidity();
      tempSol = SHT2x.GetTemperature();
      Serial.println("Humidity Soil(%RH): ");
      Serial.print(umidSol);
      Serial.print("     Temperature Soil (C): ");
      Serial.print(tempSol);
       }    
       

  //  sensors.requestTemperatures(); // Temperatura Out

 
// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(6,state);
  return 1;
}
