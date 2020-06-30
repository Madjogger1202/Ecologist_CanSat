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
  uint32_t timer;      // переменная для подсчета выполненных циклов программы
} data_1, rec_data;

struct telemetry_p2    //Создаем структуру
{
  bool id = 1;
  float O2_percent;
  float CO_ppm;
  int trash1;
  int8_t MH_Z14A_temp; // переменная для хранения температуры с датчика СО2
  int16_t CO2_ppm;     // переменная для хранения значения CO2 в ppm
  float NO2_ppm;
  float NH3_ppm;  
  uint32_t timer;      // переменная для подсчета выполненных циклов программы
} data_2;
  
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
    radio.setChannel(120);                                // Указываем канал приёма данных (от 0 до 127), 5 - значит приём данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
    radio.setDataRate     (RF24_250KBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
    radio.setPALevel      (RF24_PA_HIGH);                 // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
    radio.openReadingPipe (1, 0x1234567899LL);            // Открываем 1 трубу с идентификатором 0x1234567890 для приема данных (на ожном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
    radio.startListening  ();                             // Включаем приемник, начинаем прослушивать открытую трубу
}
void loop()
{
    if(radio.available())
    {                                
      radio.read(&rec_data, sizeof(rec_data));                   // читаем данные и указываем сколько байт читать
      if(rec_data.id)
      {
        data_2.O2_percent = rec_data.temp_str;
        data_2.CO_ppm = rec_data.press_str;
        data_2.MH_Z14A_temp = rec_data.y_str;
        data_2.CO2_ppm = rec_data.z_str;
        data_2.NO2_ppm = rec_data.gps_lat;
        data_2.NH3_ppm = rec_data.gps_lon;
        data_2.timer = rec_data.timer;
          
      }
      else
      {
          data_1 = rec_data;        
      }
      
      
      
      if(!digitalRead(test_1))
      {
      Serial.print("$");
      Serial.println(data_1.press_str); // в паскалях
      Serial.print(";");
      }
      else if(!digitalRead(test_2))
      {
      Serial.print("$");
      Serial.println(data_1.temp_str); // в градусах 
      Serial.println(" "); 
//      Serial.println(data_1.bmp_temp_str); // в градусах
      Serial.print(";");
      }
      else if(!digitalRead(test_3))
      {
      Serial.print("$");
      Serial.println(data_1.x_str/3.26197); // в м/с2 
      Serial.println(" "); 
      Serial.println(data_1.y_str/3.26197); // в м/с2 
      Serial.println(" "); 
      Serial.println(data_1.z_str/3.26197); // в м/с2 
      Serial.print(";");

      }
      else
      {
 //      Serial.print("Temperature (ds18b20):");            // 
       Serial.println(data_2.timer);                     // пишем в порт температуру 
 //      Serial.print("Pressure (bmp280):");                // 
       Serial.println(data_1.x_str);                  // пишем в порт давление с барометра
  /*     Serial.print("Temperature (bmp280):");             //
       Serial.print(data_2.bmp_temp_str);                 // пишем в порт температуру с барометра
       Serial.print("X (adxl345):");                      //
       Serial.print(" ");                      //
       Serial.println(data_2.timer);  
       Serial.print(" ");  // пишем в порт ускорение по оси X
       Serial.print("Y (adxl345):");                      //   
       Serial.println(data_2.y_str*3);  
       Serial.print(" ");  // пишем в порт ускорение по оси Y
       Serial.print("Z (adxl345):");                      //
       Serial.println(data_2.z_str*3);                        // пишем в порт ускорение по оси Z
  */     Serial.println("");                        // пишем в порт ускорение по оси Z
       Serial.println("");    
       Serial.println("");    
       Serial.println("");          
      }
    }
}
