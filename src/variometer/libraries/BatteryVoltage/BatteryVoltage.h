// BatteryVoltage.h
//

#ifndef __BATTERYVOLTAGE_H__
#define __BATTERYVOLTAGE_H__

#include <Arduino.h>

#define SCALE_FACTOR			(1) // (1000.0 / (270.0 + 1000.0))	// voltage divisor : R1(270K), R2(1M)
#define	LPF_FACTOR				(0.2)

#define ADC_CH					ADC1
#define ADC_PIN					PA0
#define ADC_VREF				(3.3)
#define ADC_RESOLUTION			((1 << 12) - 1)
#define ADC_TO_VOLTAGE(x)		((x) *  ADC_VREF / ADC_RESOLUTION / SCALE_FACTOR)

#define ADC_MEASURE_COUNT		(10)
#define ADC_MEASURE_INTERVAL	(2000)


///////////////////////////////////////////////////////////////////////////
// class BatteryVoltage

class BatteryVoltage
{
public:
	void						begin(uint8_t pin);
	void						update();
	
	double						getVoltage();
	
private:
	double						measVoltage;
	
	uint8_t						adcPin;
	uint32_t					lastTick;
};

#endif // __BATTERYVOLTAGE_H__
