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
	
	float total = 0;
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
		int adcValue = analogRead(adcPin);
		float value = ADC_TO_VOLTAGE(adcValue);
		//Serial.print(adcValue); Serial.print(", "); Serial.println(value);
		measVoltage = value * ADC_LPF_FACTOR + measVoltage * (1 - ADC_LPF_FACTOR);
	}
}

float BatteryVoltage::getVoltage()
{
	return measVoltage + ADC_BIAS_VOLTAGEDROP;
}

