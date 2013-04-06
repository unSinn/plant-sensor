/* Plant Mesurement Node
*  Requires Barometer_Sensor Library by http://www.seeedstudio.com 
 */
#include "Barometer.h"
#include <Wire.h>

// MOISTURE SENSOR
int moisturePin = A1;

// LIGHTSENSOR
int lightPin = A0;

// BAROMETER
Barometer myBarometer;
float temperature;
float pressure;
float atm;
float altitude;
boolean first;

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
  temperature = myBarometer.bmp085GetTemperature(myBarometer.bmp085ReadUT()); //Get the temperature, bmp085ReadUT MUST be called first
  pressure = myBarometer.bmp085GetPressure(myBarometer.bmp085ReadUP());//Get the pressure
  altitude = myBarometer.calcAltitude(pressure); //Uncompensated caculation - in Meters 
  atm = pressure / 101325; 

  sendJSONValue("b-temp", toString(temperature), "deg C");
  sendJSONValue("b-pres", toString(pressure), "Pa");
  sendJSONValue("b-rala", toString(atm), "atm"); //ralated_atmosphere
  sendJSONValue("b-alti", toString(altitude), "m");
}

void printLightSensor(){
  int sensorValue = analogRead(lightPin);
  float rlsens=(float)(1023-sensorValue)*10/sensorValue;
  sendJSONValue("l-light", toString(rlsens), "mylux");
}

void printMoistureSensor(){
  int moistureValue = analogRead(moisturePin);
  float moisturePercent = moistureValue / 950.0 *100.0;
  sendJSONValue("m-moisture", toString(moisturePercent), "%");
}

void loop()
{
  first=true;

  printBarometer();
  printLightSensor();
  printMoistureSensor();
  
  delay(60000);
}






