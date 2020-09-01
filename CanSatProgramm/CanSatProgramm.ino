  
#define Radio_CE 12                     // пин chip enable для радио
#define Radio_CSN 13                    // пин chip select для радио
#define Acsel_pin 27                    // пин chip select для акселерометра
#define Temperature_pin 42              // пин chip select для термометра
#define Pressure_pin 26                 // пин chip select для барометра
#define Sd_pin 41                       // пин chip select для SD карты
#define MH_Z14A_PWM 18                  // пин для ШИМ датчика СО2
#define PIN_O2  A0                      // пин для датчика О2
#define PIN_CO  46                      // пин для датчика CО
#define PIN_NO2 47                      // пин для датчика NО2
#define PIN_NH3 48                      // пин для датчика NH3

#define OW_SKIP_ROM 0xCC                // Пропуск этапа адресации на шине 
#define OW_DS18B20_CONVERT_T 0x44       // Команда на начало замера
#define OW_DS18B20_READ_SCRATCHPAD 0xBE // Чтение скратчпада ds18b20
#define DS18B20_SCRATCHPAD_SIZE 9       // Размер скратчпада ds18b20

#define telemetri_rate 250              // задержка между отправкой пакетов, задаётся в милисекундах 

#include <SD.h>
#include "MICS6814.h"
#include <iarduino_GPS_NMEA.h>                    //  Подключаем библиотеку для расшифровки строк протокола NMEA получаемых по UART.
iarduino_GPS_NMEA gps; 
#include <OneWire.h>
#include <string.h>  // для memcpy
#include <stdint.h>  // для int8_t, uint8_t и т.п.
#include <math.h> // для NAN
#include "Adafruit_BMP280.h"
#include "SparkFun_ADXL345.h"
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(Radio_CE, Radio_CSN);        // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
Adafruit_BMP280 bmp(Pressure_pin);      // создаём объект bmp для работы с барометром
OneWire  ds(36);                        // создаём объект ds для работы с термометром
ADXL345 adxl = ADXL345(Acsel_pin);      // создаём объект ADXL345 для работы с акселерометром
MICS6814 gas(PIN_CO, PIN_NO2, PIN_NH3);
float FLAT, FLON, ALT;

int x, y, z;                            // переменные для ускорений по 3 осям

bool buzz;

struct telemetry_p1       //Создаем структуру
{
  bool id = 0;
  float temp_str;      // переменная для температуры
  float press_str;     // переменная для давления с барометра
  int x_str;           ////////////////////////////////////////////////
  int8_t y_str;           //  переменные для ускорений с акселерометра  //
  int16_t z_str;           ////////////////////////////////////////////////
  float gps_lat;
  float gps_lon;
  float GPStime;
  uint32_t timer;      // переменная для подсчета выполненных циклов программы
} data_1;

struct telemetry_p2    //Создаем структуру
{
  bool id = 1;
  float O2_percent;
  float CO_ppm;
  int GPSaltit;
  int8_t MH_Z14A_temp; // переменная для хранения температуры с датчика СО2
  int16_t CO2_ppm;     // переменная для хранения значения CO2 в ppm
  float NO2_ppm;
  float NH3_ppm;  
  float rad;
  uint32_t timer;      // переменная для подсчета выполненных циклов программы
} data_2;
  
volatile int counter = 0;   //
void rad_tick();            //

long int ds18b20_timer = 0 ;                                         //
long int bmp280_timer =  0 ;                                         //
long int adxl345_timer = 0 ;                                         //
long int gps_timer =     0 ;                                         //
long int co2_timer =     0 ;                                         //
long int o2_timer =      0 ;                                         //
long int gaz_x3_timer =  0 ;                                         //
long int rad_timer =     0 ;                                         //
long int radio_timer =     0 ;                                         //

int gas_turn;

void setup() {
  pinMode(46, INPUT_PULLUP);
  pinMode(47, INPUT_PULLUP);
  pinMode(48, INPUT_PULLUP);
  pinMode(16, 1);
  digitalWrite(16, 1);
  pinMode(Sd_pin, OUTPUT);
  SD.begin(Sd_pin);
  //////////////////////////////////////////////
  pinMode(7, INPUT);                          //
  attachInterrupt(7, rad_tick, FALLING);      //
  //////////////////////////////////////////////
  pinMode(18, INPUT);
  Serial.begin(9600);                         //  Инициируем работу с аппаратной шиной UART для получения данных от GPS модуля на скорости 9600 бит/сек.
  gps.begin(Serial);                          //  Инициируем расшифровку строк NMEA указав объект используемой шины UART.
  SPI.begin();                                               // инициализируем работу с SPI
  SPI.setDataMode(SPI_MODE3);                                // насотройка SPI
  delay(100);  
  radio.begin();                                             // Инициируем работу nRF24L01+
  radio.setChannel(120);                                     // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate(RF24_2MBPS);                           // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
  radio.setPALevel(RF24_PA_HIGH);                            // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openWritingPipe(0x1234567899LL);                     // Открываем трубу с идентификатором 0x1234567899LL для передачи данных (на одном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
  gas.loadCalibrationData(12,12,12);
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
  if (millis()/100>=ds18b20_timer)        //  раз в 3 итерации выполняется снятие данных с датчика
  {  
    if (data_1.timer != 0)            //  при первой итерации - пропускается блок снятия показаний, после чего посылается запрос на температуру (датчик не может мгновенно дать показания)
    {                               //                                                                                    |
      float temp;                   //                                                                                    |                                                                                    |
      if(ds18b20_read_t(temp))      //                                                                                    |
        data_1.temp_str = temp;       //                                                                                    |
      else                          //                                                                                    |
        data_1.temp_str = NAN;        //   в случае исключения в переменную пишем, что она была посчитана неверно           |
    }
    ds18b20_timer = millis()/100+10;  //                                                                                    |
    ds18b20_convert_t();            // <<|--------------------------------------------------------------------------------/
  }                                 //
  ////////////////////////////////////
//  if(millis()/100 >= gaz_x3_timer)
//  {
//    get_3_gas_value(data_2.CO_ppm, data_2.NO2_ppm, data_2.NH3_ppm);   
//    gaz_x3_timer = millis()/100 + 100;
//  }

  if((millis()/100 >= o2_timer)&&(!gas_turn))
  {
    get_O2_percent(data_2.O2_percent);
    o2_timer = millis()/100 + 10;
  }
  
  if((millis()/100 >= gaz_x3_timer)&&(gas_turn))
  {
    if(gas_turn == 1)
    {
      data_2.CO_ppm = gas.measure(CO);
    }
    else if(gas_turn == 2)
    {
      data_2.NO2_ppm = gas.measure(NO2);
    }
    else
    {
      data_2.NH3_ppm = gas.measure(NH3);
    }
    gaz_x3_timer = millis()/100 + 3;
    if(gas_turn != 3)gas_turn++;
    else gas_turn=0;
    
  }

  if(millis()/100>=adxl345_timer)
  {
    adxl.readAccel(&x, &y, &z);                         // запись значений для ускорений в указанные переменные
    data_1.x_str = x;                                   /////////////////////////////////////////////////////////////
    data_1.y_str = y;                                   //   запись в структурную переменную телеметрии ускорений  //
    data_1.z_str = z;                                   /////////////////////////////////////////////////////////////
    adxl345_timer = millis()/100 + 2;
  }

  if(millis()/100>=bmp280_timer)
  {
    data_1.press_str = bmp.readPressure();              //
    bmp280_timer = millis()/100 +10;
  }

  if(millis()/100 >= co2_timer)
  {
    get_MH_Z14A_data(data_2.CO2_ppm);// получение данных с датчика CO2
    co2_timer = millis()/100 +10;
  }

  if(millis()/100 >= gps_timer)
  {
    gps.read(); 
    if(gps.errPos){}
    else 
    {
      data_1.gps_lat = gps.latitude;
      data_1.gps_lon = gps.longitude;                          
      data_2.GPSaltit = gps.altitude;
      data_1.GPStime = gps.seconds + 100*gps.minutes + 10000*gps.Hours;
    }
    gps_timer = millis()/100 + 20;
  }

  if(millis()/100 >= rad_timer)
  {
    get_Radiation_value(data_2.rad);
    rad_timer = millis()/100 + 600;
  }
  
  if((millis()/100) >= radio_timer )
  {
  
    radio.write(&data_1, sizeof(data_1));                 // отправка в эфир пакета данных
    radio.write(&data_2, sizeof(data_2));                 // отправка в эфир пакета данных
    data_1.timer++;
    data_2.timer++;                                       // + 1 выполненный цикл
    radio_timer = millis()/100 + 2;
    if(buzz)
    {
      tone(5, 2000);
    }
    buzz = !buzz;
    File allData = SD.open("Eco.csv", FILE_WRITE);
    if (allData)
    {
    
      allData.print(data_1.GPStime);
      allData.print(";");
      allData.print(data_2.GPSaltit);
      allData.print(";");
      allData.print(data_1.gps_lat, 7);
      allData.print(";");
      allData.print(data_1.gps_lon, 7);
      allData.print(";");
      allData.print(data_1.press_str);
      allData.print(";");
      allData.print(data_1.temp_str);
      allData.print(";");
      allData.print(data_1.x_str);
      allData.print(";");
      allData.print(data_1.y_str);
      allData.print(";");
      allData.print(data_1.z_str);
      allData.print(";");
      allData.print(data_2.O2_percent);
      allData.print(";");
      allData.print(data_2.CO2_ppm);
      allData.print(";");
      allData.print(data_2.CO_ppm);
      allData.print(";");
      allData.print(data_2.NO2_ppm);
      allData.print(";");
      allData.print(data_2.NH3_ppm);
      allData.print(";");
      allData.print(data_2.rad);
      allData.print(";");
      allData.print(data_2.timer);
      allData.print(";");
      allData.println(millis()/1000);
      allData.close();
    }
    else
    {
      tone(5, 701);
      delay(10);
      tone(5, 500);
      delay(50);
      noTone(5);
      delay(100);
      tone(5, 701);
      delay(10);
      tone(5, 500);
      delay(50);
      noTone(5);
    }
  }
  ////////////////////////////////////////////////////
  
}

void rad_tick()
{
  counter++;       //
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

boolean get_MH_Z14A_data(int16_t &ppm)
{
  unsigned long th, tl, tmr;
  tmr = millis()/100;
  do {
    th = pulseIn(18, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm =  5000 * (th-2)/(th+tl-4); // расчёт для диапазона от 0 до 5000ppm
    if(millis()/100-tmr >= 3000)return 0; 
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
    delayMicroseconds(250);
  }
  sum = sum >> 6;
  O2 = sum / 9.27;
  return 1;
}

boolean get_Radiation_value(float &doze)
{
  doze = counter;
  counter = 0;
  return 1;
}

boolean get_3_gas_value(float &COv, float &NO2v, float &NH3v)
{
   unsigned long COrs;
   unsigned long NO2rs;
   unsigned long NH3rs;
   for(int i = 0; i< 100; i++)
   {
      COrs += analogRead(PIN_CO);
      NO2rs += analogRead(PIN_NO2);
      NH3rs += analogRead(PIN_NH3);
      delay(2);
   }
   float ratio;
   float cor;
   float no2r;
   float nh3r;
   cor = COrs/13*(1023-13)/(1024 - COrs);
   no2r = NO2rs/13*(1023-13)/(1024 - NO2rs);
   nh3r = NH3rs/13*(1023-13)/(1024 - NH3rs);      
   COv = pow(cor, -1.179)*4.385;
   NO2v = pow(no2r, 1.007)/6.855;
   NH3v = pow(nh3r, -1.67)/1.47;
   return 0;
}
