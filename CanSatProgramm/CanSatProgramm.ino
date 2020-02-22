


/* 
  Основная программа аппарата "Эколог"
  
  Модули, работа которых реализуется: 
            1)датчик температуры
            2)датчик давления
            3)акселерометр  (сначала я напишу для mpu6050, так как он у меня есть)
            4)датчик кислорода
            5)датчик углекислого газа
            6)датчик примесей
            7)радиомодуль
            8)модуль SD карты
            9)дозиметр
            10)GPS
             
  Во-первых, частота(промежутки времени) замеров: 
    1)Барометр:
        разрешение ~ 2 м => замеры делаем каждые 0,5 секунд (при скорости падения 8 м/с)
    2)Датчик температуры:
        раз в секунду (не является константой, вопрос открыт)
    3)Акселерометр:
        раз в 0,5 секунды (тоже самое, что и с температурой)
    4)Датчик кислорода:
        раз в 0,5 секунду (не является константой, вопрос открыт)
    5)Датчик примесей:
        раз в 0,5 секунду (не является константой, вопрос открыт)
    6)Датчик CO2:
        раз в 0,5 секунду (не является константой, вопрос открыт)
    7)Дозиметр:
        тут сложнее, но замеры промежуточные будут получаться где-то через секунд 5, 
        и это при времени более-менее точного замера в 75 секунд (вопрос открыт)
    8)Не совсем замер, но тоже вписывается: Радиомодуль+SD карточка+GPS: 
        пакеты собираются и отправляются где-то раз в 0,5 секунды,
        надо посоветоваться (в пакеты можно собрать сразу пачку значений за несколько промежутков времени)
        (вопрос открыт)

  Ссылки в помощь:
      Датчик кислорода: http://wiki.seeedstudio.com/Grove-Gas_Sensor-O2/
      Датчик СО2: https://domoticx.com/arduino-co2-sensor-mh-z14/ (сайт на норвежском)
      Датчик примесей: увы, тут как повезёт 
      барометр: из примера 
      термометр: из примера
      акселерометр: из примера
      радиомодуль: http://arduino.zl3p.com/modules/radio
      sd карта: https://robotclass.ru/tutorials/arduin-read-write-micro-sd-card/
      дозиметр: https://cxem.net/dozimetr/3-10.php

      Модули, используемые в тестовой программе:
      1)Модуль СД карты
      2)BMP280
      3)DS18B20
      4)MPU6050
      
      Структура тестовой программы (без отправки по радио)
      {
        подключение библиотек
        инициализация переменных 


      }


*/ 

#define TEMP_PIN 2 // вешаем датчик температуры на 2 ногу
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_BMP280.h"
Adafruit_BMP280 barometr; // объект нашего барометра
#include "I2Cdev.h"     //Библиотека для работы с I2C устройствами
#include "MPU6050.h"    //Библиотека для работы с MPU6050
#include "microDS18B20.h"
MicroDS18B20 temp_sensor(TEMP_PIN); // определяем наш датчик на его пин 
MPU6050 accelgyro;
const int chipSelect = 4;
int HeightCalibrate;  // калибровка высоты (корректировка по первоначальной высоте(высоте, на которой произошла инициализация ))
int Height;  // текущая высота относительно точки запуска
float Temp1;  // температура с DS18B20 (основная)
int Temp2;  // температура с BMP280
int Pressure;  // давление с BMP280
/*   переменные для значений с акселерометра    */
int x_s;  // статичное по X
int y_s;  // статичное по Y
int z_s;  // статичное по Z
int x_a;  // ускорение по X
int y_a;  // ускорение по Y
int z_a;  // ускорение по Z

void setup() {
  Serial.begin(9600);
  accelgyro.initialize();  
  
  Serial.println("   /// АППАРАТ ЭКОЛОГ К РАБОТЕ ГОТОВ ///   ");
  Serial.println("   /// ИНИЦИАЛИЗАЦИЯ БАРОМЕТРА...    ///");
  
  delay(100);
}

void loop() {
  delay(1000);
}


void SD_write(String dataString)
{
   if (dataFile) 
   {
      dataFile.println(dataString);
      dataFile.close();
   } else 
   {
      Serial.println("Ошибка записи!");
   }
}


void Barometr_settings()
{
  barometr.setSampling(Adafruit_BMP280::MODE_NORMAL,     
                       Adafruit_BMP280::SAMPLING_X2,     
                       Adafruit_BMP280::SAMPLING_X16,    
                       Adafruit_BMP280::FILTER_X16,      
                       Adafruit_BMP280::STANDBY_MS_500);
}
void Get_temp_and_pressure()
{
  temp_sensor.requestTemp();
  Pressure = barometr.readPressure();
  Temp1 = temp_sensor.getTemp();
}
void Get_accel()
{
  accelgyro.getMotion6(&x_a, &y_a, &z_a, &x_s, &y_s, &z_s); 
}
int SD_error()
{
  if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        return 1;
    }
}
int BMP_error()
{
  if (!barometr.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
}
int DS18_error()
{
  return(0);
}
int NRF_error()
{
  return(0);
}
