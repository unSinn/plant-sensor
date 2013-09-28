/* Plant Mesurement Node
*  Requires Barometer_Sensor Library by http://www.seeedstudio.com 
 */
#include <DHT22.h>
#include "Barometer.h"
#include <Wire.h>

// MOISTURE SENSOR
#define MOISTURE_PIN1  A1
#define MOISTURE_PIN2  A2
#define MOISTURE_PIN3  A3

#define LED_RED 9
#define LED_GREEN 10
#define LED_BLUE 11
#define MOISTURE_VCC 12

// LIGHTSENSOR
#define LIGHT_PIN A0

// BAROMETER
Barometer myBarometer;

// HUMIDITY
#define DHT22_PIN 2
DHT22 myDHT22(DHT22_PIN);

void setup(){
  Serial.begin(57600);
  myBarometer.init();
  
  pinMode(MOISTURE_VCC, OUTPUT);
  
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
  float temperature = myBarometer.bmp085GetTemperature(myBarometer.bmp085ReadUT()); //Get the temperature, bmp085ReadUT MUST be called first
  float pressure = myBarometer.bmp085GetPressure(myBarometer.bmp085ReadUP());//Get the pressure
  float altitude = myBarometer.calcAltitude(pressure); //Uncompensated caculation - in Meters 
  float atm = pressure / 101325; 

  sendJSONValue("b-temp", toString(temperature), "deg C");
  sendJSONValue("b-pres", toString(pressure), "Pa");
  sendJSONValue("b-rala", toString(atm), "atm"); //ralated_atmosphere
  sendJSONValue("b-alti", toString(altitude), "m");
}

void printLightSensor(){
  int sensorValue = analogRead(LIGHT_PIN);
  float rlsens=(float)(1023-sensorValue)*10/sensorValue;
  sendJSONValue("l-light", toString(rlsens), "mylux");
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
  int moistureValue;
  String pinstr = String(pin-4, DEC);
  moistureValue = analogRead(pin);
  float moisturePercent = moistureValue / 950.0 *100.0;
  sendJSONValue(String("m-moisture"+pinstr), toString(moisturePercent), "%");
}

void printHumidity(){
  DHT22_ERROR_t errorCode;
  errorCode = myDHT22.readData();
  if(errorCode == DHT_ERROR_NONE){
    float temperature = myDHT22.getTemperatureC();
    float humidity = myDHT22.getHumidity();
    sendJSONValue("h-humidity", toString(humidity), "% RH");
    sendJSONValue("h-temperature", toString(temperature), "deg C");
  }
}

void setColor (unsigned char red, unsigned char green, unsigned char blue)  
{
  analogWrite(LED_RED, red);
  analogWrite(LED_GREEN, green);
  analogWrite(LED_BLUE, blue);
}

void loop()
{
  printBarometer();
  printLightSensor();
  readMoistureSensors();
  printHumidity();
  
  while(Serial.available() > 0){
    char charByte = Serial.read();
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
  
  delay(1000);
  
}

