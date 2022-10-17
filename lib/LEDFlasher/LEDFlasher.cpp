// LEDFlasher.cpp
//

#include "LEDFlasher.h"


///////////////////////////////////////////////////////////////////////////////////////////////
//

typedef struct tagBlinkPattern
{
	int16_t 	count;
	int16_t *	pattern;
} BlinkPattern;

int16_t pattern1[] = { 800, -800 };
int16_t pattern2[] = { 200, -200 };
int16_t pattern3[] = { 800, -200 };
int16_t pattern4[] = { 200, -800 };

int16_t pattern5[] = { 200, -200, 200, -200, 800, -200 };
int16_t pattern6[] = { 200, -200, 200, -200, 200, -200, 800, -200 };
int16_t pattern7[] = { 200, -200, 200, -800 };
int16_t pattern8[] = { 200, -200, 200, -200, 200, -800 };

BlinkPattern blinkPattern[] = 
{
	{ 2, pattern1 },	// BTYPE_LONG_ON_OFF
	{ 2, pattern2 },	// BTYPE_SHORT_ON_OFF
	{ 2, pattern3 },	// BTYPE_LONG_ON_SHORT_OFF
	{ 2, pattern4 },	// BTYPE_SHORT_ON_LONG_OFF
	{ 6, pattern5 },	// BTYPE_BLINK_2_LONG_ON
	{ 8, pattern6 },	// BTYPE_BLINK_3_LONG_ON
	{ 4, pattern7 },	// BTYPE_BLINK_2_LONG_OFF
	{ 6, pattern8 },	// BTYPE_BLINK_3_LONG_OFF
};


///////////////////////////////////////////////////////////////////////////////////////////////
// class LEDFlasher

LEDFlasher::LEDFlasher()
{
	blinkType = BLINK_NONE;
}

void LEDFlasher::begin(uint8_t pin, uint8_t active)
{
	blinkPin = pin;
	activeState = active;
	
	pinMode(blinkPin, OUTPUT);
}

void LEDFlasher::update()
{
	if (blinkType == BLINK_NONE)
		return;
	
	if ((millis() - blinkTick) > blinkInterval)
	{
		indexPattern = (indexPattern + 1) % blinkPattern[blinkType].count;
		blinkInterval = blinkPattern[blinkType].pattern[indexPattern];
		
		if (blinkInterval < 0)
			ledOff();
		else
			ledOn();
		
		blinkInterval = abs(blinkInterval);
		blinkTick = millis();
	}
}

void LEDFlasher::blink(uint8_t type)
{
	switch (type)
	{
	case BTYPE_LONG_ON_OFF		:
	case BTYPE_SHORT_ON_OFF		:
	case BTYPE_LONG_ON_SHORT_OFF		:
	case BTYPE_SHORT_ON_LONG_OFF		:
	case BTYPE_BLINK_2_LONG_ON		:
	case BTYPE_BLINK_3_LONG_ON		:
	case BTYPE_BLINK_2_LONG_OFF		:
	case BTYPE_BLINK_3_LONG_OFF		:
		blinkType = type;
		indexPattern = 0;
		blinkInterval = blinkPattern[blinkType].pattern[0];
		
		if (blinkInterval < 0)
			ledOff();
		else
			ledOn();
		
		blinkInterval = abs(blinkInterval);
		blinkTick = millis();
		break;
	case BLINK_NONE:
		turnOff();
		break;
	}
}

void LEDFlasher::turnOn()
{
	blinkType = BLINK_NONE;
	ledOn();
}

void LEDFlasher::turnOff()
{
	blinkType = BLINK_NONE;
	ledOff();
}

void LEDFlasher::ledOn()
{
	digitalWrite(blinkPin, activeState ? HIGH : LOW);
}

void LEDFlasher::ledOff()
{
	digitalWrite(blinkPin, activeState ? LOW : HIGH);
}
