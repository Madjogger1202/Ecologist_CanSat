#define Radio_CE 12                     // пин chip enable для радио
#define Radio_CSN 13                    // пин chip select для радио
#define Acsel_pin 43                    // пин chip select для акселерометра
#define Temperature_pin 42              // пин chip select для термометра
#define Pressure_pin 44                 // пин chip select для барометра
#define Sd_pin 39                       // пин chip select для SD карты
#define MH_Z14A_RXD D2  // по даташиту - TX датчика на 11 контакте
#define MH_Z14A_TXD D1 // по даташиту - RX датчика на 10 контакте

#define OW_SKIP_ROM 0xCC                // Пропуск этапа адресации на шине 
#define OW_DS18B20_CONVERT_T 0x44       // Команда на начало замера
#define OW_DS18B20_READ_SCRATCHPAD 0xBE // Чтение скратчпада ds18b20
#define DS18B20_SCRATCHPAD_SIZE 9       // Размер скратчпада ds18b20

#define telemetri_rate 250              // задержка между отправкой пакетов, задаётся в милисекундах 

#include <SoftwareSerial.h>       // для софтварного uart
#include <OneWire.h>
#include <TinyGPS.h>
#include <string.h>  // для memcpy
#include <stdint.h>  // для int8_t, uint8_t и т.п.
#include <math.h> // для NAN
#include "Adafruit_BMP280.h"
#include "SparkFun_ADXL345.h"
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(Radio_CE, Radio_CSN);        // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
Adafruit_BMP280 bmp(Pressure_pin);      // создаём объект bmp для работы с барометром
OneWire  ds(42);                        // создаём объект ds для работы с термометром
ADXL345 adxl = ADXL345(Acsel_pin);      // создаём объект ADXL345 для работы с акселерометром
SoftwareSerial MH_Z14A(MH_Z14A_RXD, MH_Z14A_TXD); // инициализация
SoftwareSerial GPS(9, 8);
float FLAT, FLON, ALT;

short int x, y, z;                            // переменные для ускорений по 3 осям

struct telemetry       //Создаем структуру
{
  float temp_str;      // переменная для температуры
  float bmp_temp_str;  // переменная для температуры с барометра
  float press_str;     // переменная для давления с барометра
  short int x_str;     ////////////////////////////////////////////////
  short int y_str;     //  переменные для ускорений с акселерометра  //
  short int z_str;     ////////////////////////////////////////////////
  int16_t CO2_ppm;     // переменная для хранения значения CO2 в ppm
  int8_t MH_Z14A_temp; // переменная для хранения температуры с датчика СО2
  uint32_t timer;      // переменная для подсчета выполненных циклов программы
} data;


void setup() {
  Serial.begin(115200);
  MH_Z14A.begin(9600);
  GPS.begin(9600); 
  MH_Z14A.setTimeout(80);
  GPS.setTimeout(100);
  SPI.begin();                                               // инициализируем работу с SPI
  SPI.setDataMode(SPI_MODE3);                                // насотройка SPI
  delay(100);  
  radio.begin();                                             // Инициируем работу nRF24L01+
  radio.setChannel(120);                                     // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate(RF24_250KBPS);                           // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
  radio.setPALevel(RF24_PA_HIGH);                            // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openWritingPipe(0x1234567899LL);                     // Открываем трубу с идентификатором 0x1234567899LL для передачи данных (на одном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)

  bmp.begin();
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* режим работы      */
                  Adafruit_BMP280::SAMPLING_X2,     /* коэф. температуры */
                  Adafruit_BMP280::SAMPLING_X16,    /* коэф. давления    */
                  Adafruit_BMP280::FILTER_X16,      /* фильтр            */
                  Adafruit_BMP280::STANDBY_MS_500); /* время ожидания    */

  adxl.powerOn();                     // вывод датчика из режима пониженного энергопотребления (на случай, если он был случайно включён)
  adxl.setRangeSetting(16);           // настройка чувствительности (макс - 16)
  delay(1000);                     
}
void loop()
{
  ////////////////////////////////////          ДЛЯ РАБОТЫ С ДАТЧИКОМ ТЕМПЕРАТУРЫ DS18B20
  if ((data.timer % 3 == 0))        //  раз в 3 итерации выполняется снятие данных с датчика
  {                                 //  
    if (data.timer != 0)            //  при первой итерации - пропускается блок снятия показаний, после чего посылается запрос на температуру (датчик не может мгновенно дать показания)
    {                               //                                                                                    |
      float temp;                   //                                                                                    |                                                                                    |
      if(ds18b20_read_t(temp))      //                                                                                    |
        data.temp_str = temp;       //                                                                                    |
      else                          //                                                                                    |
        data.temp_str = NAN;        //   в случае исключения в переменную пишем, что она была посчитана неверно           |
    }                               //                                                                                    |
    ds18b20_convert_t();            // <<|--------------------------------------------------------------------------------/
  }                                 //
  ////////////////////////////////////


  //////////////////////////////////////////////////// ЗАПИСЬ ОСТАВШИХСЯ ДАННЫХ В СТРУКТУРНЫЕ ПЕРЕМЕННЫЕ И СКИДЫВАЕМ СТРУКТУРУ ПО РАДИО
  adxl.readAccel(&x, &y, &z);                       // запись значений для ускорений в указанные переменные
  data.bmp_temp_str = bmp.readTemperature();        // запись в структурную переменную телеметрии значений температуры с барометра
  data.press_str = bmp.readPressure();              //
  data.x_str = x;                                   /////////////////////////////////////////////////////////////
  data.y_str = y;                                   //   запись в структурную переменную телеметрии ускорений  //
  data.z_str = z;                                   /////////////////////////////////////////////////////////////
  get_MH_Z14A_data(data.CO2_ppm, data.MH_Z14A_temp);// получение данных с датчика CO2                          
  radio.write(&data, sizeof(data));                 // отправка в эфир пакета данных
  delay(telemetri_rate);                            // задержка для отправки данных
  data.timer++;                                     // + 1 выполненный цикл
  ////////////////////////////////////////////////////
}
bool ds18b20_convert_t()
{

  if (!ds.reset()) // даем reset на шину
  {
    return false;
  }
  ds.write(OW_SKIP_ROM, 1);
  ds.write(OW_DS18B20_CONVERT_T, 1);
  return true;
}

bool ds18b20_read_t(float & temperatur)
{

  if (!ds.reset()) // даем резет на шину
    return false;

  ds.write(OW_SKIP_ROM, 1); // Пропускаем этап адресации
  uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE];
  ds.write(OW_DS18B20_READ_SCRATCHPAD, 1);
  ds.read_bytes(scratchpad, sizeof(scratchpad));
  uint8_t crc_actual = scratchpad[DS18B20_SCRATCHPAD_SIZE - 1]; // Берем контрольную сумму, которую насчитал у себя датчик и положил в последний байт скратчпада
  uint8_t crc_calculated = OneWire::crc8(scratchpad, DS18B20_SCRATCHPAD_SIZE - 1); // Считаем сами по всем байтам скратчпада кроме последнего
  float temp;
  if (crc_calculated != crc_actual)
  {
    return false;
  }
  uint16_t uraw_temp;
  uraw_temp = scratchpad[0] | (static_cast<uint16_t>(scratchpad[1]) << 8);
  int16_t raw_temp;
  memcpy(&raw_temp, &uraw_temp, sizeof(raw_temp));
  temp = raw_temp / 16.f;
  temperatur = temp;
  return true;
}

boolean get_MH_Z14A_data(int16_t &ppm, int8_t &temp)
{
  
 byte send_request[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79}; 
// массив для запроса данных. 1)команда для старта
//                            2)номер датчика, хз нужно ли, в даташите не увидел общей команды
//                            3)сама команда, из даташита: send concentration value of the sensor
//                            4-8)спам нулями
//                            9)запрос контрольной суммы, в даташите не увидел, но в интернете вроде так ставят
unsigned char get_value[9];
// массив для запроса данных. 1)команда для старта
//                            2)номер датчика, хз нужно ли, в даташите не увидел общей команды
//                            3)в даташите это называется high channel
//                            4)в даташите это называется low channel
//                            5)температура в градусах, но только с приплюсовыванием 40
//                            6-8)спам нулями
//                            9)crc
 
  uint8_t highCh;
  uint8_t lowCh; 
  MH_Z14A.flush(); 
  MH_Z14A.write(send_request, 9);
  delay(10);
  MH_Z14A.readBytes(get_value, 9);
  uint8_t crc = 0;
  for (int i = 1; i < 8; i++) crc+=get_value[i]; //считаем контрольную сумму по формуле из даташита
  crc = ~crc;
  crc++;
  if(!(get_value[0] == 0xFF && get_value[1] == 0x86 && get_value[8] == crc))
  return 0;
  highCh = uint8_t(get_value[2]);
  lowCh =  uint8_t(get_value[3]);
  ppm = highCh*256+lowCh;
  temp = get_value[4]-40;
  return 1;
}

boolean get_gps_data(float &flat,float &flon, float &alt)
{
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
    GPS.flush();
    while (GPS.available())
    {
      char c = GPS.read();
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
