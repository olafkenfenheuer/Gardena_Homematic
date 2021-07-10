// drv8838.cpp
// Basic Arduino library for the TI DRV8838 motor driver
// Christopher Green - Use this code at your own risk/peril

#include "drv8838.h"

uint8_t _EN_pin ;
uint8_t _PH_pin ;
uint8_t _SLEEP_pin ;


//drv8838:drv8838(); //default constructor

drv8838::drv8838(uint8_t ENABLE, uint8_t PHASE, uint8_t SLEEP){ //Constructor that sets pins for board - enable should be PWM
	_EN_pin = ENABLE;
	_PH_pin = PHASE;
	_SLEEP_pin = SLEEP;

	digitalWrite(_EN_pin, LOW);
	pinMode(_EN_pin, OUTPUT);
	digitalWrite(_EN_pin, LOW);

	digitalWrite(_PH_pin, LOW);
	pinMode(_PH_pin, OUTPUT);
	digitalWrite(_PH_pin, LOW);

	digitalWrite(_SLEEP_pin, LOW);
	pinMode(_SLEEP_pin, OUTPUT);
	digitalWrite(_SLEEP_pin, LOW);
}
/*
void drv8838::configure(uint8_t ENABLE, uint8_t PHASE){ //Manually set pin values
	_EN_pin = ENABLE;
	_PH_pin = PHASE;
	pinMode(_EN_pin, OUTPUT);
	pinMode(_PH_pin, OUTPUT);
}
*/

bool drv8838::_check_valid(){ //check that configuration is set before doing anything
	return (_EN_pin && _PH_pin);
}

void drv8838::setDirection(boolean direction){ //set motor direction
	if(_check_valid())
		digitalWrite(_PH_pin, direction);
}

void drv8838::setSpeed(uint8_t speed){ //set motor speed (0-255)
	if(_check_valid())
		analogWrite(_EN_pin, speed);
}

void drv8838::sleep(boolean sleep){ //set motor direction
	if(_check_valid())
		digitalWrite(_SLEEP_pin, sleep);
}

