
#include "SparkFun_ADXL345.h"         // SparkFun ADXL345 Library

#include "SparkFunBME280.h"

BME280 mySensorB;

ADXL345 adxl = ADXL345(43);         

void setup(){
  
  Serial.begin(115200);                 // Start the serial terminal
  Serial.println("SparkFun ADXL345 Accelerometer Hook Up Guide Example");
  Serial.println();
  
  adxl.powerOn();                     // Power on the ADXL345
  adxl.setRangeSetting(16);           // Give the range settings                                   // Accepted values are 2g, 4g, 8g or 16g
                                      // Higher Values = Wider Measurement Range
                                      // Lower Values = Greater Sensitivity

    adxl.setSpiBit(0);                  // Configure the device to be in 4 wire SPI mode when set to '0' or 3 wire SPI mode when set to 1
                                      // Default: Set to 1  
   adxl.setActivityXYZ(1, 0, 0);       // Set to activate movement detection in the axes "adxl.setActivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
   adxl.setActivityThreshold(75);      // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
 
   adxl.setInactivityXYZ(1, 0, 0);     // Set to detect inactivity in all the axes "adxl.setInactivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
   adxl.setInactivityThreshold(75);    // 62.5mg per increment   // Set inactivity // Inactivity thresholds (0-255)
   adxl.setTimeInactivity(10);         // How many seconds of no activity is inactive?

   adxl.setTapDetectionOnXYZ(0, 0, 1); // Detect taps in the directions turned ON "adxl.setTapDetectionOnX(X, Y, Z);" (1 == ON, 0 == OFF)
// Set values for what is considered FREE FALL (0-255)
   adxl.setFreeFallThreshold(7);       // (5 - 9) recommended - 62.5mg per increment
   adxl.setFreeFallDuration(30);       // (20 - 70) recommended - 5ms per increment
   mySensorB.settings.commInterface = SPI_MODE;
   mySensorB.settings.chipSelectPin = 44;
   mySensorB.settings.runMode = 3; //  3, Normal mode
   mySensorB.settings.tStandby = 0; //  0, 0.5ms
   mySensorB.settings.filter = 0; //  0, filter off
    mySensorB.settings.pressOverSample = 1;
    mySensorB.settings.humidOverSample = 1;
    mySensorB.settings.tempOverSample = 1;
    delay(10);
   mySensorB.begin();
   delay(10);
}
void loop(){
  int Pressure;
  Pressure = mySensorB.readFloatPressure();
  delay(10);
  int x,y,z;   
  adxl.readAccel(&x, &y, &z);         // Read the accelerometer values and store them in variables declared above x,y,z
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.print(", ");
  Serial.println(z); 
  Serial.print("Pressure: ");
  Serial.println(Pressure, 2);
  delay(500);
}
