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

BatteryVoltage::BatteryVoltage() : adc(ADC_CH)
{
}
	
BatteryVoltage & BatteryVoltage::getInst()
{
	static BatteryVoltage & batVolt;
	
	return batVolt;
}

void BatteryVoltage::begin()
{
	uint8_t pin = ADC_PIN;
	
	//
	measVoltage = 0.0;	
	measState = 0;
	
	lastTick = millis();
	
	//
	adc.calibrate();
	
	adc.setSampleRate(ADC_SMPR_1_5);
	adc.setPins(&pin, 1);
	adc.setScanMode();
	adc.setDMA(&measData[0], 1, (DMA_MINC_MODE | DMA_TRNS_CMPLT), DMA1_CH1_Event);
	
	adc.startConversion();
	SET_STATE(ADC_SAMPLING);
}

void BatteryVoltage::update()
{
	if (! IS_SET(ADC_SAMPLING) && (millis() - lastTick) > ADC_MEASURE_INTERVAL)
	{
		adc.startConversion();
		SET_STATE(ADC_SAMPLING);
	}
}

double BatteryVoltage::getVoltage()
{
	return measVoltage;
}

void BatteryVoltage::finish()
{
	measVoltage = ADC_TO_VOLTAGE(measData[0]);
	UNSET_STATE(ADC_SAMPLING);
}

static void DMA1_CH1_Event()
{
	BatteryVoltage &batVolt = BatteryVoltage::getInst();	
	batVolt.finish();
}
