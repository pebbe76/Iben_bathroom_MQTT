#include <Wire.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <stdlib.h>
#include "arduino_secrets.h"
#include "PubSubClient.h"

#define CLIENT_ID "Bathroom"
#define PUBLISH_DELAY 3000 // that is 3 seconds interval

int radio_Status = WL_IDLE_STATUS;  // WiFi radio's status

int sensorBlackWater = 0;
int sensorShowerSump = 0;
int sensorTankLevel = 0;

//False: liquid detected. True: liquid not detected
bool sensorTank1 = true;
bool sensorTank2 = true;
bool sensorTank3 = true;
bool sensorTank4 = true;

//Pin assignement 
int tankPin1 = 4;
int tankPin2 = 5;
int tankPin3 = 8;
int tankPin4 = 9;
int anaWaterSensorBlackWater = A0;
int anaWaterSensorShowerSump = A2;

int teller = 0;

WiFiClient wifiClient;
PubSubClient mqttClient;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("start");

  pinMode(tankPin1, INPUT);
  pinMode(tankPin2, INPUT);
  pinMode(tankPin3, INPUT);
  pinMode(tankPin4, INPUT);

  // check for the WiFi module:
  Serial.println("Starting WiFi...");
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  WiFi.setHostname("Arduino_Bathroom");

  // attempt to connect to WiFi network:
  while (radio_Status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network:
    radio_Status = WiFi.begin(WIFI_SSID, WIFI_PWD);

    // wait 5 seconds for connection:
    delay(2000);
  }

  // setup mqtt client
  mqttClient.setClient(wifiClient);
  mqttClient.setServer( "192.168.1.2", 1883); // or local broker
}

void loop() {
  reset();
  delay(PUBLISH_DELAY);

  Serial.print("start reading: ");
  Serial.println(teller);
  readTankLevel();
  readLeakSensors();

  //transmitData_Serial();
  transmitData_MQTT();
  mqttClient.loop();
  teller = teller + 1;
}

void readLeakSensors(){
  int black = 0;
  int sump = 0;

  black = analogRead(anaWaterSensorBlackWater);
  sump = analogRead(anaWaterSensorShowerSump);

//Serial.println(black);
  if (black <=850) {
    sensorBlackWater = 0;
  }
  else if (black >850 && black <=990){
    sensorBlackWater = 25;
  }
  else if (black >990 && black <=1020){
    sensorBlackWater = 75;
  }
  else if (black >1020){
    sensorBlackWater = 100;
  }

  if (sump <=850) {
    sensorShowerSump = 0;
  }
  else if (sump >850 && sump <=900){
    sensorShowerSump = 25;
  }
  else if (sump >900 && sump <=1000){
    sensorShowerSump = 75;
  }
  else if (sump >1000){
    sensorShowerSump = 100;
  }
}

void transmitData_MQTT(){
  if (mqttClient.connect(CLIENT_ID, "preben", "passord")) {
    char a[3];
    char b[3];
    char c[3];

    //Serial.println(sensorTankLevel);

    itoa (sensorTankLevel, a, 10); 
    itoa (sensorBlackWater, b, 10); 
    itoa (sensorShowerSump, c, 10); 
    
    mqttClient.publish("bathroom/tanklevel", a);
    mqttClient.publish("bathroom/blackwaterleak", b);
    mqttClient.publish("bathroom/showersumpleak", c);
  }
}

void transmitData_Serial(){
  Serial.print(sensorTankLevel);
  Serial.print(";");
  Serial.print(sensorBlackWater);
  Serial.print(";");
  Serial.print(sensorShowerSump);
  Serial.println(";");
}

void readTankLevel(){
  //********************************** BLACK WATER TANK ****************************************
  sensorTank1 = digitalRead(tankPin1); //Digital input 2
  sensorTank2 = digitalRead(tankPin2); //Digital input 3
  sensorTank3 = digitalRead(tankPin3); //Digital input 4
  sensorTank4 = digitalRead(tankPin4); //Digital input 5

  Serial.println(sensorTank1);
  Serial.println(sensorTank2);
  Serial.println(sensorTank3);
  Serial.println(sensorTank4);
  Serial.println(" ");
  
  //need one reading for all four sensors, then if/else or switch/case on what sensor is active
  if (sensorTank4 == false) {
    sensorTankLevel = 100;
  }
  else if (sensorTank3 == false){
    sensorTankLevel = 75;
  }
  else if (sensorTank2 == false){
    sensorTankLevel = 35;
  }
  else if (sensorTank1 == false){
    sensorTankLevel = 10;
  }
}

void reset(){
  sensorBlackWater = 0;
  sensorShowerSump = 0;
  sensorTankLevel = 0;
  readTankLevel();
  sensorTank1 = true;
  sensorTank2 = true;
  sensorTank3 = true;
  sensorTank4 = true;
}
