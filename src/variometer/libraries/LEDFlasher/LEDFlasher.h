// LEDFlasher.h
//

#ifndef __LEDFLASHER_H__
#define __LEDFLASHER_H__

#include <DefaultSettings.h>
#include <Arduino.h>

enum BLINK_TYPE 
{
	BLINK_IMU_FAILED		, // 0 : ---_-_-_-_
	BLINK_SD_FAIELD			, // 1 : ---_-_-_
	BLINK_ACCEL_MEASURE		, // 2 : ---___
	BLINK_MEASURE_VALID		, // 3 : ---_
	BLINK_MEASURE_INVALID	, // 4 : -___
	BLINK_FIRMWARE_UPDATE	, // 5 : -_
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
