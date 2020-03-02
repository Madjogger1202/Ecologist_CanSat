#include "microDS18B20.h"
#include "Adafruit_BMP280.h"
#include "SparkFun_ADXL345.h"  
#include <nRF24L01.h>                                     // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>    
RF24           radio(40, 41);                              // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
int            data[5];    
Adafruit_BMP280 bmp(44);
MicroDS18B20 t_sensor(42);
/*********** COMMUNICATION SELECTION ***********/
/*    Comment Out The One You Are Not Using    */
ADXL345 adxl = ADXL345(43);           // USE FOR SPI COMMUNICATION, ADXL345(CS_PIN);
void setup(){
   radio.begin();                                        // Инициируем работу nRF24L01+
   radio.setChannel(126);                                  // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
   radio.setDataRate     (RF24_1MBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
   radio.setPALevel      (RF24_PA_HIGH);                 // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
   radio.openWritingPipe (0x1234567899LL);               // Открываем трубу с идентификатором 0x1234567890 для передачи данных (на одном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)

  Serial.begin(115200);                 // Start the serial terminal
  Serial.println("SparkFun ADXL345 Accelerometer Hook Up Guide Example");
  Serial.println();
 
  
   
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
 
}

void loop(){
   t_sensor.requestTemp();
 
  int x,y,z;   
  adxl.readAccel(&x, &y, &z);         // Read the accelerometer values and store them in variables declared above x,y,z
  int temp, prs;  
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.print(", ");
  Serial.println(z); 
  Serial.print("Pressure = ");
  prs=bmp.readPressure();
  Serial.print(prs);
  Serial.println(" Pa");
   Serial.print("t: ");
   temp=t_sensor.getTemp();  
  Serial.println(temp);
  data[0]=prs;
  data[1]=temp;
  data[2]=x;
  data[3]=y;
  data[4]=z;
  radio.write(&data, sizeof(data));
  delay(100);
}
