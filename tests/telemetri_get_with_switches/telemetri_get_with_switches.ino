#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(10, 9);

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
} data_1, rec_data;

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


bool led;
void setup()
{

  Serial.begin(115200);
  radio.begin();                                        // Инициируем работу nRF24L01+
  radio.setChannel(100);                                // Указываем канал приёма данных (от 0 до 127), 5 - значит приём данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate     (RF24_1MBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
  radio.setPALevel      (RF24_PA_HIGH);                 // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openWritingPipe (0x1234567899LL);            // Открываем 1 трубу с идентификатором 0x1234567890 для приема данных (на ожном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
  radio.setAutoAck(false);
  //    radio.startListening  ();                             // Включаем приемник, начинаем прослушивать открытую трубу
}
void loop()
{
  // if(radio.available())
  // {
  delay(100);
  data_1.timer++;
  radio.write(&data_1, sizeof(data_1));                   // читаем данные и указываем сколько байт читать
       digitalWrite(A1, led);
       led=!led;  
  /*
     Serial.print("Temperature (ds18b20):");            //
     Serial.println(data_1.temp_str);                     // пишем в порт температуру
     Serial.print("Pressure (bmp280):");                //
     Serial.println(data_1.press_str);                  // пишем в порт давление с барометра
     Serial.print("X (adxl345):");                      //
     Serial.print(" ");                      //
     Serial.println(data_1.x_str);    // пишем в порт ускорение по оси X
     Serial.print("Y (adxl345):");                      //
     Serial.println(data_1.y_str);
     Serial.print("Z (adxl345):");                      //*/
  //      Serial.println(data_1.z_str);                        // пишем в порт ускорение по оси Z
  /*       Serial.print("CO2 ppm: ");                        // пишем в порт ускорение по оси Z
         Serial.println(data_2.CO2_ppm);
         Serial.print("GPS : ");
         Serial.print(data_1.gps_lat, 5);
         Serial.print("    ");
         Serial.println(data_1.gps_lon, 5);
         Serial.print("O2 : ");
         Serial.println(data_2.O2_percent);
         Serial.print("CO ppm :");
         Serial.println(data_2.CO_ppm);
         Serial.print("NO2 ppm :");
         Serial.println(data_2.NO2_ppm);
         Serial.print("NH3 ppm :");
         Serial.println(data_2.NH3_ppm);
         Serial.print("Radiation :");
         Serial.println(data_2.rad, 5);
         Serial.println("");
         Serial.println("");
         Serial.println("");
         Serial.println("");
         Serial.println("");
         Serial.println("");
  */
  //  }
}
