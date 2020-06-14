#define Radio_CE 12  //40                 // пин chip enable для радио
#define Radio_CSN 13     //41             // пин chip select для радио
#define Acsel_pin 43                  // пин chip select для акселерометра
#define Temperature_pin 42            // пин chip select для термометра
#define Pressure_pin 44               // пин chip select для барометра
#define Sd_pin 39                     // пин chip select для SD карты

#define OW_SKIP_ROM 0xCC                // Пропуск этапа адресации на шине 
#define OW_DS18B20_CONVERT_T 0x44       // Команда на начало замера
#define OW_DS18B20_READ_SCRATCHPAD 0xBE // Чтение скратчпада ds18b20
#define DS18B20_SCRATCHPAD_SIZE 9       // Размер скратчпада ds18b20

#define telemetri_rate 250 // задержка между отправкой пакетов, задаётся в милисекундах 

#include <OneWire.h>
#include <string.h>  // для memcpy
#include <stdint.h>  // для int8_t, uint8_t и т.п.
#include <math.h> // для NAN
#include "Adafruit_BMP280.h"
#include "SparkFun_ADXL345.h"
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(Radio_CE, Radio_CSN);                              // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
Adafruit_BMP280 bmp(Pressure_pin);                            // создаём объект bmp для работы с барометром
OneWire  ds(42);                                              // создаём объект ds для работы с термометром
ADXL345 adxl = ADXL345(Acsel_pin);                            // создаём объект adxl для работы с акселерометром

int x, y, z;                                                  // переменные для ускорений по 3 осям

struct telemetry       //Создаем структуру
{
  float temp_str;      // переменная для температуры
  float bmp_temp_str;  // переменная для температуры с барометра
  float press_str;  // переменная для давления с барометра
  int x_str;           ////////////////////////////////////////////////
  int y_str;           //  переменные для ускорений с акселерометра  //
  int z_str;           ////////////////////////////////////////////////
  uint32_t timer;      // переменная для подсчета выполненных циклов программы
} data;


void setup() {
  Serial.begin(115200);
  SPI.begin();                                               // инициализируем работу с SPI
  SPI.setDataMode(SPI_MODE3);                                // насотройка SPI
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
  delay(1000);                     
}
void loop()
{
  if ((data.timer % 3 == 0))
  {
    if (data.timer != 0)
    {
      float temp;
      if(ds18b20_read_t(temp))
        data.temp_str = temp;
      else
        data.temp_str = NAN;
    }
    ds18b20_convert_t();
  }


  adxl.readAccel(&x, &y, &z);                  // запись значений для ускорений в указанные переменные
  data.bmp_temp_str = bmp.readTemperature();   // запись в структурную переменную телеметрии значений температуры с барометра
  data.press_str = bmp.readPressure();
  data.x_str = x;                              /////////////////////////////////////////////////////////////
  data.y_str = y;                              //   запись в структурную переменную телеметрии ускорений  //
  data.z_str = z;                              /////////////////////////////////////////////////////////////
  radio.write(&data, sizeof(data));            // отправка в эфир пакета данных
  delay(telemetri_rate);                       // задержка для отправки данных
  data.timer++;                               // + 1 выполненный цикл
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
