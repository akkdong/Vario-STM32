// FunctionKey.cpp
//

#include <Arduino.h>
#include "FunctionKey.h"

enum INPUT_MODE
{
	WAIT_INPUT,	// wait first input
	TRANS_DOWN,	
	TRANS_UP,
	STATE_DOWN,
	COMP_INPUT,	// composite each input
	LONGLONG_INPUT,
};

enum BUTTON_STATE
{
	BUTTON_DOWN = 0,
	BUTTON_UP
};

///////////////////////////////////////////////////////////////////////////////////////////////
// class FunctionKey

FunctionKey::FunctionKey()
{
}

void FunctionKey::begin(int pin, int active)
{
	inputPin = pin;
	activeState = active;
	pinMode(inputPin, INPUT);
	
	inputState = WAIT_INPUT;	
	inputIndex = -1;
	inputValue = 0;
	
	returnValue = 0;
	
	lastTick = millis();	
}

void FunctionKey::update()
{
//	int input = digitalRead(inputPin);
	int input = getInput();
	
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
			//
			inputValue = (inputValue << 1);
			
			if (millis() - lastTick > FKEY_MIN_LONGLONGKEY_TIME)
				inputValue = 0xFF; 
			else if (millis() - lastTick > FKEY_MIN_SHORTKEY_TIME)
				inputValue += 1; // LONG press

			//
			inputState = TRANS_UP;
			lastTick = millis();
		}
		// else ignore
		break;
		
	case TRANS_UP :
		if (millis() - lastTick > FKEY_MIN_DEBOUNCE_TIME)
		{
			if (inputValue == 0xFF)
				inputState = LONGLONG_INPUT;
			else
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
		
	case LONGLONG_INPUT :
		returnValue = 0xFF;
		inputState = WAIT_INPUT;
		break;
	}
}

boolean FunctionKey::fired()
{
	return (returnValue > 0 ? true : false);
}

int FunctionKey::getInput()
{
	int input = digitalRead(inputPin);
	
	if (activeState == ACTIVE_HIGH)
	{
		// active high (press(down) high)
		return input ? BUTTON_DOWN : BUTTON_UP;
	}
	else
	{
		// active low (press(down) low)
		return input ? BUTTON_UP : BUTTON_DOWN;
	}
}

unsigned char FunctionKey::getValue()
{
	unsigned char value = returnValue;
	returnValue = 0;
	
	return value;
}
