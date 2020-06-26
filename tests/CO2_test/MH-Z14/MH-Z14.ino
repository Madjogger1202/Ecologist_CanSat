
//#define RXD 10  // по даташиту - TX датчика на 11 контакте
//#define TXD 11 // по даташиту - RX датчика на 10 контакте

//#include <SoftwareSerial.h>       // для софтварного uart
//SoftwareSerial MH_Z14A(RXD, TXD); // инициализация
int16_t PPM;
int8_t T;
#include "TM1637.h"
#define CLK 6//pins definitions for TM1637 and can be changed to other ports       
#define DIO 5
TM1637 tm1637(CLK,DIO);
void setup()
{ Serial.begin(9600);
  Serial1.begin(9600);
 // MH_Z14A.begin(9600);
  Serial1.setTimeout(80);
  tm1637.init();
  tm1637.set(7);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

}

void loop()
{
  if (get_MH_Z14A_data(PPM, T))
  {
    Serial.print(T);
    Serial.println(" C");
    Serial.print(PPM);
    Serial.println("PPM");
    tm1637.display(0,PPM/1000);
    tm1637.display(1,PPM/100);
    tm1637.display(2,PPM/10);
    tm1637.display(3,PPM%10);
    
    
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
