#include <SPI.h>
#include "Adafruit_BMP280.h"    
Adafruit_BMP280 bmp(44); // hardware SPI

void setup() {
  Serial.begin(115200);
  Serial.println("BMP280 test");
  

  bmp.begin();
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                Adafruit_BMP280::FILTER_X16,
                Adafruit_BMP280::STANDBY_MS_500);      /* Filtering. */

}

void loop() { 
 
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");

  delay(500);
 
}
