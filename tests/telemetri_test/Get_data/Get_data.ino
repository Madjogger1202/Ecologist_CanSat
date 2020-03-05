#include <SPI.h>                                       
#include <nRF24L01.h>                                     
#include <RF24.h>                                         
                                      
RF24 radio(10, 9);         
                                
long int data[5];                                         // Создаём массив для приёма данных

void setup()
{
    Serial.begin(9600);                                    
    radio.begin();                                        // Инициируем работу nRF24L01+
    radio.setChannel(126);                                // Указываем канал приёма данных (от 0 до 127), 5 - значит приём данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
    radio.setDataRate     (RF24_2MBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
    radio.setPALevel      (RF24_PA_HIGH);                 // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
    radio.openReadingPipe (1, 0x1234567899LL);            // Открываем 1 трубу с идентификатором 0x1234567890 для приема данных (на ожном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
    radio.startListening  ();                             // Включаем приемник, начинаем прослушивать открытую трубу
}
void loop()
{
    if(radio.available())
    {                                
       radio.read(&data, sizeof(data));                   // Читаем данные в массив data и указываем сколько байт читать
       Serial.print("Temperature (ds18b20):");
       Serial.println(data[1]);
       Serial.print("Pressure (bmp280):");
       Serial.println(data[0]);
       Serial.print("X (adxl345):");
       Serial.println(data[2]);
       Serial.print("Y (adxl345):");
       Serial.println(data[3]);
       Serial.print("Z (adxl345):");
       Serial.println(data[4]);
    }
}
