// LEDFlasher.cpp
//

#include "LEDFlasher.h"


///////////////////////////////////////////////////////////////////////////////////////////////
//

typedef struct tagBlinkPattern
{
	int8_t 		count;
	int8_t *	pattern;
} BlinkPattern;

int8_t pattern1[] = { 300, -100, 100, -100, 100, -100, 100, -100 };
int8_t pattern2[] = { 300, -100, 100, -100, 100, -100 };
int8_t pattern3[] = { 300, -300 };
int8_t pattern4[] = { 300, -100 };
int8_t pattern5[] = { 100, -300 };
int8_t pattern6[] = { 100, -100 };

BlinkPattern blinkPattern[] = 
{
	{ 8, pattern1 },	// BLINK_IMU_FAILED	
	{ 6, pattern2 },	// BLINK_SD_FAIELD		
	{ 2, pattern3 },	// BLINK_ACCEL_MEASURE	
	{ 2, pattern4 },	// BLINK_MEASURE_VALID	
	{ 2, pattern5 },	// BLINK_MEASURE_INVALID
	{ 2, pattern6 },	// BLINK_FIRMWARE_UPDATE
};


///////////////////////////////////////////////////////////////////////////////////////////////
// class LEDFlasher

LEDFlasher::LEDFlasher()
{
	blinkType = BLINK_NONE;
}

void LEDFlasher::begin(uint8_t pin)
{
	blinkPin = pin;
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
	case BLINK_IMU_FAILED		:
	case BLINK_SD_FAIELD		:
	case BLINK_ACCEL_MEASURE	:
	case BLINK_MEASURE_VALID	:
	case BLINK_MEASURE_INVALID	:
	case BLINK_FIRMWARE_UPDATE	:
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
#if FLASHER_LED_ACTIVE
	// active high
	digitalWrite(blinkPin, HIGH);
#else
	// active low
	digitalWrite(blinkPin, LOW);
#endif // FLASHER_LED_ACTIVE
}

void LEDFlasher::ledOff()
{
#if FLASHER_LED_ACTIVE
	// active high
	digitalWrite(blinkPin, LOW);
#else
	// active low
	digitalWrite(blinkPin, HIGH);
#endif // FLASHER_LED_ACTIVE
}
