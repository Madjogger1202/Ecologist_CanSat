  unsigned int sum = 0;
const float VRefer = 21.59;       // voltage of adc reference
const int pinAdc   = A0;
float Vout =0;
void setup() 
{
  //pinMode(A0, INPUT_PULLUP);
    Serial.begin(9600);
}

void loop() 
{
    Serial.print(" Concentration of O2 is ");
    Serial.println(readO2Vout());
    delay(500);
}

float readO2Vout()
{
  for (unsigned char i = 32;i > 0;i--)
    {
      sum = sum + analogRead(pinAdc);
      delay(50);
    }
    sum = sum >> 5;
    //SerialUSB.println(sum);
    float output = sum / 8.08;
    //SerialUSB.println(sum);
    return output;
}
