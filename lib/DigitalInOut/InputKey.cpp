// InputKey.cpp
//

#include "InputKey.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// class InputKey

void InputKey::begin(uint8_t pin, uint8_t active)
{
	//
	inputPin = pin;
	activeState = active;
	
	//
	pinMode(pin, INPUT);
}

int InputKey::read()
{
	uint32_t input = digitalRead(inputPin);
	
	if (activeState)
		// active-high : HIGH -> INPUT_ACTIVE, LOW -> INPUT_INACTIVE
		return input ? INPUT_ACTIVE : INPUT_INACTIVE;
	else
		// active-low : HIGH -> INPUT_INACTIVE, LOW -> INPUT_ACTIVE
		return input ? INPUT_INACTIVE : INPUT_ACTIVE; 
}

void InputKey::interrupt()
{
}
