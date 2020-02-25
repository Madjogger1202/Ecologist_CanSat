#include <Wire.h>

const int adxl345 = 0x53; // I2C адрес ADXL345

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // запишем адрес регистра DEVID
  Wire.beginTransmission(adxl345);
  Wire.write(byte(0x00));
  Wire.endTransmission();

  // прочитаем регистр DEVID:
  Wire.requestFrom(adxl345, 1);
  while (Wire.available()) {
    byte c = Wire.read();
    Serial.print("ID = ");
    Serial.println(c, HEX);
  }

  // переведём акселерометр в режим измерений
  Wire.beginTransmission(adxl345);
  Wire.write(byte(0x2D));
  Wire.write(byte(0x08));
  Wire.endTransmission();
}

void loop() {
  // запишем адрес начала данных по осям X, Y и Z:
  Wire.beginTransmission(adxl345);
  Wire.write(byte(0x32));
  Wire.endTransmission();

  // прочитаем 6 байтов значений XYZ:
  int i = 0;
  byte xyz[6];
  Wire.requestFrom(adxl345, 6);
  while (Wire.available()) {
    byte c = Wire.read();
    xyz[i] = c;
    i++;
  }

  // посчитаем и отобразим значения X, Y, Z:
  int x = word(xyz[1], xyz[0]);
  int y = word(xyz[3], xyz[2]);
  int z = word(xyz[5], xyz[4]);
  
  Serial.print(x);
  Serial.print("\t");
  Serial.print(y);
  Serial.print("\t");
  Serial.println(z);

  delay(100);
}
