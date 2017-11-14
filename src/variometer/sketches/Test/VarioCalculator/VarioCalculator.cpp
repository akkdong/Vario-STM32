// VarioCalculator.cpp
//

#include "VarioCalculator.h"
#include <HardwareTimer.h>
#include <GlobalConfig.h>

#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>

//
//
//


// accelerometer parameters
#define MPU6050_G_TO_MS 			(9.80665)

#define MPU6050_GYRO_FSR 			(2000)
#define MPU6050_ACCEL_FSR 			(4)
#define MPU6050_FIFO_RATE 			(100)

//  2G ~= 16384 2^14
//  4G ~=  8192 2^13
//  8G ~=  4096 2^12
// 16G ~=  2048 2^11
#define MPU6050_ACCEL_SCALE 		(8192.0)
//  250 degree/s ~= 131 
//  500 degree/s ~= 65.5
// 1000 degree/s ~= 32.8
// 2000 degree/s ~= 16.4
#define MPU6050_GYRO_SCALE			(16.4)

// 2^30
#define MPU6050_QUAT_SCALE 			(1073741824.0)


//
//
//

// the pressure normalized sea level pressure
#define MS5611_BASE_SEA_PRESSURE 	(1013.25)

#define MS5611_ADDRESS 				(0x77)
#define MS5611_CMD_RESET 			(0x1E)
#define MS5611_CMD_READ_PROM 		(0xA2)
#define MS5611_CMD_CONV_D1 			(0x46)
#define MS5611_CMD_CONV_D2 			(0x58)
#define MS5611_CMD_ADC_READ 		(0x00)

#define MS5611_RESET_DELAY 			(3)
#define MS5611_CONV_DELAY 			(9)

#define MS5611_STEP_UNDEF			(0)
#define MS5611_STEP_READY			(1)
#define MS5611_STEP_READ_TEMP 		(2)
#define MS5611_STEP_READ_PRESSURE 	(3)

// the measure period need to be greater than 8.22 ms (Max 9.04 ms)
// the library use a 1024 prescale so the time unit is 1024/F_CPU 
// the INTERRUPT_COMPARE can't be greater than 255
// but a greater value give less code interrupts
// the final period is 1024/F_CPU * INTERRUPT_COMPARE
// in seconds 

#define MS5611_INTERRUPT_COMPARE	(636) // F_CPU : 72MHz, 636 ~= 9.04ms, 600 ~= 8.5ms, 590 ~= 8.39ms

#define MS5611_TIMER_PRESCALER		(1024)
#define MS5611_TIMER_CHANNEL		(1)

#define MS5611_INTERRUPT_START_DELAY (1000)


///////////////////////////////////////////////////////////////////////////////////////////////
// class VarioCalculator

VarioCalculator::VarioCalculator()
{
	baroState = MS5611_STEP_UNDEF;
	
	baroUpdated = false;
	varioUpdated = false;
}

int VarioCalculator::begin(float sigmaP, float sigmaA, int calibrateGyro)
{
	if (initBaro() && initAccel(calibrateGyro))
	{
		//
		convertD1();
		startTimer();
		
		// delay to stabilize the device
		//delay(MS5611_INTERRUPT_START_DELAY);		

		//
		float va, ba;
		int count = 0;
		
		while (count < 10)
		{
			updateVerticalAccel();
			
			if (baroUpdated && accumulateCount > 0)
			{
				//
				va = accumulateAccel / accumulateCount;
				ba = getAltitude(compensatedPressure);
				//Serial.print("va = "); Serial.println(va);
				//Serial.print("ba = "); Serial.println(ba);
				
				//
				accumulateAccel = 0.0;
				accumulateCount = 0;
				
				baroUpdated = false;
				
				//
				count += 1;
			}
		}
		
		//
		initKalman(compensatedPressure, va, sigmaP, sigmaA, millis());
		//Serial.println("initKalman");
		
		return 1;
	}
	
	return 0;
}

void VarioCalculator::end()
{
	//
	stopTimer();
	
	//
	baroState = MS5611_STEP_UNDEF;
	
	baroUpdated = false;
	varioUpdated = false;
}

int VarioCalculator::available()
{
	return varioUpdated;
}

void VarioCalculator::flush()
{
	varioUpdated = false;
}

int VarioCalculator::update()
{
	updateVerticalAccel();
	
	if (baroUpdated && accumulateCount > 0)
	{
		//
		updateKalman(compensatedPressure, accumulateAccel / accumulateCount, millis());

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
	if (baroState == MS5611_STEP_UNDEF)
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
	
		return 1;
	}
	
	return 0;
}

void VarioCalculator::resetBaro()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_RESET, 0, NULL, false);
}

uint16_t VarioCalculator::getPROMValue(int address)
{
	uint8_t data[2];
	
	I2CDevice::readBytes(MS5611_ADDRESS, MS5611_CMD_READ_PROM+(address*2), 2, data, I2CDevice::readTimeout, false);

	return ((uint16_t)data[0] << 8) + (uint16_t)data[1];
}

uint32_t VarioCalculator::getDigitalValue()
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

float VarioCalculator::getAltitude(float pressure)
{
	float alti;
	
	alti = pow((pressure/(MS5611_BASE_SEA_PRESSURE)), 0.1902949572); //0.1902949572 = 1/5.255
	alti = (1 - alti)*(288.15/0.0065);
	
	return alti;
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
		convertD1();
		
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
	
	//I2CDevice::lockDevice();
	d1 = d1i;
	d2 = d2i;
	//I2CDevice::unlockDevice();

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
	//Serial.print("p = "); Serial.print(compensatedPressure); Serial.print(", t = "); Serial.println(compensatedTemperature);

	//
	baroUpdated = true;
}

int VarioCalculator::initAccel(int calibrateGyro)
{
	// setting mpu
	mpu_init(NULL);
	mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL); 
	mpu_set_gyro_fsr(MPU6050_GYRO_FSR);
	mpu_set_accel_fsr(MPU6050_ACCEL_FSR);
	mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL);

	// setting dmp
	dmp_load_motion_driver_firmware();
	dmp_set_fifo_rate(MPU6050_FIFO_RATE);
	mpu_set_dmp_state(1);
	
	if (calibrateGyro)
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|/*DMP_FEATURE_SEND_RAW_GYRO|*/DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_GYRO_CAL); 
	else
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|/*DMP_FEATURE_SEND_RAW_GYRO|*/DMP_FEATURE_SEND_RAW_ACCEL);

	//
	accumulateAccel = 0.0;
	accumulateCount= 0;
	
	return 1;
}

void VarioCalculator::updateVerticalAccel()
{
	short igyro[3], iaccel[3];
	long iquat[4];
	unsigned long timestamp;
	short sensors;
	unsigned char fifoCount;
	
	if (dmp_read_fifo(igyro,iaccel,iquat,&timestamp,&sensors,&fifoCount))
		return;
	
	if (! sensors)
		return;

	float ux, uy, uz;
	float rax, ray, raz;
	float uv[3];

	accelData[0] = ((float)iaccel[0])/MPU6050_ACCEL_SCALE + Config.accel_calData[0];
	accelData[1] = ((float)iaccel[1])/MPU6050_ACCEL_SCALE + Config.accel_calData[1];
	accelData[2] = ((float)iaccel[2])/MPU6050_ACCEL_SCALE + Config.accel_calData[2];

	quadData[0] = ((float)iquat[0])/MPU6050_QUAT_SCALE;
	quadData[1] = ((float)iquat[1])/MPU6050_QUAT_SCALE;
	quadData[2] = ((float)iquat[2])/MPU6050_QUAT_SCALE;
	quadData[3] = ((float)iquat[3])/MPU6050_QUAT_SCALE;		
	
	ux = 2*(quadData[1]*quadData[3]-quadData[0]*quadData[2]);
	uy = 2*(quadData[2]*quadData[3]+quadData[0]*quadData[1]);
	uz = 2*(quadData[0]*quadData[0]+quadData[3]*quadData[3])-1;

	// compute real acceleration (without gravity)
	rax = accelData[0] - ux;
	ray = accelData[1] - uy;
	raz = accelData[2] - uz;

	// compute vertical acceleration
	uv[0] = ux;
	uv[1] = uy;
	uv[2] = uz;
	
	float va = (ux*rax + uy*ray + uz*raz);
	accumulateAccel += va;
	accumulateCount += 1;
	//Serial.print("#"); Serial.print(accumulateCount); Serial.print(": "); Serial.println(va);
}

void VarioCalculator::initKalman(float startP, float startA, float sigmaP, float sigmaA, uint32_t timestamp)
{
	// init base values
	p = startP;
	v = 0;
	a = startA;
	t = timestamp;
	calibrationDrift = 0.0;

	// init variance
	varp = sigmaP * sigmaP;
	vara = sigmaA * sigmaA;

	// init covariance matrix
	p11 = 0;
	p12 = 0;
	p21 = 0;
	p22 = 0;
	
	//
	varioUpdated = false;
}

void VarioCalculator::updateKalman(float measureP, float measureA, uint32_t timestamp)
{
	// delta time
	unsigned long deltaTime = timestamp - t;
	float dt = ((float)deltaTime)/1000.0;
	t = timestamp;

	//
	// prediction
	//

	// values
	a = measureA;  // we use the last acceleration value for prediction 
	float dtPower = dt * dt; //dt^2
	p += dt*v + dtPower*a/2;
	v += dt*a;
	//a = measureA; // uncomment to use the previous acceleration value 

	// covariance
	float inc;

	dtPower *= dt;  // now dt^3
	inc = dt*p22 + dtPower*vara / 2;
	dtPower *= dt; // now dt^4
	p11 += dt*(p12 + p21 + inc) - (dtPower*vara/4);
	p21 += inc;
	p12 += inc;
	p22 += dt*dt*vara;

	//
	// gaussian product
	//

	// kalman gain
	float s, k11, k12, y;

	s = p11 + varp;
	k11 = p11/s;
	k12 = p12/s;
	y = measureP - p;

	// update
	p += k11 * y;
	v += k12 * y;
	
	p22 -= k12 * p21;
	p12 -= k12 * p11;
	p21 -= k11 * p21;
	p11 -= k11 * p11;
	
	//
	varioUpdated = true;
}
