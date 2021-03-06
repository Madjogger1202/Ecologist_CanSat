#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(10, 9);

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
};
all_data ecologist_data;     //

int buf_size;

// всего тестов :  1)давление для bmp280 2) температура для ds18b20 3) ускорения для adxl
#define test_1 3
#define test_2 4
#define test_3 5

void setup()
{
  pinMode(test_1, INPUT_PULLUP);
  pinMode(test_2, INPUT_PULLUP);
  pinMode(test_3, INPUT_PULLUP);
  Serial.begin(115200);
  radio.begin();                                        // Инициируем работу nRF24L01+
//  radio.enableDynamicPayloads();
  radio.setChannel(20);                                // Указываем канал приёма данных (от 0 до 127), 5 - значит приём данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate     (RF24_2MBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
  radio.setPALevel      (RF24_PA_HIGH);                 // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openReadingPipe (1, 0xE7E7E2E7E7);            // Открываем 1 трубу с идентификатором 0x1234567890 для приема данных (на ожном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
  radio.startListening  ();                             // Включаем приемник, начинаем прослушивать открытую трубу
}
void loop()
{
  if (radio.available())
  {
    //Serial.print(222);
    buf_size = 18;
    boolean valid;
    int16_t sum;
    int mode = (!digitalRead(3))*4 + (!digitalRead(4))*2+!digitalRead(5);
    Serial.println(mode);
    int8_t data_recieve[buf_size];
    radio.read(&data_recieve, sizeof(data_recieve));    // читаем данные и указываем сколько байт читать
//    for(int i =0; i<17; i++)
//    sum+=data_recieve[i];
//    sum=~sum;
 //   if(!(sum == data_recieve[17])) return;
    Serial.println(data_recieve[0]);
    switch (data_recieve[0])
    {
      case 0: 
      memcpy(&ecologist_data.temp_str, &data_recieve[1], sizeof(&ecologist_data.temp_str));
      break;
      case 1: 
      memcpy(&ecologist_data.press_str, &data_recieve[1], sizeof(&ecologist_data.press_str));
      Serial.println(data_recieve[1]+data_recieve[2]+data_recieve[3]+data_recieve[4]);
      break;
      case 2: 
      memcpy(&ecologist_data.x_str, &data_recieve[1], sizeof(&ecologist_data.x_str));
      memcpy(&ecologist_data.y_str, &data_recieve[1+sizeof(ecologist_data.x_str)], sizeof(&ecologist_data.y_str));
      memcpy(&ecologist_data.z_str, &data_recieve[1+sizeof(ecologist_data.x_str)+sizeof(ecologist_data.y_str)], sizeof(&ecologist_data.z_str));
      break;
      case 3: 
      memcpy(&ecologist_data.gps_lon, &data_recieve[1], sizeof(&ecologist_data.gps_lon));
      memcpy(&ecologist_data.gps_lat, &data_recieve[1+sizeof(ecologist_data.gps_lon)], sizeof(&ecologist_data.gps_lat));
      break;
      case 4: 
      memcpy(&ecologist_data.CO2_ppm, &data_recieve[1], sizeof(&ecologist_data.CO2_ppm));      
      break;
      case 5: 
      memcpy(&ecologist_data.O2_percent, &data_recieve[1], sizeof(&ecologist_data.O2_percent));
      break;
      case 6: 
      memcpy(&ecologist_data.CO_ppm, &data_recieve[1], sizeof(&ecologist_data.CO_ppm));
      memcpy(&ecologist_data.NO2_ppm, &data_recieve[1+sizeof(ecologist_data.CO_ppm)], sizeof(&ecologist_data.NO2_ppm));
      memcpy(&ecologist_data.NH3_ppm, &data_recieve[1+sizeof(ecologist_data.CO_ppm)+sizeof(ecologist_data.y_str)], sizeof(&ecologist_data.NH3_ppm));
      break;
      case 7: 
      memcpy(&ecologist_data.radiation, &data_recieve[1], sizeof(&ecologist_data.radiation));
      break;
      
    }
    Serial.println(76);


    switch(mode)
    {
      case 0:
        Serial.println("///////////////////// ALL DATA //////////////////////");
        Serial.print(" Acsel (adxl345) x : ");
        Serial.print(ecologist_data.x_str);
        Serial.print("  y : ");
        Serial.print(ecologist_data.y_str);
        Serial.print("  z : ");
        Serial.println(ecologist_data.z_str);
        Serial.print(" Temperature (ds18b20): ");
        Serial.println(ecologist_data.temp_str);
        Serial.print(" Pressure (bmp280) : ");
        Serial.println(ecologist_data.press_str);
        Serial.print(" GPS data : ");
        Serial.print(ecologist_data.gps_lat);
        Serial.print("    ");
        Serial.println(ecologist_data.gps_lon);
        Serial.print(" 3 gas sensor (ppm) NH3 : ");
        Serial.print(ecologist_data.NH3_ppm);
        Serial.print("  NO2 : ");
        Serial.print(ecologist_data.NO2_ppm);
        Serial.print("  CO : ");
        Serial.println(ecologist_data.CO_ppm);
        Serial.print(" CO2 sensor (ppm) : ");
        Serial.println(ecologist_data.CO2_ppm);
        Serial.print(" O2 percentege : ");
        Serial.println(ecologist_data.O2_percent);
        Serial.println("");
        Serial.println("");
        Serial.println("");
        Serial.println("");
        Serial.println("");
        Serial.println("");
        break;
      case 1:
        Serial.print("$");
        Serial.print(ecologist_data.temp_str);
        Serial.print(";");
        break;
      case 2:
        Serial.print("$");
        Serial.print(ecologist_data.press_str);
        Serial.print(";");
        break;
      case 3:
        Serial.print("$");
        Serial.print(ecologist_data.x_str);
        Serial.print(" ");
        Serial.print(ecologist_data.x_str);
        Serial.print(" ");
        Serial.print(ecologist_data.x_str);
        Serial.print(";");
        break;
      case 4: 
        Serial.print("GPS lat , lon   ");
        Serial.print(ecologist_data.gps_lat);
        Serial.print("   ");
        Serial.print(ecologist_data.gps_lon);
        Serial.println("");
        Serial.println("");
        Serial.println("");
        Serial.println("");
        Serial.println("");
        Serial.println("");
        Serial.println("");
        Serial.println("");
        break;
      case 5:
        Serial.print("$");
        Serial.print(ecologist_data.CO2_ppm);
        Serial.print(";");
        break;
    }
   }
}
