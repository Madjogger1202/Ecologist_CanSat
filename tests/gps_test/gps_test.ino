#include <iarduino_GPS_NMEA.h>                    //  Подключаем библиотеку для расшифровки строк протокола NMEA получаемых по UART.
iarduino_GPS_NMEA gps;                            //  Объявляем объект gps для работы с функциями и методами библиотеки iarduino_GPS_NMEA.
                                                  //
void setup(){                                     //
     Serial.begin(9600);                          //  Инициируем работу с аппаратной шиной UART для вывода данных в монитор последовательного порта на скорости 9600 бит/сек.
     Serial1.begin(9600);                         //  Инициируем работу с аппаратной шиной UART для получения данных от GPS модуля на скорости 9600 бит/сек.
     gps.begin(Serial1);                          //  Инициируем расшифровку строк NMEA указав объект используемой шины UART.
}                                                 //
                                                  //

void loop(){                                      //
//   Читаем данные:                               //  Чтение может занимать больше 1 секунды.
     gps.read();                                  //  Функцию можно вызвать с указанием массива для получения данных о спутниках.
//   Проверяем достоверность координат:           //
     if(gps.errPos){                              //  Если данные не прочитаны (gps.errPos=1) или координаты недостоверны (gps.errPos=2), то ...
       Serial.println("Координаты недостоверны"); //  Выводим сообщение об ошибке.
       delay(2000); return;                       //  Ждём 2 секунды и выполняем функцию loop() с начала.
     }                                            //                 //
     Serial.print(" "); Serial.print(gps.latitude ,5); Serial.print("°, ");
     Serial.print(" "); Serial.print(gps.longitude,5); Serial.print("°.\r\n");
}    
