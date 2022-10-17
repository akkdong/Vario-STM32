// BatteryVoltage.h
//

#ifndef __BATTERYVOLTAGE_H__
#define __BATTERYVOLTAGE_H__

#include <DefaultSettings.h>
#include <Arduino.h>


///////////////////////////////////////////////////////////////////////////
// class BatteryVoltage

class BatteryVoltage
{
public:
	void						begin(uint8_t pin);
	void						update();
	
	float						getVoltage();
	
private:
	float						measVoltage;
	
	uint8_t						adcPin;
	uint32_t					lastTick;
};

#endif // __BATTERYVOLTAGE_H__
