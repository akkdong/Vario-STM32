// BatteryVoltage.cpp
//

#include "BatteryVoltage.h"

#define CLEAR_STATE()		measState = 0
#define SET_STATE(bit)		measState |= (bit)
#define UNSET_STATE(bit)	measState &= ~(bit)

#define IS_SET(bit)			(measState & (bit)) == (bit)


#define ADC_SAMPLING		(1 << 0)


///////////////////////////////////////////////////////////////////////////
// class BatteryVoltage

void BatteryVoltage::begin(uint8_t pin)
{
	//
	pinMode(pin, INPUT_ANALOG);
	
	double total = 0;
	for (int32_t i = 0; i < ADC_MEASURE_COUNT; i++)
		total += analogRead(pin);
	
	//
	adcPin = pin;
	measVoltage = ADC_TO_VOLTAGE(total / ADC_MEASURE_COUNT);	
	lastTick = millis();
}

void BatteryVoltage::update()
{
	if ((millis() - lastTick) > ADC_MEASURE_INTERVAL)
	{
		double value = ADC_TO_VOLTAGE(analogRead(adcPin));
		measVoltage = value * ADC_LPF_FACTOR + measVoltage * (1 - ADC_LPF_FACTOR);
	}
}

double BatteryVoltage::getVoltage()
{
	return measVoltage;
}

