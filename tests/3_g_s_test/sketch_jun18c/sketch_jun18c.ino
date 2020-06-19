void setup() {
  
Serial.begin(9600);
}

void loop() {
  long int NO2, CO, NH3;
//  for(int i=0; i<100; i++)
//  {
    NO2 =analogRead(0);
 //   delay(1);
//  }
//  for(int i=0; i<100; i++)
//   {
    CO=analogRead(1);
//    delay(1);
 //  }
//  for(int i=0; i<100; i++)
//   {
    NH3 =analogRead(2);
//    delay(1);
//   }
Serial.print(NO2);
Serial.print("   ");
Serial.print(CO);
Serial.print("    ");
Serial.println(NH3);
delay(100);
}
