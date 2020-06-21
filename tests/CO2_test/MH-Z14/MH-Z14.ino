#define RXD D2  // по даташиту - TX датчика на 11 контакте
#define TXD D1 // по даташиту - RX датчика на 10 контакте

#include <SoftwareSerial.h>       // для софтварного uart
SoftwareSerial MH_Z14A(RXD, TXD); // инициализация
int16_t PPM;
int8_t T;
void setup()
{
  
  MH_Z14A.begin(9600); 
  MH_Z14A.setTimeout(80);
  Serial.begin(9600);
}

void loop()
{
  get_MH_Z14A_data(PPM, T);
  Serial.print(T);
  Serial.println(" C");
  Serial.print(PPM);
  Serial.println("PPM");
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
  MH_Z14A.flush(); 
  MH_Z14A.write(send_request, 9);
  MH_Z14A.readBytes(get_value, 9);
  uint8_t crc = 0;
  for (int i = 1; i < 8; i++) crc+=get_value[i]; //считаем контрольную сумму по формуле из даташита
  crc = ~crc;
  crc++;
  if(!(get_value[0] == 0xFF && get_value[1] == 0x86 && get_value[8] == crc))
  return 0;
  highCh = uint8_t(get_value[2]);
  lowCh =  uint8_t(get_value[3]);
  ppm = highCh*256+lowCh;
  temp = get_value[4]-40;
  return 1;
}
