#include <iarduino_GPS_NMEA.h>                    //  Подключаем библиотеку для расшифровки строк протокола NMEA получаемых по UART.
iarduino_GPS_NMEA gps;                            //  Объявляем объект gps для работы с функциями и методами библиотеки iarduino_GPS_NMEA.
int16_t PPM;
int8_t T;                                             //
void setup(){                                     //
     Serial.begin(9600);                          //  Инициируем работу с аппаратной шиной UART для вывода данных в монитор последовательного порта на скорости 9600 бит/сек.
     Serial2.begin(9600);                         //  Инициируем работу с аппаратной шиной UART для получения данных от GPS модуля на скорости 9600 бит/сек.
     gps.begin(Serial2);                          //  Инициируем расшифровку строк NMEA указав объект используемой шины UART.
     Serial1.begin(9600);
     Serial1.setTimeout(80);
}                                                 //
                                                  //

void loop(){                                      //
     gps.read();                              
     if(gps.errPos){                            
       Serial.println("Координаты недостоверны"); 
       delay(2000); return;                       
     }                                            //                 
     Serial.print(" "); Serial.print(gps.latitude ,5); Serial.print("°, ");
     Serial.print(" "); Serial.print(gps.longitude,5); Serial.print("°.\r\n");
     if (get_MH_Z14A_data(PPM, T))
     {
       Serial.print(T);
       Serial.println(" C");
       Serial.print(PPM);
       Serial.println("PPM");
    
  }
  else
  {
    Serial.println("ERR");
  }
  delay(200);

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
  Serial1.flush();
  Serial1.write(send_request, 9);
  if(Serial1.readBytes(get_value, 9) == 9)
  {
  
    uint8_t crc = 0;
    for (int i = 1; i < 8; i++) crc += get_value[i]; //считаем контрольную сумму по формуле из даташита
    crc = ~crc;
    crc++;
    if (!(get_value[0] == 0xFF && get_value[1] == 0x86 && get_value[8] == crc))
      return 0;
    highCh = uint8_t(get_value[2]);
    lowCh =  uint8_t(get_value[3]);
    ppm = highCh * 256 + lowCh;
    temp = get_value[4] - 40;
    return 1;
  }
  else return 0;

}
