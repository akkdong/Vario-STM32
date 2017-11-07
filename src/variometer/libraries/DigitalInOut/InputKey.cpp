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
		// active-high : HIGH -> INPUT_HIGH, LOW -> INPUT_LOW
		return input ? INPUT_HIGH : INPUT_LOW;
	else
		// active-low : HIGH -> INPUT_LOW, LOW -> INPUT_HIGH
		return input ? INPUT_LOW : INPUT_HIGH; 
}

void InputKey::interrupt()
{
}
