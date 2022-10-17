// LEDFlasher.h
//

#ifndef __LEDFLASHER_H__
#define __LEDFLASHER_H__

#include <DefaultSettings.h>
#include <Arduino.h>

enum BLINK_TYPE 
{
	BTYPE_LONG_ON_OFF,			// ----____
	BTYPE_SHORT_ON_OFF,			// -_
	BTYPE_LONG_ON_SHORT_OFF,	// ----_
	BTYPE_SHORT_ON_LONG_OFF,	// -____
	BTYPE_BLINK_2_LONG_ON,		// -_-_----_
	BTYPE_BLINK_3_LONG_ON,		// -_-_-_----_
	BTYPE_BLINK_2_LONG_OFF,		// -_-_____
	BTYPE_BLINK_3_LONG_OFF,		// -_-_-_____

	BLINK_TYPE_COUNT,
	BLINK_NONE = -1
};


///////////////////////////////////////////////////////////////////////////////////////////////
// class LEDFlasher

class LEDFlasher
{
public:
	LEDFlasher();
	
public:
	void				begin(uint8_t pin, uint8_t active);
	
	void				update();
	
	void				blink(uint8_t type);
	void				turnOn();
	void				turnOff();

private:
	void				ledOn();
	void				ledOff();
	
private:
	uint8_t				blinkPin;
	uint8_t				activeState;
	
	int8_t				blinkType;
	int8_t				indexPattern;
	int16_t				blinkInterval;
	
	uint32_t			blinkTick;	
};

#endif // __LEDFLASHER_H__
