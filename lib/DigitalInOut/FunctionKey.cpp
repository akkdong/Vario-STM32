// FunctionKey.cpp
//

#include <Arduino.h>
#include "FunctionKey.h"

#define DELAY_LONG	250
#define DELAY_SHORT	100


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
	toneCount = 0;
	
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
			{
				inputValue = 0xFFFF; 
				inputIndex = 0;
				
				tone[0].freq = NOTE_G4;
				tone[0].length = DELAY_LONG * 2;
			}
			else if (millis() - lastTick > FKEY_MIN_SHORTKEY_TIME)
			{
				inputValue += 1; // LONG press

				tone[inputIndex*2].freq = NOTE_G4;
				tone[inputIndex*2].length = DELAY_LONG;
			}
			else
			{
				tone[inputIndex*2].freq = NOTE_G4;
				tone[inputIndex*2].length = DELAY_SHORT;
			}
			
			tone[inputIndex*2+1].freq = 0;
			tone[inputIndex*2+1].length = DELAY_SHORT;
			

			//
			inputState = TRANS_UP;
			lastTick = millis();
		}
		// else ignore
		break;
		
	case TRANS_UP :
		if (millis() - lastTick > FKEY_MIN_DEBOUNCE_TIME)
		{
			if (inputValue == 0xFFFF)
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
				returnValue = ((inputIndex + 1) << 12) + inputValue;
				inputState = WAIT_INPUT;
				toneCount = (inputIndex + 1) * 2;
			}
		}
		break;
		
	case LONGLONG_INPUT :
		returnValue = inputValue;
		inputState = WAIT_INPUT;
		toneCount = 2;
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

uint16_t FunctionKey::getValue()
{
	uint16_t value = returnValue;
	returnValue = 0;
	
	return value;
}
