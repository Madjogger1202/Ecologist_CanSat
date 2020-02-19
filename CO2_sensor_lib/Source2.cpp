#include <SoftwareSerial.h>
SoftwareSerial SerialCom(A1, A0);
int myDelay = 2000;

byte addArray[] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };

char dataValue[9];
String dataString = "";

void setup() {
	Serial.begin(9600);
	SerialCom.begin(9600);
	Serial.println("MH-Z14 C02 Sensor Test Code");
	Serial.println("------------------------------------");
}

void loop() {
	SerialCom.write(addArray, 9);
	SerialCom.readBytes(dataValue, 9);
	int resHigh = (int)dataValue[2];
	int resLow = (int)dataValue[3];
	int pulse = (256 * resHigh) + resLow;
	dataString = String(pulse);
	Serial.print("PPM: ");
	Serial.println(pulse);
	delay(myDelay);
}