#define Radio_CE 40                   // пин chip enable для радио
#define Radio_CSN 41                  // пин chip select для радио
#define Acsel_pin 43                  // пин chip select для акселерометра
#define Temperature_pin 42            // пин chip select для термометра
#define Pressure_pin 44               // пин chip select для баометра
#define Sd_pin 39                     // пин chip select для SD карты

#define telemetri_rate 300 // задержка между отправкой пакетов, задаётся в милисекундах 

#include "microDS18B20.h"
#include "Adafruit_BMP280.h"
#include "SparkFun_ADXL345.h"   
#include <nRF24L01.h>                                    
#include <RF24.h>  
#include <SD.h>
RF24 radio(Radio_CE, Radio_CSN);                              // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
Adafruit_BMP280 bmp(Pressure_pin);                            // создаём объект bmp для работы с барометром
MicroDS18B20 t_sensor(Temperature_pin);                       // создаём объект t_sensor для работы с термометром
ADXL345 adxl = ADXL345(Acsel_pin);                            // создаём объект adxl для работы с акселерометром

long int timer;                                               // переменная для подсчета выполненных циклов программы
int last_temperature;                                         // переменная для сверки значений с термометра
bool lst;
//long int data[6];                                             // массив для телеметрии (нужно будет переделать в структуру)
int x,y,z;                                                    // переменные для ускорений по 3 осям

struct telemetry    //Создаем структуру
{                  
   float temp_str;      //
   float bmp_temp_str;  //
   long int press_str;       //
   int x_str;           //
   int y_str;           //
   int z_str;           //
}data; 
void setup(){
   pinMode(Sd_pin, OUTPUT);                                   // настройка chip select катрочки на отправку
   SPI.begin();                                               // инициализируем работу с SPI 
   SPI.setDataMode(SPI_MODE3);                                // насотройка SPI
   SD.begin(Sd_pin);                                          // инициализация sd карты 
   delay(100);                                                // задержка для уверенности в успешности инициализации 
   radio.begin();                                             // Инициируем работу nRF24L01+
   radio.setChannel(126);                                     // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
   radio.setDataRate(RF24_2MBPS);                             // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
   radio.setPALevel(RF24_PA_HIGH);                            // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
   radio.openWritingPipe(0x1234567899LL);                     // Открываем трубу с идентификатором 0x1234567890 для передачи данных (на одном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)           
   
   
   bmp.begin();
   bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* режим работы */
                   Adafruit_BMP280::SAMPLING_X2,     /* коэф. температуры */
                   Adafruit_BMP280::SAMPLING_X16,    /* коэф. давления */
                   Adafruit_BMP280::FILTER_X16,      /* фильтр */
                   Adafruit_BMP280::STANDBY_MS_500); /* время ожидания */

   adxl.powerOn();                     // вывод датчика из режима пониженного энергопотребления
   adxl.setRangeSetting(16);           // настройка чувствительности (макс - 16)
   t_sensor.requestTemp();             // отправляем запрос на температуру
//   telemetry data;                     // создаём объект структуры
   delay(1000);                        // задержка для адекватных значений температуры 
}
void loop()
{

   adxl.readAccel(&x, &y, &z);             // запись значений для ускорений в указанные переменные
//   data[2]=x;                              /////////////////////////////////////////////
//   data[3]=y;                              //   запись в массив телеметрии ускорений  //
//   data[4]=z;                              /////////////////////////////////////////////
//   data[0]=bmp.readPressure();             // запись в массив телеметрии значений давления 
//   data[5]=bmp.readTemperature()*10;       // запись в массив телеметрии значений температуры с барометра
//   data[1]=t_sensor.getTemp()*10;          // запись в массив телеметрии значений температуры 
   data.temp_str = t_sensor.getTemp();
   data.bmp_temp_str = bmp.readTemperature();
   data.press_str = bmp.readPressure()/4; 
   data.x_str = x;
   data.y_str = y;
   data.z_str = z;
 
  if(timer == 4) last_temperature =  data.temp_str;
  if (timer > 4) {
    if  ( (( ( data.temp_str>400) || ( data.temp_str< 0) )||(abs( data.temp_str-last_temperature)>=19))&&(!lst)) {
      data.temp_str=last_temperature;
      lst=1;
    }
    else {
    lst=0;
    last_temperature= data.temp_str;
   }
   }
   radio.write(&data, sizeof(data));       //  отправка в эфир пакета данных
   t_sensor.requestTemp();                 //  запрос температуры

  File dataFile = SD.open("datalog.csv", FILE_WRITE);  // открываем для записи файл, если его нет - создаём
  if (dataFile)                                        // проверка, что пишем не в воздух
  {
    dataFile.print(data.press_str);            ///////////////////////////  
    dataFile.print(", ");                      //
    dataFile.print(data.temp_str);             //
    dataFile.print(", ");                      //   
    dataFile.print(data.x_str);                //
    dataFile.print(", ");                      //   запись  телеметрии на сд карту 
    dataFile.print(data.y_str);                //
    dataFile.print(", ");                      //
    dataFile.print(data.z_str);                //
    dataFile.print(", ");                      //
    dataFile.println(data.bmp_temp_str);       ///////////////////////////    
    dataFile.close();                          //   для подтверждения записи 
  }
   delay(telemetri_rate);                      // задержка для отправки данных 
   timer++;                                    // + 1 выполненный цикл
}
