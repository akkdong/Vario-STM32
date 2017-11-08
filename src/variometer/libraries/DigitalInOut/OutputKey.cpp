// OutputKey.cpp
//

#include "OutputKey.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// class OutputKey

void OutputKey::begin(uint8_t pin, uint8_t active, uint8_t init)
{
	//
	outputPin = pin;
	activeState = active;
	
	//
	pinMode(pin, OUTPUT);

	//
	if (init)
		enable();
	else
		disable();
}

void OutputKey::enable()
{
	// active-high : enable -> HIGH
	// actgive-low : enable -> LOW
	digitalWrite(outputPin, activeState ? HIGH : LOW);
}

void OutputKey::disable()
{
	// active-high : disable -> LOW
	// actgive-low : disable -> HIGH
	digitalWrite(outputPin, activeState ? LOW : HIGH);
}
