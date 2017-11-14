// VarioCalculator.cpp
//

#include "VarioCalculator.h"
#include <HardwareTimer.h>

/* the pressure normalized sea level pressure */  
#define MS5611_BASE_SEA_PRESSURE 	(1013.25)

#define MS5611_ADDRESS 				(0x77)
#define MS5611_CMD_RESET 			(0x1E)
#define MS5611_CMD_READ_PROM 		(0xA2)
#define MS5611_CMD_CONV_D1 			(0x46)
#define MS5611_CMD_CONV_D2 			(0x58)
#define MS5611_CMD_ADC_READ 		(0x00)

#define MS5611_RESET_DELAY 			(3)
#define MS5611_CONV_DELAY 			(9)

#define MS5611_STEP_RESET			(0)
#define MS5611_STEP_READY			(1)
#define MS5611_STEP_READ_TEMP 		(2)
#define MS5611_STEP_READ_PRESSURE 	(3)

/* the measure period need to be greater than 8.22 ms (Max 9.04 ms) */
/* the library use a 1024 prescale so the time unit is 1024/F_CPU */
/* the INTERRUPT_COMPARE can't be greater than 255 */
/* but a greater value give less code interrupts */
/* the final period is 1024/F_CPU * INTERRUPT_COMPARE */
/* in seconds */

// F_CPU : 72MHz
#define MS5611_INTERRUPT_COMPARE	(636) // 636 ~= 9.04ms, 600 ~= 8.5ms, 590 ~= 8.39ms

#define MS5611_TIMER_PRESCALER		(1024)
#define MS5611_TIMER_CHANNEL		(1)

#define MS5611_INTERRUPT_START_DELAY (1000)


///////////////////////////////////////////////////////////////////////////////////////////////
// class VarioCalculator

VarioCalculator::VarioCalculator()
{
	baroState = MS5611_STEP_RESET;
}

int VarioCalculator::begin()
{
	if (initBaro() && initAccel())
	{
		//
		convertD1();
		startTimer();

		//
		int count = 0;
		while (count < 10)
		{
			if (update())
				count += 1;
		}
		
		//initKalman(...);
		
		return 1;
	}
	
	return 0;
}

void VarioCalculator::end()
{
}

int VarioCalculator::available()
{
	return 0;
}

void VarioCalculator::flush()
{
}

int VarioCalculator::update()
{
	updateVerticalAccel();
	
	if (baroUpdated && accumulateCount > 0)
	{
		// blah blah~~
		//

		//
		accumulateCount = 0;
		accumulateAccel = 0;
		
		baroUpdated = false;
		
		return 1;
	}
	
	return 0;
}

VarioCalculator & VarioCalculator::getInstance()
{
	static VarioCalculator vario;
	
	return vario;
}

void VarioCalculator::unlockI2C_()
{
	getInstance().unlockI2C();
}

void VarioCalculator::unlockI2C()
{
	if (interruptWait)
	{
		convertNext();

		interruptWait = false;
	}
}
	
void VarioCalculator::timerProc_()
{
	getInstance().timerProc();
}

void VarioCalculator::timerProc()
{
	if (I2CDevice::locked)
	{
		interruptWait = true;
	}
	else
	{
		convertNext();
	}
}

void VarioCalculator::startTimer()
{
	Timer2.pause();
	Timer2.setMode(MS5611_TIMER_CHANNEL, TIMER_OUTPUT_COMPARE);
	Timer2.setPrescaleFactor(MS5611_TIMER_PRESCALER);
	Timer2.setCompare(MS5611_TIMER_CHANNEL, MS5611_INTERRUPT_COMPARE);
	Timer2.attachInterrupt(MS5611_TIMER_CHANNEL, timerProc_);
	Timer2.refresh();
	Timer2.resume();	
}

void VarioCalculator::resetTimer()
{
	Timer2.refresh();
}

void VarioCalculator::stopTimer()
{
	Timer2.pause();
}

int VarioCalculator::initBaro()
{
	if (baroState == MS5611_STEP_RESET)
	{
		//
		resetBaro();
		delay(MS5611_RESET_DELAY);
		
		//
		c1 = getPROMValue(0);
		c2 = getPROMValue(1);
		c3 = getPROMValue(2);
		c4 = getPROMValue(3);
		c5 = getPROMValue(4);
		c6 = getPROMValue(5);
		
		//
		baroState = MS5611_STEP_READY;
		baroUpdated = false;
		interruptWait = false;
	
		return 0;
	}
	
	return 1;
}

void VarioCalculator::resetBaro()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_RESET, 0, NULL, false);
}

void VarioCalculator::convertNext()
{
	if (baroState == MS5611_STEP_READ_TEMP)
	{
		d1i = getDigitalValue();
		convertD2();
	}
	else
	{
		d2i = getDigitalValue();
		convertD2();
		
		updateBaro();
	}
	
	resetTimer();
}

void VarioCalculator::convertD1()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_CONV_D1, 0, NULL, false);
	baroState = MS5611_STEP_READ_TEMP;
}

void VarioCalculator::convertD2()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_CONV_D2, 0, NULL, false);
	baroState = MS5611_STEP_READ_PRESSURE;
}

void VarioCalculator::updateBaro()
{
	// lock the mutex to get the values
	uint32_t d1;
	uint32_t d2;
	
	I2CDevice::lockDevice();
	d1 = d1i;
	d2 = d2i;
	I2CDevice::unlockDevice();

	// compute temperature
	int32_t dt, temp;

	int32_t c5s = c5;
	c5s <<= 8;
	dt = d2 - c5s;

	int32_t c6s = c6;
	c6s *= dt;
	c6s >>= 23;

	temp = 2000 + c6s;

	// compute compensation
	int64_t off, sens;

	// offset
	int64_t c2d = c2;
	c2d <<=  16;

	int64_t c4d = c4;
	c4d *= dt;
	c4d >>= 7;

	off = c2d + c4d;

	// sens
	int64_t c1d = c1;
	c1d <<= 15;

	int64_t c3d = c3;
	c3d *= dt;
	c3d >>= 8;

	sens = c1d + c3d;

	// second order compensation
	int64_t t2, off2, sens2;

	if( temp < 2000 )
	{
		t2 = dt;
		t2 *= t2;
		t2 >>= 31;

		off2 = temp-2000;
		off2 *= off2;
		off2 *= 5;
		sens2 = off2;
		off2 >>= 1;
		sens2 >>= 2;

		if( temp < -1500 )
		{
			int64_t dtemp = temp + 1500;
			dtemp *= dtemp;
			off2 += 7*dtemp;
			dtemp *= 11;
			dtemp >>= 1;
			sens2 += dtemp;
		}
		
		temp = temp - t2;
		off = off - off2;
		sens = sens - sens2;
	}

	// compute pressure
	int64_t p;

	p = d1 * sens;
	p >>= 21;
	p -= off;
	//p >>= 15 !!! done with doubles, see below

	// save result
	compensatedTemperature = (float)temp/100;
	compensatedPressure = ((float)p / (float)32768)/(float)100;  //32768 = 2^15	
	
	//
	baroUpdated = true;
}

int VarioCalculator::initAccel()
{
	return 0;
}

void VarioCalculator::updateVerticalAccel()
{
}

int VarioCalculator::initKalman()
{
	return 0;
}

int VarioCalculator::updateKalman()
{
	return 0;
}
