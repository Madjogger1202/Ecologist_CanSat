#define Radio_CE 12                     // пин chip enable для радио
#define Radio_CSN 13                    // пин chip select для радио
#define Acsel_pin 43                    // пин chip select для акселерометра
#define Temperature_pin 42              // пин chip select для термометра
#define Pressure_pin 44                 // пин chip select для барометра
#define Sd_pin 39                       // пин chip select для SD карты
#define MH_Z14A_PWM 18                  // пин для ШИМ датчика СО2
#define PIN_O2  A0                      // пин для датчика О2
#define PIN_CO  A1                      // пин для датчика CО
#define PIN_NO2 A2                      // пин для датчика NО2
#define PIN_NH3 A3                      // пин для датчика NH3

#define RADIO_BUF_SIZE 18               // макс размер - 3хfloat + 4 байт таймера + 2 байта контрольной суммы

#define OW_SKIP_ROM 0xCC                // Пропуск этапа адресации на шине 
#define OW_DS18B20_CONVERT_T 0x44       // Команда на начало замера
#define OW_DS18B20_READ_SCRATCHPAD 0xBE // Чтение скратчпада ds18b20
#define DS18B20_SCRATCHPAD_SIZE 9       // Размер скратчпада ds18b20


#include "MICS6814.h"
#include <iarduino_GPS_NMEA.h>          // Подключаем библиотеку для расшифровки строк протокола NMEA получаемых по UART.          
#include <OneWire.h>                    // библиотека для работы с барометром по одноименному протоколу
#include <string.h>                     // библиотека для memcpy
#include <stdint.h>                     // на всякий случай для int8_t, uint8_t и т.п.
#include <math.h>                       // библитотека для NAN
#include "Adafruit_BMP280.h"            // библиотека для работы с барометром
#include "SparkFun_ADXL345.h"           // библиотека для работы с акселерометром
#include <nRF24L01.h>                   // суб-библиотека для радиомодуля
#include <RF24.h>                       // основная библиотека для радиомодуля
iarduino_GPS_NMEA gps;                  // объект для работы с GPS модулем
RF24 radio(Radio_CE, Radio_CSN);        // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
Adafruit_BMP280 bmp(Pressure_pin);      // создаём объект bmp для работы с барометром
OneWire  ds(42);                        // создаём объект ds для работы с термометром
ADXL345 adxl = ADXL345(Acsel_pin);      // создаём объект ADXL345 для работы с акселерометром
MICS6814 gas(PIN_CO, PIN_NO2, PIN_NH3); // 

enum data_id          //  
{                     //
  ds18_id =   0,      //
  bmp_id =    1,      //
  adxl_id =   2,      //
  gps_id =    3,      //
  co2_id =    4,      //
  o2_id =     5,      //
  gaz_x3_id = 6,      //
  rad_id =    7       //
  
};

struct all_data       // Создаем структуру
{
  float temp_str;     // переменная для температуры
  float press_str;    // переменная для давления с барометра
  int x_str;          ////////////////////////////////////////////////
  int y_str;          //  переменные для ускорений с акселерометра  //
  int z_str;          ////////////////////////////////////////////////
  float gps_lat;      // 
  float gps_lon;      //
  float O2_percent;   //
  int16_t CO2_ppm;    // переменная для хранения значения CO2 в ppm
  float CO_ppm;       //
  float NO2_ppm;      //
  float NH3_ppm;      //
  float radiation;    //
  uint32_t timer;     // переменная для подсчета выполненных циклов программы
} ecologist_data;     //

////////////////////////////// функции для обновления данных (получения их)  
boolean ds18b20_convert_t();
boolean ds18b20_read_t(float & temperatur);
boolean get_MH_Z14A_data(int16_t &ppm);
boolean get_O2_percent(float &O2);
boolean get_acsel(int &x, int &y, int &z);
boolean get_pressure(float  &bmp_press);
boolean get_GPS_data(float &lon, float &lat);
boolean get_Radiation_value(float &doze);
boolean get_3_gas_value(float &CO_val, float &NO2_val, float &NH3_val);

////////////////////////////// функции для отправки пакетов ////////////
boolean send_O2_package(float O2);                                    //
boolean send_ds18_package(float temp);                                //
boolean send_bmp_package(float pressur);                              //
boolean send_adxl_package(int x_a, int y_a, int z_a);                 //
boolean send_CO2_package(int16_t Co2);                                //
boolean send_3_gas_package(float co, float no2, float nh3);           //
boolean send_GPS_package(float lon, float lat);                       //
boolean send_radiation_package(float rad_doze);                       //
////////////////////////////////////////////////////////////////////////

////////////////////////////// зануляем таймеры для отправки пакетов //
long int ds18b20_timer = 0 ;                                         //
long int bmp280_timer =  0 ;                                         //
long int adxl345_timer = 0 ;                                         //
long int gps_timer =     0 ;                                         //
long int co2_timer =     0 ;                                         //
long int o2_timer =      0 ;                                         //
long int gaz_x3_timer =  0 ;                                         //
long int rad_timer =     0 ;                                         //
///////////////////////////////////////////////////////////////////////

////////////////////////////// задержка между отправкой пакетов данных для каждого модуля ///
long int ds18b20_rate = 1000;                                                              //
long int bmp280_rate =  300 ;                                                              //
long int adxl345_rate = 200 ;                                                              //
long int gps_rate =     1000;                                                              //
long int co2_rate =     500 ;                                                              //
long int o2_rate =      1000;                                                              //
long int gaz_x3_rate =  1000;                                                              //
long int rad_rate =     60000;                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////

volatile int counter = 0;   //
void rad_tick();            //


void setup() {
  //////////////////////////////////////////////
  pinMode(7, INPUT);                          //
  attachInterrupt(7, rad_tick, FALLING);      //
  //////////////////////////////////////////////
  Serial.begin(9600);                         //  Инициируем работу с аппаратной шиной UART для получения данных от GPS модуля на скорости 9600 бит/сек.
  gps.begin(Serial);                          //  Инициируем расшифровку строк NMEA указав объект используемой шины UART.
  SPI.begin();                                // инициализируем работу с SPI
  SPI.setDataMode(SPI_MODE3);                 // насотройка SPI
  delay(100);  
  pinMode(18, INPUT);
  radio.begin();                              // Инициируем работу nRF24L01+
  radio.setChannel(120);                      // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate(RF24_2MBPS);              // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
  radio.setPALevel(RF24_PA_LOW);              // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openWritingPipe(0x1234567899LL);      // Открываем трубу с идентификатором 0x1234567899LL для передачи данных (на одном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
  gas.calibrate();
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
  if (millis >= ds18b20_timer)      // 
  {                                 //  
   if (ecologist_data.timer != 0)   //  при первой итерации - пропускается блок снятия показаний, после чего посылается запрос на температуру (датчик не может мгновенно дать показания)
    {                               //                                                                                    |
      float temp;                   //                                                                                    |                                                                                    |
      if(ds18b20_read_t(temp))      //                                                                                    |
        ecologist_data.temp_str = temp;       //                                                                                    |
      else                          //                                                                                    |
        ecologist_data.temp_str = NAN;        //   в случае исключения в переменную пишем, что она была посчитана неверно           |
    }                               //                                                                                    |
    ds18b20_convert_t();            // <<|--------------------------------------------------------------------------------/
    send_ds18_package(ecologist_data.temp_str);
    ds18b20_timer=millis+ds18b20_rate;//
  }                                 //
  ////////////////////////////////////
  if (millis >= bmp280_timer)
  {
    get_pressure(ecologist_data.press_str);     //
    send_bmp_package(ecologist_data.press_str); //
    bmp280_timer=millis+bmp280_rate;            //
  }

  if (millis >= adxl345_timer)
  {
    get_acsel(ecologist_data.x_str, ecologist_data.y_str, ecologist_data.z_str);          //
    send_adxl_package(ecologist_data.x_str, ecologist_data.y_str, ecologist_data.z_str);  //
    adxl345_timer=millis+adxl345_rate;                                                    //
  }

  if (millis >= gps_timer)
  {
    get_GPS_data(ecologist_data.gps_lon, ecologist_data.gps_lat);     //
    send_GPS_package(ecologist_data.gps_lon, ecologist_data.gps_lat); //
    gps_timer=millis+gps_rate;                                        //
  } 

  if (millis >= co2_timer)
  {
    get_MH_Z14A_data(ecologist_data.CO2_ppm);     //
    send_CO2_package(ecologist_data.CO2_ppm);     //
    co2_timer=millis+co2_rate;                    //
  }
  if (millis >= o2_timer)
  {
    get_O2_percent(ecologist_data.O2_percent);    //
    send_O2_package(ecologist_data.O2_percent);   //
    o2_timer=millis+o2_rate;                      //
  }
  

  if (millis >= gaz_x3_timer)
  {
    get_3_gas_value(ecologist_data.CO_ppm, ecologist_data.NO2_ppm, ecologist_data.NH3_ppm);     //
    send_3_gas_package(ecologist_data.CO_ppm, ecologist_data.NO2_ppm, ecologist_data.NH3_ppm);  //
    gaz_x3_timer=millis+gaz_x3_rate;                                                            //
  }
  if (millis >= rad_timer)
  {
    get_Radiation_value(ecologist_data.radiation);      //
    send_radiation_package(ecologist_data.radiation);   //
    rad_timer=millis+rad_rate;                          //
  }
  ecologist_data.timer++;                                       // + 1 выполненный цикл
  ////////////////////////////////////////////////////
}

void rad_tick()
{
  counter++;       //
}

boolean ds18b20_convert_t()
{

  if (!ds.reset()) // даем reset на шину
  {
    return false;
  }
  ds.write(OW_SKIP_ROM, 1);
  ds.write(OW_DS18B20_CONVERT_T, 1);
  return true;
}

boolean ds18b20_read_t(float & temperatur)
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

boolean get_MH_Z14A_data(int16_t &ppm)
{
  unsigned long th, tl, ppm2 = 0, ppm3 = 0;
  do {
    th = pulseIn(18, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm =  5000 * (th-2)/(th+tl-4); // расчёт для диапазона от 0 до 5000ppm 
  } while (th == 0);
}

boolean get_O2_percent(float &O2)
{
  unsigned int sum = 0;
  const float VRefer = 21.59;       // voltage of adc reference
  float Vout =0;
  for (unsigned char i = 64;i > 0;i--)
  {
    sum = sum + analogRead(PIN_O2);
    delay(1);
  }
  sum = sum >> 6;
  O2 = sum / 9.07;
  return 1;
}

boolean get_acsel(int &x, int &y, int &z)
{
  adxl.readAccel(&x, &y, &z);
  return 1;
}

boolean get_pressure(float  &bmp_press)
{
  bmp_press = bmp.readPressure();
  return 1;
}

boolean get_GPS_data(float &lon, float &lat)
{
  gps.read();
  if(gps.errPos)
  return 0;
  
  lat = gps.latitude;
  lon = gps.longitude;                          
  return 1;
}

boolean get_Radiation_value(float &doze)
{
  doze = counter/233,3;
  counter = 0;
  return 1;
}

boolean get_3_gas_value(float &CO_val, float &NO2_val, float &NH3_val)
{
  CO_val = gas.measure(CO);
  NO2_val = gas.measure(NO2);
  NH3_val = gas.measure(NH3);
  return 0; 
}

boolean send_O2_package(float O2)
{
  int16_t crc_pckg;
  int8_t radio_pack[RADIO_BUF_SIZE];
  memset(&radio_pack[0], 0x00, sizeof(radio_pack));
  radio_pack[0] = o2_id;
  memcpy(&radio_pack[1], &O2, sizeof(O2));
  for(int i =0; i<RADIO_BUF_SIZE; i++)
  {
    crc_pckg+=radio_pack[i];
  }
  crc_pckg = ~crc_pckg;
  memcpy(&radio_pack[16], &crc_pckg, sizeof(crc_pckg));
  
  radio.write(radio_pack, sizeof(radio_pack));
  return 1;
}

boolean send_ds18_package(float temp)
{
  int16_t crc_pckg;
  int8_t radio_pack[RADIO_BUF_SIZE];
  memset(&radio_pack[0], 0x00, sizeof(radio_pack));
  radio_pack[0] = ds18_id;
  memcpy(&radio_pack[1], &temp, sizeof(temp));
  for(int i =0; i<RADIO_BUF_SIZE; i++)
  {
    crc_pckg+=radio_pack[i];
  }
  crc_pckg = ~crc_pckg;
  memcpy(&radio_pack[16], &crc_pckg, sizeof(crc_pckg));
  
  radio.write(radio_pack, sizeof(radio_pack));
  return 1;
}

boolean send_bmp_package(float pressur)
{
  int16_t crc_pckg;
  int8_t radio_pack[RADIO_BUF_SIZE];
  memset(&radio_pack[0], 0x00, sizeof(radio_pack));
  radio_pack[0] = bmp_id;
  memcpy(&radio_pack[1], &pressur, sizeof(pressur));
  for(int i =0; i<RADIO_BUF_SIZE; i++)
  {
    crc_pckg+=radio_pack[i];
  }
  crc_pckg = ~crc_pckg;
  memcpy(&radio_pack[16], &crc_pckg, sizeof(crc_pckg));
  
  radio.write(radio_pack, sizeof(radio_pack));
  return 1;  
}

boolean send_adxl_package(int x_a, int y_a, int z_a)
{
  int16_t crc_pckg;
  int8_t radio_pack[RADIO_BUF_SIZE];
  memset(&radio_pack[0], 0x00, sizeof(radio_pack));
  radio_pack[0] = adxl_id;
  memcpy(&radio_pack[1], &x_a, sizeof(x_a));
  memcpy(&radio_pack[1+sizeof(x_a)], &y_a, sizeof(y_a));
  memcpy(&radio_pack[1+sizeof(x_a)+sizeof(y_a)], &z_a, sizeof(z_a));
  for(int i =0; i<RADIO_BUF_SIZE; i++)
  {
    crc_pckg+=radio_pack[i];
  }
  crc_pckg = ~crc_pckg;
  memcpy(&radio_pack[16], &crc_pckg, sizeof(crc_pckg));
  

  radio.write(radio_pack, sizeof(radio_pack));
  return 1;    
}

boolean send_CO2_package(int16_t Co2)
{
  int16_t crc_pckg;
  int8_t radio_pack[RADIO_BUF_SIZE];
  memset(&radio_pack[0], 0x00, sizeof(radio_pack));
  radio_pack[0] = co2_id;
  memcpy(&radio_pack[1], &Co2, sizeof(Co2));
  for(int i =0; i<RADIO_BUF_SIZE; i++)
  {
    crc_pckg+=radio_pack[i];
  }
  crc_pckg = ~crc_pckg;
  memcpy(&radio_pack[16], &crc_pckg, sizeof(crc_pckg));
  
  radio.write(radio_pack, sizeof(radio_pack));
  return 1;    
  
}

boolean send_3_gas_package(float co, float no2, float nh3)
{
  int16_t crc_pckg;
  int8_t radio_pack[RADIO_BUF_SIZE];
  memset(&radio_pack[0], 0x00, sizeof(radio_pack));
  radio_pack[0] = gaz_x3_id;
  memcpy(&radio_pack[1], &co, sizeof(co));
  memcpy(&radio_pack[1+sizeof(co)], &no2, sizeof(no2));
  memcpy(&radio_pack[1+sizeof(co)+sizeof(no2)], &nh3, sizeof(nh3));
  for(int i =0; i<RADIO_BUF_SIZE; i++)
  {
    crc_pckg+=radio_pack[i];
  }
  crc_pckg = ~crc_pckg;
  memcpy(&radio_pack[16], &crc_pckg, sizeof(crc_pckg));
  

  radio.write(radio_pack, sizeof(radio_pack));
  return 1;   
}

boolean send_GPS_package(float lon, float lat)
{
  int16_t crc_pckg;
  int8_t radio_pack[RADIO_BUF_SIZE];
  memset(&radio_pack[0], 0x00, sizeof(radio_pack));
  radio_pack[0] = gps_id;
  memcpy(&radio_pack[1], &lon, sizeof(lon));
  memcpy(&radio_pack[1+sizeof(lon)], &lat, sizeof(lat));
  for(int i =0; i<RADIO_BUF_SIZE; i++)
  {
    crc_pckg+=radio_pack[i];
  }
  crc_pckg = ~crc_pckg;
  memcpy(&radio_pack[16], &crc_pckg, sizeof(crc_pckg));
  

  radio.write(radio_pack, sizeof(radio_pack));
  return 1;
  
}

boolean send_radiation_package(float rad_doze)
{
  int16_t crc_pckg;
  int8_t radio_pack[RADIO_BUF_SIZE];
  memset(&radio_pack[0], 0x00, sizeof(radio_pack));
  radio_pack[0] = rad_id;
  memcpy(&radio_pack[1], &rad_doze, sizeof(rad_doze));
  for(int i =0; i<RADIO_BUF_SIZE; i++)
  {
    crc_pckg+=radio_pack[i];
  }
  crc_pckg = ~crc_pckg;
  memcpy(&radio_pack[16], &crc_pckg, sizeof(crc_pckg));
  

  radio.write(radio_pack, sizeof(radio_pack));
  return 1;
  
}
