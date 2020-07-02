volatile int counter = 0;  // переменная-счётчик
void setup() {
  Serial.begin(9600); 
  pinMode(2, INPUT); 
  attachInterrupt(0, buttonTick, FALLING);
}
void buttonTick() {
  counter++;  // + нажатие
}
void loop() {
  for(int i =0; i<counter; i++)
  {
    Serial.print("#");
  }
  Serial.print(" ");
  Serial.println(counter);
  counter = 0;// выводим
  delay(60000);              // ждём
}
