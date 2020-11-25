
               // пин для ШИМ датчика СО2 ()

#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 8);        // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)

int x, y, z;                            // переменные для ускорений по 3 осям

bool buzz;

volatile unsigned long long co2_th;
volatile unsigned long long co2_tl;


// отладочные данные, удалить по окончанию сбора данных
int16_t CO2_time_inneed; // wery low bt btw
int16_t tVOC_time_inneed; // wery low too 
int16_t bmp280_time_inneed; //
int16_t ds18b20_time_inneed;
int16_t radio_nrf_time_inneed;
int16_t radio_lora_time_inneed;
int16_t acsel_time_inneed;
int16_t humidity_time_inneed;
int16_t GPS_tme_inneed;    // idk bt i hope not so high
int16_t SD_writing_time;   // i hope that it is not so huge)



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
  int16_t CO2_ppm;     // переменная для хранения значения CO2 в ppm 
  float rad;
  uint32_t timer;      // переменная для подсчета выполненных циклов программы
} data_2;


struct LoRa_telemerty
{
  int CO2_ppm;
  float tVOC;
  float pm2_5;
  float humid;
  float press;
  float temperature;
  float radiation;
  int8_t x_acs;
  int8_t y_acs; 
  int8_t z_acs;
  float gps_lon;
  float gps_lat;
  uint8_t timer;
  uint16_t time;
  int8_t error_code;

}LoRa_data;

volatile int counter = 0;   //
void rad_tick();            //

long int ds18b20_timer = 0 ;                                         //
long int gps_timer =     0 ;                                         //
long int rad_timer =     0 ;                                         //


void rad_tick();
bool ds18b20_convert_t();
bool ds18b20_read_t(float & temperatur);
boolean get_CO2_data(int16_t &ppm);
boolean get_Radiation_value(float &doze);
bool readgps();
void Co2_tl();
void Co2_th();
void getTVOC_val();
void getPM25_val();
void getHumidity_val();
void make_noise();
void stop_noise();
void write_to_SD();
void send_by_NRF();
void send_by_LoRa();

void setup() { 
 


  Serial.begin(9600);                         //  Инициируем работу с аппаратной шиной UART для получения данных от GPS модуля на скорости 9600 бит/сек.
  SPI.begin();                                               // инициализируем работу с SPI
  SPI.setDataMode(SPI_MODE3);                                // настройка SPI
  delay(100);  
  radio.begin();                                             // Инициируем работу nRF24L01+
  radio.setChannel(120);                                     // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate(RF24_2MBPS);                           // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
  radio.setPALevel(RF24_PA_HIGH);                            // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openWritingPipe(0x1234567899LL);                     // Открываем трубу с идентификатором 0x1234567899LL для передачи данных (на одном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
//  bmp.begin();
//  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* режим работы      */
//                  Adafruit_BMP280::SAMPLING_X2,     /* коэф. температуры */
//                  Adafruit_BMP280::SAMPLING_X16,    /* коэф. давления    */
//                  Adafruit_BMP280::FILTER_X16,      /* фильтр            */
//                  Adafruit_BMP280::STANDBY_MS_500); /* время ожидания    */
//
 // adxl.powerOn();                                   // вывод датчика из режима пониженного энергопотребления (на случай, если он был случайно включён)
//  adxl.setRangeSetting(16);                         // настройка чувствительности (макс - 16)
//  delay(1000);   
//  get_CO2_data(data_2.CO2_ppm);  
               
}
void loop()
{

/*
  ////////////////////////////////////          ДЛЯ РАБОТЫ С ДАТЧИКОМ ТЕМПЕРАТУРЫ DS18B20
  if (millis()/100>=ds18b20_timer)        //  раз в 800 мс выполняется снятие данных с датчика
  {  
    if (data_1.timer != 0)            //  при первой итерации - пропускается блок снятия показаний, после чего посылается запрос на температуру (датчик не может мгновенно дать показания)
    {                               //                                                                                    |
      float temp;                   //                                                                                    |                                                                                    |
      if(ds18b20_read_t(temp))      //                                                                                    |
        data_1.temp_str = temp;       //                                                                                    |
      else                          //                                                                                    |
        data_1.temp_str = NAN;        //   в случае исключения в переменную пишем, что она была посчитана неверно           |
    }
    ds18b20_timer = millis()/100+8;  //                                                                                    |
    ds18b20_convert_t();            // <<|--------------------------------------------------------------------------------/
  }                                 //



  adxl.readAccel(&x, &y, &z);                         // запись значений для ускорений в указанные переменные
  data_1.x_str = x;                                   /////////////////////////////////////////////////////////////
  data_1.y_str = y;                                   //   запись в структурную переменную телеметрии ускорений  //
  data_1.z_str = z;                                   /////////////////////////////////////////////////////////////

  data_1.press_str = bmp.readPressure();              //

  get_CO2_data(data_2.CO2_ppm);                   // получение данных с датчика CO2

  
  readgps(); //  ToDo -  нужно переделать c минимальным возможным временем парсинга данных (пока что алгоритм нерабочий)
  

  if(millis()/100 >= rad_timer)
  {
    get_Radiation_value(data_2.rad);
    rad_timer = millis()/100 + 600;
  }

  send_by_NRF();
  write_to_SD();
*/
  radio.write(&data_1, sizeof(data_1));
  data_1.z_str++;
  
  data_2.timer++; 
  
  delay(1000);
  }
  ////////////////////////////////////////////////////
  
