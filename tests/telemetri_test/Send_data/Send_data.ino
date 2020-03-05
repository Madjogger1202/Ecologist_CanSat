#define Radio_CE 40
#define Radio_CSN 41
#define Acsel_pin 43
#define Temperature_pin 42
#define Pressure_pin 44
#define Sd_pin 

#include "microDS18B20.h"
#include "Adafruit_BMP280.h"
#include "SparkFun_ADXL345.h"   
#include <nRF24L01.h>                                    
#include <RF24.h>  
  
RF24 radio(Radio_CE, Radio_CSN);                              // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
Adafruit_BMP280 bmp(Pressure_pin);
MicroDS18B20 t_sensor(Temperature_pin); 
ADXL345 adxl = ADXL345(Acsel_pin);      

long int data[5]; 
int x,y,z;
  
void setup(){
   radio.begin();                                        // Инициируем работу nRF24L01+
   radio.setChannel(126);                                  // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
   radio.setDataRate(RF24_2MBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
   radio.setPALevel(RF24_PA_HIGH);                 // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
   radio.openWritingPipe(0x1234567899LL);               // Открываем трубу с идентификатором 0x1234567890 для передачи данных (на одном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)           
   
   SPI.begin();
   SPI.setDataMode(SPI_MODE3);
   bmp.begin();
   bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                   Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                   Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                   Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                   Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

   adxl.powerOn();                     // Power on the ADXL345
   adxl.setRangeSetting(16);           // Give the range settings
   t_sensor.requestTemp();
}

void loop()
{
   adxl.readAccel(&x, &y, &z);         // Read the accelerometer values and store them in variables declared above x,y,z
   data[2]=x;
   data[3]=y;
   data[4]=z; 
   data[0]=bmp.readPressure();
   data[1]=t_sensor.getTemp();
   radio.write(&data, sizeof(data));
   t_sensor.requestTemp();  
   delay(1000);
}
