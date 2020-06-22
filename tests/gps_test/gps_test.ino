#include <SoftwareSerial.h>

#include <TinyGPS.h>

TinyGPS gps;
SoftwareSerial ss(9, 8);
float FLAT, FLON, ALT;
void setup()
{
  Serial.begin(9600);
  ss.begin(9600);
}

void loop()
{
  if(get_gps_data(FLAT, FLON, ALT))
 {   Serial.print("LAT=");
    Serial.print(FLAT, 6);
    Serial.print(" LON=");
    Serial.print(FLON, 6);
    Serial.print("  ");
    Serial.println(ALT);
  
 }Serial.println("...");
  delay(200);
}

boolean get_gps_data(float &flat,float &flon, float &alt)
{
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
    ss.flush();
    while (ss.available())
    {
      char c = ss.read();
      Serial.write(c); // Это можно откомментить для просмотра потока данных с модуля
      if (gps.encode(c)) 
        newData = true;
    }


  if (newData)
  {
    gps.f_get_position(&flat, &flon);

    alt=gps.f_altitude()-30;
    return 1;
  }
  
  gps.stats(&chars, &sentences, &failed);
  if (chars == 0)
    return 0;
}
