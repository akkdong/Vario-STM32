// BatteryVoltage.h
//

#ifndef __BATTERYVOLTAGE_H__
#define __BATTERYVOLTAGE_H__

#include <Arduino.h>
#include <STM32ADC.h>

#define SCALE_FACTOR			(1000.0 / (270.0 + 1000.0))	// voltage divisor : R1(270K), R2(1M)
#define	LPF_FACTOR				(0.2)

#define ADC_CH					ADC1
#define ADC_PIN					PA0
#define ADC_VREF				(3.3)
#define ADC_TO_VOLTAGE(x)		((x) *  ADC_VREF / 1024.0 / SCALE_FACTOR)

#define ADC_MEASURE_COUNT		(10)
#defien ADC_MEASURE_INTERVAL	(2000)


///////////////////////////////////////////////////////////////////////////
// class BatteryVoltage

class BatteryVoltage
{
protected:
	BatteryVoltage();
	
public:
	static BatteryVoltage &		getInst();
	
	void						begin();
	void						update();
	
	double						getVoltage();
	
	static void					finish();
	
private:
	volatile uint8_t			measState;	// READY, SAMPLING
	uint16_t					measData[1];
	
	double						measVoltage;
	STM32ADC					adc;
	
	uint32_t					lastTick;
}

#endif // __BATTERYVOLTAGE_H__
