#include "MHZ14.h"


MHZ14::MHZ14(uint8_t pin_){
	pin = pin_; // save pin to read from
	pinMode(pin, INPUT); // set pin mode to input
	}

long MHZ14::read(){
	
	last = CO2PPM;
	// allow one cycle to pass before we start timing
	timeouttimer = millis();
	while(!digitalRead(pin) && !timeoutt()){};// 
	while( digitalRead(pin) && !timeoutt()){};//
  
	// actual timing start now
	timer = micros();// low timing
	while(!digitalRead(pin) && !timeoutt()){};
	low = micros() - timer;

 
	timer = micros();// high timing
	while(digitalRead(pin) && !timeoutt()){};
	high = micros() - timer;
	
	if( !timeoutt() && ( high + low ) > 998000 ){// 998000 to ensure that the pulse length about 1008 millisec to do the calculation
		CO2PPM = 5000 * ( high - 2000) / ( high + low - 4000); // the formula as indicated in the datasheet https://www.compel.ru/pdf-items/winsen/pn/mh-z14a-ndir-co2-sensor/2ef21253651a5e4838151c427769e7b6
		
		if(unit == 1){// convert to mg/M^3
			CO2PPM *= PPM_TO_MG_M3;
		}
	}else{
		CO2PPM = -1;
	}

	if(DEBUG){
		Serial.println("HIGH: "+String(high)+" LOW: "+String(low)+" LENGTH: "+String(high+low)+" PPM: "+String(CO2PPM)+" mg/M^3: "+String(CO2PPM * PPM_TO_MG_M3));
	}
	status = false;
	return CO2PPM;// return CO2 value
}

void MHZ14::calibrate(uint8_t pin_){// calibration
	digitalWrite(pin_, LOW);
	delay(15000);// datasheet indicates that calibration occurs if pin 20 is held low for more than 7 seconds
	digitalWrite(pin_, HIGH);
}
bool MHZ14::timeoutt(){// check if timeout
	
	if(millis() - timeouttimer > timeout){
		status = true; 
	}
	return status;
}
	
