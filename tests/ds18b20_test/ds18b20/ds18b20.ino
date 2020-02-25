#include "microDS18B20.h"

MicroDS18B20 t_sensor(2);

void setup() {
  Serial.begin(9600);  
}

void loop() {
  t_sensor.requestTemp();
  delay(1000);
  Serial.print("t: ");  
  Serial.println(t_sensor.getTemp());
}
