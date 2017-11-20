// MS5611.cpp
//

#include "MS5611.h"



///////////////////////////////////////////////////////////////////////////////////////////////
//

MS5611 ms5611_;


///////////////////////////////////////////////////////////////////////////////////////////////
// class MS5611

MS5611::MS5611() : Timer(MS5611_TIMER_ID), baroState(MS5611_STEP_READY)
{
}

void MS5611::begin()
{
	//
	resetBaro();
	delay(MS5611_RESET_DELAY*10);
	
	//
	c1 = getPROMValue(0);
	c2 = getPROMValue(1);
	c3 = getPROMValue(2);
	c4 = getPROMValue(3);
	c5 = getPROMValue(4);
	c6 = getPROMValue(5);
	//Serial.print("c1 = "); Serial.println(c1);
	//Serial.print("c2 = "); Serial.println(c2);
	//Serial.print("c3 = "); Serial.println(c3);
	//Serial.print("c4 = "); Serial.println(c4);
	//Serial.print("c5 = "); Serial.println(c5);
	//Serial.print("c6 = "); Serial.println(c6);
	
	//
	baroState = MS5611_STEP_READY;
	baroUpdated = false;
	interruptWait = false;
	
	// start measurement
	convertD1();
	// measure digital value on timer proc
	startTimer();
}

void MS5611::end()
{
	//
	stopTimer();
	
	//
	baroState = MS5611_STEP_READY;
	baroUpdated = false;
}

int MS5611::available()
{
	return baroUpdated;
}

int MS5611::read(float * p, float * t)
{
	if (baroUpdated)
	{
		//
		if (p)
			*p = compensatedPressure;
		
		if (t)
			*t = compensatedTemperature;
		
		//
		baroUpdated = false;
		
		return 1;
	}
	
	return 0;
}

float MS5611::getAltitude(float p, float seaLevel)
{
	float alti;
	
	alti = pow((p/seaLevel), 0.1902949572); //0.1902949572 = 1/5.255
	alti = (1-alti)*(288.15/0.0065);
	
	return alti;
}

void MS5611::unlockI2C()
{
	ms5611_.unlockI2CInternal();
}

void MS5611::timerProc()
{
	ms5611_.timerProcInternal();
}

void MS5611::unlockI2CInternal()
{
	if (interruptWait)
	{
		convertNext();

		interruptWait = false;
	}
}

void MS5611::timerProcInternal()
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

void MS5611::resetBaro()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_RESET, 0, NULL, false);
}

uint16_t MS5611::getPROMValue(int address)
{
	uint8_t data[2];
	
	I2CDevice::readBytes(MS5611_ADDRESS, MS5611_CMD_READ_PROM+(address*2), 2, data, I2CDevice::readTimeout, false);

	return ((uint16_t)data[0] << 8) + (uint16_t)data[1];
}

uint32_t MS5611::getDigitalValue()
{
	uint8_t data[3];
	
	I2CDevice::readBytes(MS5611_ADDRESS, MS5611_CMD_ADC_READ, 3, data, I2CDevice::readTimeout, false);
	
	uint32_t value = (uint32_t)data[0];
	value <<= 8;
	value += data[1];
	value <<= 8;
	value += data[2];
	
	return value;
}

void MS5611::convertNext()
{
	if (baroState == MS5611_STEP_READ_PRESSURE)
	{
		d1i = getDigitalValue();
		convertD2();
	}
	else
	{
		d2i = getDigitalValue();
		convertD1();
	}
	
	resetTimer();

	if (baroState == MS5611_STEP_READ_PRESSURE)
		updateBaro();
}

void MS5611::convertD1()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_CONV_D1, 0, NULL, false);
	baroState = MS5611_STEP_READ_PRESSURE;
}

void MS5611::convertD2()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_CONV_D2, 0, NULL, false);
	baroState = MS5611_STEP_READ_TEMP;
}

void MS5611::updateBaro()
{
	#if 0 
	// timerProc never be called in updateBaro --> so.... no needs mutex lock
	// lock the mutex to get the values
	uint32_t d1;
	uint32_t d2;
	
	I2CDevice::lockDevice();
	d1 = d1i;
	d2 = d2i;
	I2CDevice::unlockDevice();
	#endif

	// compute temperature
	int32_t dt, temp;

	int32_t c5s = c5;
	c5s <<= 8;
	dt = d2i - c5s;

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

	p = d1i * sens;
	p >>= 21;
	p -= off;
	//p >>= 15 !!! done with doubles, see below

	// save result
	compensatedTemperature = (float)temp/100;
	compensatedPressure = ((float)p / (float)32768)/(float)100;  //32768 = 2^15	
	//Serial.print("p = "); Serial.print(compensatedPressure); Serial.print(", t = "); Serial.println(compensatedTemperature);

	//
	baroUpdated = true;
}

void MS5611::startTimer()
{
	Timer.pause();
	Timer.setMode(MS5611_TIMER_CHANNEL, TIMER_OUTPUT_COMPARE);
	Timer.setPrescaleFactor(MS5611_TIMER_PRESCALER);
	Timer.setCompare(MS5611_TIMER_CHANNEL, MS5611_INTERRUPT_COMPARE);
	Timer.attachInterrupt(MS5611_TIMER_CHANNEL, timerProc);
	Timer.refresh();
	Timer.resume();	
}

void MS5611::resetTimer()
{
	Timer.refresh();
}

void MS5611::stopTimer()
{
	Timer.pause();
}
