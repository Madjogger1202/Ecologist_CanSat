#include <SPI.h>                                       
#include <nRF24L01.h>                                     
#include <RF24.h>                                         
                                      
RF24 radio(10, 9);         
                              
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
void setup()
{
    Serial.begin(115200);                                    
    radio.begin();                                        // Инициируем работу nRF24L01+
    radio.setChannel(20);                                // Указываем канал приёма данных (от 0 до 127), 5 - значит приём данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
    radio.setDataRate     (RF24_2MBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
    radio.setPALevel      (RF24_PA_HIGH);                 // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
    radio.openReadingPipe (1, 0xE7E7E2E7E7);            // Открываем 1 трубу с идентификатором 0x1234567890 для приема данных (на ожном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
    radio.startListening  ();                             // Включаем приемник, начинаем прослушивать открытую трубу
}
void loop()
{
    if(radio.available())
    {                                
       radio.read(&data, sizeof(data));                   // читаем данные и указываем сколько байт читать
       Serial.print("Temperature (ds18b20):");            // 
       Serial.println(data.temp_str);                     // пишем в порт температуру 
       Serial.print("Pressure (bmp280):");                // 
       Serial.println(data.press_str);                  // пишем в порт давление с барометра
       Serial.print("Temperature (bmp280):");             //
       Serial.println(data.bmp_temp_str);                 // пишем в порт температуру с барометра
       Serial.print("X (adxl345):");                      //
       Serial.print(" ");                      //
       Serial.println(data.x_str);  
       Serial.print(" ");  // пишем в порт ускорение по оси X
       Serial.print("Y (adxl345):");                      //   
       Serial.println(data.y_str);  
       Serial.print(" ");  // пишем в порт ускорение по оси Y
       Serial.print("Z (adxl345):");                      //
       Serial.println(data.z_str);                        // пишем в порт ускорение по оси Z
       Serial.println("");                        // пишем в порт ускорение по оси Z
       Serial.println("");    
       Serial.println("");    
       Serial.println("");       
    }
}
