/* Plant Mesurement Node
*  Requires Barometer_Sensor Library by http://www.seeedstudio.com 
 */
#include <DHT22.h>
#include "Barometer.h"
#include <Wire.h>

// MOISTURE SENSOR
#define MOISTURE_PIN  A1

// LIGHTSENSOR
#define LIGHT_PIN A0

// BAROMETER
Barometer myBarometer;
boolean first;

// HUMIDITY
#define DHT22_PIN 2
DHT22 myDHT22(DHT22_PIN);

void setup(){
  Serial.begin(9600);
  myBarometer.init();

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

void printMoistureSensor(){
  int moistureValue = analogRead(MOISTURE_PIN);
  float moisturePercent = moistureValue / 950.0 *100.0;
  sendJSONValue("m-moisture", toString(moisturePercent), "%");
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

void loop()
{
  first=true;

  printBarometer();
  printLightSensor();
  printMoistureSensor();
  printHumidity();
  
  delay(2000);
}






