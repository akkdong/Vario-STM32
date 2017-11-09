// SensorMS5611.cpp
//

#include "IMUSensor.h"
#ifdef ARDUINO_ARCH_STM32F1
#include <HardwareTimer.h>
#endif // ARDUINO_ARCH_STM32F1

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

#define MS5611_STEP_READ_TEMP 		(0)
#define MS5611_STEP_READ_PRESSURE 	(1)

/* the measure period need to be greater than 8.22 ms */
/* the library use a 1024 prescale so the time unit is 1024/F_CPU */
/* the INTERRUPT_COMPARE can't be greater than 255 */
/* but a greater value give less code interrupts */
/* the final period is 1024/F_CPU * INTERRUPT_COMPARE */
/* in seconds */
#if defined ARDUINO_ARCH_AVR
	#if F_CPU >= 16000000L
	//#define MS5611_INTERRUPT_COMPARE 	(130)
	#define MS5611_INTERRUPT_COMPARE 	(154)
	#else
	//#define MS5611_INTERRUPT_COMPARE 	(66)
	#define MS5611_INTERRUPT_COMPARE 	(78)
	#endif
#elif defined ARDUINO_ARCH_STM32F1
	// F_CPU : 72MHz
	#define MS5611_INTERRUPT_COMPARE	(590)
	
	#define MS5611_TIMER_PRESCALER		(1024)
	#define MS5611_TIMER_CHANNEL		(1)
#else // no supported architecture
	//#error This architecture is not supported!
#endif

#define MS5611_INTERRUPT_START_DELAY (1000)



///////////////////////////////////////////////////////////////////////////////////////////////
// class SensorMS5611

SensorMS5611::SensorMS5611()
{
	interruptWait = false;
	deviceReset = false;
}

SensorMS5611 & SensorMS5611::GetInstance()
{
	static SensorMS5611 ms5611;
	
	return ms5611;
}

void SensorMS5611::initSensor()
{
	// reset the device if needed
	if( ! deviceReset )
	{
		resetCommand();
		delay(MS5611_RESET_DELAY);

		c1 = getPROMValue(0);
		c2 = getPROMValue(1);
		c3 = getPROMValue(2);
		c4 = getPROMValue(3);
		c5 = getPROMValue(4);
		c6 = getPROMValue(5);

		#if 0
		Serial.print("c1 = "); Serial.println(c1);
		Serial.print("c2 = "); Serial.println(c2);
		Serial.print("c3 = "); Serial.println(c3);
		Serial.print("c4 = "); Serial.println(c4);
		Serial.print("c5 = "); Serial.println(c5);
		Serial.print("c6 = "); Serial.println(c6);
		#endif

		deviceReset = true;
	}

	// initialize interrupt variables
	measureStep = MS5611_STEP_READ_TEMP;
	newData = false;

	// convert D1 before starting interrupts
	convertD1();

	// start interrupts
	startTimer();

	// delay to stabilize the device
	delay(MS5611_INTERRUPT_START_DELAY);	
}

// check if we have new data
boolean SensorMS5611::dataReady()
{
	return newData;
}

void SensorMS5611::updateData()
{
	// lock the mutex to get the values
	uint32_t d1;
	uint32_t d2;
	
	I2CDevice::lockDevice();
	d1 = d1i;
	d2 = d2i;
	newData = false;
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
}

float SensorMS5611::getTemperature()
{
	return compensatedTemperature;
}

float SensorMS5611::getPressure()
{
	return compensatedPressure;
}

float SensorMS5611::getAltitude()
{
	float alti;
	
	alti = pow((compensatedPressure/(MS5611_BASE_SEA_PRESSURE)), 0.1902949572); //0.1902949572 = 1/5.255
	alti = (1-alti)*(288.15/0.0065);
	
	return alti;
}

void SensorMS5611::UnlockI2C()
{
	SensorMS5611 & THIS = SensorMS5611::GetInstance();
	
	// check if and interrupt was done between lock and release
	if( THIS.interruptWait )
	{
		// the interrupt can't read, do it for it
		THIS.readStep(); 
		THIS.interruptWait = false;
		
		// reset timer
		THIS.restartTimer();
	}
}

// the main interrupt function : read at stable frequency
#if defined ARDUINO_ARCH_AVR
#ifdef TIMER2_COMPA_vect
ISR(TIMER2_COMPA_vect) {
#else
ISR(TIMER3_COMPA_vect) {
#endif // TIMER2_COMPA_vect
	// call default timer proc
	SensorMS5611::TimerProc();
}
#endif // ARDUINO_ARCH_AVR

void SensorMS5611::TimerProc()
{
	SensorMS5611 & THIS = SensorMS5611::GetInstance();
	
	// if mutex locked, let the main loop do the job when release 
	if( I2CDevice::locked )
	{
		THIS.interruptWait = true;
	}
	else
	{
		// reenable interrupts for i2c
		interrupts();
	
		// read at stable frequency
		THIS.readStep();
	}
}

void SensorMS5611::resetCommand()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_RESET, 0, NULL, false);
}

uint16_t SensorMS5611::getPROMValue(int address)
{
	uint8_t data[2];
	
	I2CDevice::readBytes(MS5611_ADDRESS, MS5611_CMD_READ_PROM+(address*2), 2, data, I2CDevice::readTimeout, false);

	return ((uint16_t)data[0] << 8) + (uint16_t)data[1];
}

void SensorMS5611::getDigitalValue(volatile uint32_t& value)
{
	uint8_t data[3];
	
	I2CDevice::readBytes(MS5611_ADDRESS, MS5611_CMD_ADC_READ, 3, data, I2CDevice::readTimeout, false);
	
	value = (uint32_t)data[0];
	value <<= 8;
	value += data[1];
	value <<= 8;
	value += data[2];	
}

void SensorMS5611::convertD1()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_CONV_D1, 0, NULL, false);
}

void SensorMS5611::convertD2()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_CONV_D2, 0, NULL, false);
}

void SensorMS5611::readTempStep()
{
	// read raw temp
	getDigitalValue(d1i);

	// convert d2
	convertD2();	
}

void SensorMS5611::readPressureStep()
{
	// read raw pressure
	getDigitalValue(d2i);

	// convert d1
	convertD1();

	// now we have new data
	newData = true;
}

// altimeter read step
// MUST BE DONE AT STABLE FREQUENCY
// here using interrupts
void SensorMS5611::readStep()
{
	if( measureStep == MS5611_STEP_READ_TEMP ) {
		readTempStep();
		measureStep = MS5611_STEP_READ_PRESSURE;
	} else {
		readPressureStep();
		measureStep = MS5611_STEP_READ_TEMP;
	}
}

void SensorMS5611::startTimer()
{
	// disable all interrupts
	noInterrupts();

#if defined ARDUINO_ARCH_AVR
	#ifdef TIMER2_COMPA_vect
		TCCR2A = 0b00000010; //CTC MODE
		TCCR2B = 0b00000111; //1024 prescale
		TIMSK2 = 0b00000010; //enable CompA

		TCNT2  = 0;
		OCR2A = MS5611_INTERRUPT_COMPARE;
	#else
		TCCR3A = 0b00000000; //CTC MODE
		TCCR3B = 0b00001101; //1024 prescale
		TIMSK3 = 0b00000010; //enable CompA

		TCNT3  = 0;
		OCR3A = MS5611_INTERRUPT_COMPARE;
	#endif
#elif defined ARDUINO_ARCH_STM32F1
	Timer2.pause();
	Timer2.setMode(MS5611_TIMER_CHANNEL, TIMER_OUTPUT_COMPARE);
	Timer2.setPrescaleFactor(MS5611_TIMER_PRESCALER);
	Timer2.setOverflow(MS5611_INTERRUPT_COMPARE);
	Timer2.setCompare(MS5611_TIMER_CHANNEL, MS5611_INTERRUPT_COMPARE);
	Timer2.setCount(0);
	Timer2.attachInterrupt(MS5611_TIMER_CHANNEL, TimerProc);
	Timer2.resume();
#else
	//#error This architecture is not supported!
#endif

	interrupts();
}

void SensorMS5611::restartTimer()
{
#if defined ARDUINO_ARCH_AVR
	// reset timer-counter
	#ifdef TIMER2_COMPA_vect
		TCNT2 = 0;
	#else
		TCNT3 = 0;
	#endif
#elif defined ARDUINO_ARCH_STM32F1
	Timer2.setCount(0);
#else
	//#error This architecture is not supported!
#endif 
}
