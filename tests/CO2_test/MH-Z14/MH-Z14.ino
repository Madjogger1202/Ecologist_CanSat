#define RXD D2  // по даташиту - TX датчика на 11 контакте
#define TXD D1 // по даташиту - RX датчика на 10 контакте
byte send_request[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79}; 
// массив для запроса данных. 1)команда для старта
//                            2)номер датчика, хз нужно ли, в даташите не увидел общей команды
//                            3)сама команда, из даташита: send concentration value of the sensor
//                            4-8)спам нулями
//                            9)запрос контрольной суммы, в даташите не увидел, но в интернеье вроде так ставят
unsigned char get_value[9];
// массив для запроса данных. 1)команда для старта
//                            2)номер датчика, хз нужно ли, в даташите не увидел общей команды
//                            3)в даташите это называется high channel
//                            4)в даташите это называется low channel
//                            5)температура (надеюсь, что сразу в градусах по цельсию)
//                            6-8)спам нулями
//                            9)запрос контрольной суммы, в даташите не увидел, но в интернеье вроде так ставят

int ppm;
uint8_t highCh;
uint8_t lowCh;

#include <SoftwareSerial.h>       // для софтварного uart
SoftwareSerial MH_Z14A(RXD, TXD); // инициализация

void setup()
{
  MH_Z14A.begin(9600); 
  Serial.begin(9600);
}

void loop()
{
  MH_Z14A.write(send_request, 9);
  memset(get_value, 0, 9);
  delay(10);
  MH_Z14A.readBytes(get_value, 9);
  uint8_t crc = 0;
  for (int i = 1; i < 8; i++) crc+=get_value[i]; //считаем контрольную сумму по формуле из даташита
  crc = ~crc;
  crc++;
  if ( !(get_value[0] == 0xFF && get_value[1] == 0x86 && get_value[8] == crc) ) // сравниваем ответ с  контрольной суммой
  {
    Serial.println("CRC error: " + String(crc) + " / "+ String(get_value[0]));
  } 
  highCh = uint8_t(get_value[2]);
  lowCh =  uint8_t(get_value[3]);
  ppm = highCh*256+lowCh;
  Serial.print(get_value[4]-40);
  Serial.println(" C");
  Serial.print(ppm);
  Serial.println("PPM");
  delay(200);
}
