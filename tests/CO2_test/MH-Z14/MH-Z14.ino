
#include "MHZ14.h"

int MHZ14PIN = 7;// MHZ14 input pin
int MHZ14_CALIB_PIN = 6;// MHZ14 calibrate pin

MHZ14 CO2(MHZ14PIN);// initializer

void setup(){
  Serial.begin(9600);
  
  pinMode(MHZ14_CALIB_PIN, OUTPUT);
  digitalWrite(MHZ14_CALIB_PIN, HIGH);
}



void loop(){
  unsigned long PPM = CO2.read();
  Serial.println("CO2 PPM: "+String(PPM)+ " CO2 mg/M^3: "+ String(PPM * CO2.PPM_TO_MG_M3));
  delay(100);
}
