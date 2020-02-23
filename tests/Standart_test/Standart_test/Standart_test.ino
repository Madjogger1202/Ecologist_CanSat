// первый тест атмеги
#define Led_pin 8
#define Delay_time 500
void setup() {
  pinMode(Led_pin, OUTPUT);
}

void loop() {
  digitalWrite(Led_pin, HIGH);
  delay(Delay_time);
  digitalWrite(Led_pin, LOW);
  delay(Delay_time);
}
