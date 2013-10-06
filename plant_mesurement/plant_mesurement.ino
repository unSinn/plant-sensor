/* Plant Mesurement Node
*  Requires Barometer_Sensor Library by http://www.seeedstudio.com 
 */
#include <DHT22.h>
#include "Barometer.h"
#include <Wire.h>
#include "TM1637.h"

// Number Display
#define CLK 2 // Port D2 on Grove Shield
#define DIO 3
TM1637 tm1637(CLK,DIO);

// Displaying another value on the NumberDisplay every Second
#define DISPLAYINTERVAL 2000
#define NUMBEROFDISPLAYFUNCTIONS 6
unsigned long currentMillis;
unsigned long lastChangeOnDisplayMillis;
int currentDisplay = 1;

// Interval of SerialCommunication
#define SERIALINTERVAL 30000
unsigned long lastSerialSendMillis;

// MOISTURE SENSOR
int moistureValue;
#define MOISTURE_PIN1  A1
#define MOISTURE_PIN2  A2
#define MOISTURE_PIN3  A3
#define MOISTURE_VCC 12

// LEDs
#define LED_RED 9
#define LED_GREEN 10
#define LED_BLUE 11

// LIGHTSENSOR
#define LIGHT_PIN A0
float lightSensor;

// BAROMETER
Barometer myBarometer;
float b_temperature;
float b_pressure;
float b_altitude;
float b_atm;

// HUMIDITY
#define DHT22_PIN 3
DHT22 myDHT22(DHT22_PIN);
float dht_temperature;
float dht_humidity;

// Raspberry Heartbeat/Clock
#define RASPBERRYWATCHTIME 120000
int raspberry_hour;
int raspberry_minute;
unsigned long lastRaspberryTimeMillis = 0;

void setup(){
  Serial.begin(57600);
  myBarometer.init();
  
  pinMode(MOISTURE_VCC, OUTPUT);
  
  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
}

// {{"name":"b-temp"}, {"value":"13.12"}, {"Munit":"deg C"} }
void sendJSONValue(String name, String value, String unit){
  Serial.print("{");
  sendJSONAttribute("name", name);
  Serial.print(",");
  sendJSONAttribute("value", value);
  Serial.print(",");
  sendJSONAttribute("unit", unit);
  Serial.print("}\n");
  Serial.flush();
}

// {"name":"value"}
void sendJSONAttribute(String name, String value){ 
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\"");
  Serial.print(":");
  Serial.print("\"");
  Serial.print(value);
  Serial.print("\"");
}

String toString(float f){
  char buffer[12];
  dtostrf(f, 1, 3, buffer);
  return buffer;
}

void printBarometer(){
  b_temperature = myBarometer.bmp085GetTemperature(myBarometer.bmp085ReadUT()); //Get the temperature, bmp085ReadUT MUST be called first
  b_pressure = myBarometer.bmp085GetPressure(myBarometer.bmp085ReadUP());//Get the pressure
  b_altitude = myBarometer.calcAltitude(b_pressure); //Uncompensated caculation - in Meters 
  b_atm = b_pressure / 101325; 

  sendJSONValue("b-temp", toString(b_temperature), "deg C");
  sendJSONValue("b-pres", toString(b_pressure), "Pa");
  sendJSONValue("b-rala", toString(b_atm), "atm"); //ralated_atmosphere
  sendJSONValue("b-alti", toString(b_altitude), "m");
}

void printLightSensor(){
  int sensorValue = analogRead(LIGHT_PIN);
  float lightSensor=(float)(1023-sensorValue)*10/sensorValue;
  sendJSONValue("l-light", toString(lightSensor), "mylux");
}

void readMoistureSensors(){
  digitalWrite(MOISTURE_VCC, HIGH);
  delay(50);
  printMoisture(MOISTURE_PIN1);
  printMoisture(MOISTURE_PIN2);
  printMoisture(MOISTURE_PIN3);
  delay(50);
  digitalWrite(MOISTURE_VCC, LOW);
}

void printMoisture(int pin){
  String pinstr = String(pin-4, DEC);
  moistureValue = analogRead(pin);
  float moisturePercent = moistureValue / 950.0 *100.0;
  sendJSONValue(String("m-moisture"+pinstr), toString(moisturePercent), "%");
}

void printHumidity(){
  DHT22_ERROR_t errorCode;
  errorCode = myDHT22.readData();
  if(errorCode == DHT_ERROR_NONE){
    dht_temperature = myDHT22.getTemperatureC();
    dht_humidity = myDHT22.getHumidity();
    sendJSONValue("h-temperature", toString(dht_temperature), "deg C");
    sendJSONValue("h-humidity", toString(dht_humidity), "% RH");
  }
}

// LED Functions
void setColor (unsigned char red, unsigned char green, unsigned char blue)  
{
  analogWrite(LED_RED, red);
  analogWrite(LED_GREEN, green);
  analogWrite(LED_BLUE, blue);
}

// Display Functions
void displayMoisturePercent(){
  displayValue(moistureValue);
}
void displayLightValue(){
  displayValue((int)lightSensor);
}

void displayTemperature_Inside(){
  displayTemperature((int)dht_temperature);
}

void displayTemperature_Outside(){
  displayTemperature((int)b_temperature);
}

void displayRaspberryTime(){
  tm1637.point(POINT_ON);
  tm1637.display(0, raspberry_hour / 10);
  tm1637.display(1, raspberry_hour % 10); 
  tm1637.display(2, raspberry_minute / 10);
  tm1637.display(3, raspberry_minute % 10);
}

void displayTemperature(int temp){
  tm1637.point(POINT_OFF); 
  tm1637.display(0, temp / 10);
  tm1637.display(1, temp % 10); 
  tm1637.display(2, 0x7f);
  tm1637.display(3, 12);
}

void displayHumidity(){
  tm1637.point(POINT_OFF); 
  tm1637.display(0, (int)dht_humidity / 10);
  tm1637.display(1, (int)dht_humidity % 10); 
  tm1637.display(2, 0x7f);
  tm1637.display(3, 15);
}

void displayValue(int value){
  tm1637.point(POINT_OFF); 
  tm1637.display(0, (int)(value / 1000));
  tm1637.display(1, (int)(value / 100)%10); 
  tm1637.display(2, (int)(value / 10)%10);
  tm1637.display(3, value % 10);
}

void loop()
{
  currentMillis = millis();
  
  if(currentMillis - lastSerialSendMillis > SERIALINTERVAL){
    printBarometer();
    printLightSensor();
    readMoistureSensors();
    printHumidity();
    lastSerialSendMillis = currentMillis;
  }
  
  while(Serial.available()>2){
    char charByte = Serial.read();
    if (charByte == 'T'){
      while(Serial.available()<2){
        // wait for the other 2 bytes to come in
      }
      raspberry_hour = Serial.read();
      raspberry_minute = Serial.read();
      lastRaspberryTimeMillis = currentMillis;
    }
    if (charByte == 'R'){
      int value = Serial.read();
      setColor(value, 0, 0);
    }
    if (charByte == 'G'){
      int value = Serial.read();
      setColor(0, value, 0);
    }
    if (charByte == 'B'){
      int value = Serial.read();
      setColor(0, 0, value);
    }
  }

  if(currentMillis - lastRaspberryTimeMillis > RASPBERRYWATCHTIME){
   setColor(255, 0, 0);
  }
  
  if(currentMillis - lastChangeOnDisplayMillis > DISPLAYINTERVAL){
    switch(currentDisplay){
      case 1: 
        displayMoisturePercent();
        break;
      case 2: 
        displayLightValue();
        break;
      case 3: 
        displayHumidity();
        break;
      case 4: 
        displayTemperature_Inside();
        break;
      case 5: 
        displayTemperature_Outside();
        break;
      case 6: 
        displayRaspberryTime();
        break;
    }
    
    currentDisplay++; // switch to next display function
    if(currentDisplay > NUMBEROFDISPLAYFUNCTIONS){
       currentDisplay = 1;
    }
    lastChangeOnDisplayMillis = currentMillis;
  }
}


