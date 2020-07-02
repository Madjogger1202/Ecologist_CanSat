volatile int counter = 0;  // переменная-счётчик
void setup() 
{
  Serial.begin(9600);
  pinMode(2, INPUT); // 2 пин поддерживает прерывания = на многих платах ардуино
  attachInterrupt(0, buttonTick, FALLING); // 0 - это номер прерывания, а не номер пина, про прерывания почитать можно туть: https://alexgyver.ru/lessons/interrupts/
}

void buttonTick()
{
  counter++;  // + нажатие
}

void loop()
{ 
  int limit = map(counter, 0, 200, 0, 20); // синтаксис функции: источник, 
                                           //мин знач. из источника 
                                           //макс знач из источника
                                           //мин значение для новой переменной
                                           //макс знач для новой переменной
                                           // фукция служит для того, чтобы не высчитывать каждый раз новый коэф., а так - это просто деление с опциональной инверсией
  for (int i = 0; i < limit; i++)
  {
    Serial.print("#");
  }
  Serial.print(" ");
  Serial.println(counter);
  counter = 0;// выводим
  delay(60000);              // ждём
}
