#define _MHZ14_H
#include <arduino.h>

class MHZ14{
	public:
	MHZ14(uint8_t pin_);// initializer
	long read();
	unsigned long timeout = 2100;// in milliseconds; // cycle time is about 1004 milliseconds so it must be higher than that
	long long last;// last value read
	float PPM_TO_MG_M3 = 1.7758; // ppm to mg/m^3 conversion, 25°C and 100 kPa
	void calibrate(uint8_t pin_);// calibrate function
	bool DEBUG = false;// debug 
	
	private:
	uint8_t unit = 0; // units in PPM = 0; mg/m3 = 1
	uint16_t model = 5000;// there are three models, 2000ppm, 5000ppm and 10000ppm
	bool timeoutt();// exit if timeout
	uint8_t pin;// PWM signal input pin
	unsigned long timer, high, low, timeouttimer;// timer, high time, low time;
	long CO2PPM;//-1 indicates read failed
	bool status = false;
	
	
};

// to do:: calibrate function
