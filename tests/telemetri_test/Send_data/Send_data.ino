#define Radio_CE 40  //40                 // пин chip enable для радио
#define Radio_CSN 41     //41             // пин chip select для радио
#define Acsel_pin 43                  // пин chip select для акселерометра
#define Temperature_pin 42            // пин chip select для термометра
#define Pressure_pin 44               // пин chip select для барометра
#define Sd_pin 39                     // пин chip select для SD карты

#define telemetri_rate 300 // задержка между отправкой пакетов, задаётся в милисекундах 

#include <OneWire.h>
#include "Adafruit_BMP280.h"
#include "SparkFun_ADXL345.h"   
#include <nRF24L01.h>                                    
#include <RF24.h>  
#include <SD.h>
RF24 radio(Radio_CE, Radio_CSN);                              // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
Adafruit_BMP280 bmp(Pressure_pin);                            // создаём объект bmp для работы с барометром
OneWire  ds(Temperature_pin);                                 // создаём объект t_sensor для работы с термометром
ADXL345 adxl = ADXL345(Acsel_pin);                            // создаём объект adxl для работы с акселерометром

int last_temperature;                                         // переменная для сверки значений с термометра
bool lst;                                                     // флаг для обработки ошибок
int x,y,z;                                                    // переменные для ускорений по 3 осям
bool ready_t, ready_t_f;

struct telemetry        //Создаем структуру
{                  
   float temp_str;      // переменная для температуры
   float bmp_temp_str;  // переменная для температуры с барометра
   long int press_str;  // переменная для давления с барометра
   int x_str;           ////////////////////////////////////////////////
   int y_str;           //  переменные для ускорений с акселерометра  //
   int z_str;           ////////////////////////////////////////////////
   long int timer;      // переменная для подсчета выполненных циклов программы
}data; 
void setup(){
   pinMode(Sd_pin, OUTPUT);                                   // настройка chip select катрочки на отправку
   SPI.begin();                                               // инициализируем работу с SPI 
   SPI.setDataMode(SPI_MODE3);                                // насотройка SPI
   SD.begin(Sd_pin);                                          // инициализация sd карты 
   delay(100);                                                // задержка для уверенности в успешности инициализации 
   radio.begin();                                             // Инициируем работу nRF24L01+
   radio.setChannel(120);                                     // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
   radio.setDataRate(RF24_250KBPS);                             // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
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
   delay(1000);                        // задержка для адекватных значений температуры 
}
void loop()
{ 
  byte i;
   byte present = 0;
   byte type_s;
   byte data2[12];
   byte addr[8];
  if(ready_t)
  {
   if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
    if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);      
  }
  if(ready_t_f)
  {
    present = ds.reset();
    ds.select(addr);    
    ds.write(0xBE);  
    for ( i = 0; i < 9; i++) 
    { 
      data2[i] = ds.read(); 
    }
     int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  data.temp_str = (float)raw / 16.0;
    
  }

   adxl.readAccel(&x, &y, &z);                  // запись значений для ускорений в указанные переменные
//   data.temp_str = t_sensor.getTemp();          // запись в структурную переменную телеметрии значений температуры
   data.bmp_temp_str = bmp.readTemperature();   // запись в структурную переменную телеметрии значений температуры с барометра
   data.press_str = bmp.readPressure()/2;       // запись в структурную переменную телеметрии значений температуры с барометра
   data.x_str = x;                              /////////////////////////////////////////////////////////////
   data.y_str = y;                              //   запись в структурную переменную телеметрии ускорений  //
   data.z_str = z;                              ///////////////////////////////////////////////////////////// 
   radio.write(&data, sizeof(data));       //  отправка в эфир пакета данных
//   t_sensor.requestTemp();                 //  запрос температуры

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
    dataFile.close();                          // для подтверждения записи 
  }
   delay(telemetri_rate);                      // задержка для отправки данных 
   data.timer++;                                    // + 1 выполненный цикл
}
