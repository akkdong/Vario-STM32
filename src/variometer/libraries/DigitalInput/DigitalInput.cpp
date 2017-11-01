// DigitalInput.cpp
//

#include <Arduino.h>
#include "DigitalInput.h"

enum INPUT_MODE
{
	WAIT_INPUT,	// wait first input
	TRANS_DOWN,	
	TRANS_UP,
	STATE_DOWN,
	COMP_INPUT,	// composite each input
};

enum BUTTON_STATE
{
	BUTTON_DOWN = 0,
	BUTTON_UP
};

///////////////////////////////////////////////////////////////////////////////////////////////
// class DigitalInput

DigitalInput::DigitalInput()
{
}

void DigitalInput::begin(int pin)
{
	inputPin = pin;
	//pinMode(inputPin, INPUT);
	
	inputState = WAIT_INPUT;	
	inputIndex = -1;
	inputValue = 0;
	
	returnValue = 0;
	
	lastTick = millis();	
}

void DigitalInput::update()
{
	int input = digitalRead(inputPin);
	
	switch (inputState)
	{
	case WAIT_INPUT :
		if (input == BUTTON_DOWN)
		{			
			inputState = TRANS_DOWN;
			lastTick = millis();
			
			inputIndex = -1;
			inputValue = 0;			
		}
		break;
		
	case TRANS_DOWN :
		if (millis() - lastTick > FKEY_MIN_DEBOUNCE_TIME)
		{
			if (input == BUTTON_DOWN)
			{
				inputState = STATE_DOWN;
				lastTick = millis();
				
				inputIndex += 1;
			}
			else
			{
				//if (inputIndex < 0)
				//{
				//	inputState = WAIT_INPUT;
				//}
				//else
				{
					inputState = COMP_INPUT;
					lastTick = millis();
				}
			}
		}
		// else ignore
		break;
		
	case STATE_DOWN :
		if (input == BUTTON_UP)
		{
			if (millis() - lastTick > FKEY_MIN_SHORTKEY_TIME)
			{
				// LONG press
				inputValue += (1 << inputIndex);
			}

			inputState = TRANS_UP;
			lastTick = millis();
		}
		// else ignore
		break;
		
	case TRANS_UP :
		if (millis() - lastTick > FKEY_MIN_DEBOUNCE_TIME)
		{
			inputState = COMP_INPUT;
			lastTick = millis();
		}
		// else ignore
		break;

	case COMP_INPUT :
		if (input == BUTTON_DOWN)
		{
			inputState = TRANS_DOWN;
			lastTick = millis();
		}
		else
		{
			if (millis() - lastTick > FKEY_MIN_FIRE_TIME)
			{
				returnValue = ((inputIndex + 1) << 5) + inputValue;
				inputState = WAIT_INPUT;
			}
		}
		break;
	}
}

boolean DigitalInput::fired()
{
	return (returnValue > 0 ? true : false);
}

int DigitalInput::getInput()
{
	int input = digitalRead(inputPin);
	
#if FKEY_INPUT_ACTIVE
	// active high (press(down) high)
	// 
	return input ? BUTTON_DOWN : BUTTON_UP;
#else
	// active low (press(down) low)
	return input ? BUTTON_UP : BUTTON_DOWN;
#endif // FKEY_INPUT_ACTIVE
}

unsigned char DigitalInput::getValue()
{
	unsigned char value = returnValue;
	returnValue = 0;
	
	return value;
}
