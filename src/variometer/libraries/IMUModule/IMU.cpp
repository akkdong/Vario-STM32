// IMU.cpp
//

#include "IMU.h"
#include <HardwareTimer.h>
#include <GlobalConfig.h>

#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>


//
// MS5611
//

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
// F_CPU : 72MHz
#define MS5611_INTERRUPT_COMPARE	(600) // 600 ~= 8.5ms, 590 ~= 8.39ms

#define MS5611_TIMER_PRESCALER		(1024)
#define MS5611_TIMER_CHANNEL		(1)

#define MS5611_INTERRUPT_START_DELAY (1000)


//
// MPU6050
//

/* eeprom calibration adresses */
#define MPU6050_EPROM_TAG 			(9806)
#define MPU6050_EPROM_ADDR 			(0x00)

/* accelerometer parameters */
#define MPU6050_G_TO_MS 			(9.80665)

#define MPU6050_GYRO_FSR 			(2000)
#define MPU6050_ACCEL_FSR 			(4)
#define MPU6050_FIFO_RATE 			(100)

/* 4G ~= 2^15 */
#define MPU6050_ACCEL_SCALE 		(8192.0)
/* 2000 degree/s ~= 16.4 */
#define MPU6050_GYRO_SCALE			(16.4)

/* 2^30 */
#define MPU6050_QUAT_SCALE 			(1073741824.0)


enum IMUState
{
	IMU_INIT,
	IMU_STABILIZE,
	IMU_RUNNING,
	IMU_STOPPED
};

////////////////////////////////////////////////////////////////////////////////////
// class IMU

IMU::IMU()
{
}

void IMU::initMS6511()
{
	// reset
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_RESET, 0, NULL, false);
	delay(MS5611_RESET_DELAY);
	
	// read PROM values
	c1 = readPROMValue(0);
	c2 = readPROMValue(1);
	c3 = readPROMValue(2);
	c4 = readPROMValue(3);
	c5 = readPROMValue(4);
	c6 = readPROMValue(5);
}

void IMU::initMPU6050(boolean calibrateGyro)
{
	// setting imu
	//mpu_select_device(0);
	//mpu_init_structures();
	mpu_init(NULL);
	mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL); 
	mpu_set_gyro_fsr(MPU6050_GYRO_FSR);
	mpu_set_accel_fsr(MPU6050_ACCEL_FSR);
	mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL);

	// setting dmp
	//dmp_select_device(0);
	//dmp_init_structures();
	dmp_load_motion_driver_firmware();
	dmp_set_fifo_rate(MPU6050_FIFO_RATE);
	mpu_set_dmp_state(1);
	if (calibrateGyro )
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_GYRO|DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_GYRO_CAL); 
	else
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_GYRO|DMP_FEATURE_SEND_RAW_ACCEL);
	
	accelData[0] = accelData[1] = accelData[2] = 0.0;
	gyroData[0] = gyroData[1] = gyroData[2] = 0.0;
}

int	IMU::begin(IVerticalVelocity * vv, boolean calibrateGyro)
{
	// save VertialVelocity interface
	iVelocity = vv;
	
	//
	initMS6511();
	initMPU6050(calibrateGyro);
	
	//
	measureStep = MS5611_STEP_READ_TEMP;
	newData = false;
	
	// convert D1
	convertD1();
	
	// start interrupts
	startTimer();
	
	// delay to stabilize the device
	imuState = IMU_INIT;
	//Serial.println("IMU_INIT");
	delay(MS5611_INTERRUPT_START_DELAY);
	//Serial.println("IMU_RUNNING");
	imuState = IMU_RUNNING;

	return 1;
}

void IMU::end()
{
	stopTimer();
	
	imuState = IMU_STOPPED;
}

boolean IMU::updateAcceleration()
{
	short iaccel[3], igyro[3];
	long iquat[4];
	unsigned long timestamp;
	short sensors;
	unsigned char fifoCount;
	boolean newData = false;

	/* check if we have new data from imu */
	while( dmp_read_fifo(igyro,iaccel,iquat,&timestamp,&sensors,&fifoCount) == 0 )
	{
		newData = true;
	}
	
	if (! newData)
		return false;

	/***************************/
	/* normalize and calibrate */
	/***************************/
	//float accel[3]; 
	//float gyro[3];
	float quat[4]; 

	accelData[0] = ((float)iaccel[0])/MPU6050_ACCEL_SCALE + Config.accel_calData[0];
	accelData[1] = ((float)iaccel[1])/MPU6050_ACCEL_SCALE + Config.accel_calData[1];
	accelData[2] = ((float)iaccel[2])/MPU6050_ACCEL_SCALE + Config.accel_calData[2];
	
	gyroData[0] = ((float)igyro[0])/MPU6050_GYRO_SCALE;
	gyroData[1] = ((float)igyro[1])/MPU6050_GYRO_SCALE;
	gyroData[2] = ((float)igyro[2])/MPU6050_GYRO_SCALE;

	quat[0] = ((float)iquat[0])/MPU6050_QUAT_SCALE;
	quat[1] = ((float)iquat[1])/MPU6050_QUAT_SCALE;
	quat[2] = ((float)iquat[2])/MPU6050_QUAT_SCALE;
	quat[3] = ((float)iquat[3])/MPU6050_QUAT_SCALE;

	/******************************/
	/* real and vert acceleration */
	/******************************/

	/* compute upper direction from quaternions */
	float ux, uy, uz;
	ux = 2*(quat[1]*quat[3]-quat[0]*quat[2]);
	uy = 2*(quat[2]*quat[3]+quat[0]*quat[1]);
	uz = 2*(quat[0]*quat[0]+quat[3]*quat[3])-1;

	/* compute real acceleration (without gravity) */
	float rax, ray, raz;
	rax = accelData[0] - ux;
	ray = accelData[1] - uy;
	raz = accelData[2] - uz;

	/* compute vertical acceleration */
	vertAccel = (ux*rax + uy*ray + uz*raz);

	return true;		
}

boolean	IMU::readRawAccelData(float * accel, float * uv, float * va)
{
	short iaccel[3], igyro[3];
	long iquat[4];
	unsigned long timestamp;
	short sensors;
	unsigned char fifoCount;
	boolean newData = false;

	/* check if we have new data from imu */
	while( dmp_read_fifo(igyro,iaccel,iquat,&timestamp,&sensors,&fifoCount) == 0 )
	{
		newData = true;
	}
	
	if (! newData);
		return false;


	/*************/
	/* normalize */
	/*************/ 
	float quat[4]; 

	accel[0] = ((float)iaccel[0])/MPU6050_ACCEL_SCALE;
	accel[1] = ((float)iaccel[1])/MPU6050_ACCEL_SCALE;
	accel[2] = ((float)iaccel[2])/MPU6050_ACCEL_SCALE;

	quat[0] = ((float)iquat[0])/MPU6050_QUAT_SCALE;
	quat[1] = ((float)iquat[1])/MPU6050_QUAT_SCALE;
	quat[2] = ((float)iquat[2])/MPU6050_QUAT_SCALE;
	quat[3] = ((float)iquat[3])/MPU6050_QUAT_SCALE;


	/******************************/
	/* real and vert acceleration */
	/******************************/

	/* compute upper direction from quaternions */
	float ux, uy, uz;
	ux = 2*(quat[1]*quat[3]-quat[0]*quat[2]);
	uy = 2*(quat[2]*quat[3]+quat[0]*quat[1]);
	uz = 2*(quat[0]*quat[0]+quat[3]*quat[3])-1;

	/* compute real acceleration (without gravity) */
	float rax, ray, raz;
	rax = accel[0] - ux;
	ray = accel[1] - uy;
	raz = accel[2] - uz;


	/* compute vertical acceleration */
	uv[0] = ux;
	uv[1] = uy;
	uv[2] = uz;
	
	*va = (ux*rax + uy*ray + uz*raz);

	return true;	
}

float IMU::getAcceleration()
{
	return vertAccel * MPU6050_G_TO_MS;
}

float IMU::getPressure()
{
	return compensatedPressure;
}

float IMU::getTemperature()
{
	return compensatedTemperature;
}

float IMU::getAltitude()
{
	float alti;
	
	alti = pow((compensatedPressure/(MS5611_BASE_SEA_PRESSURE)), 0.1902949572); //0.1902949572 = 1/5.255
	alti = (1-alti)*(288.15/0.0065);
	
	return alti;	
}

float * IMU::getRawAccelData()
{
	return accelData;
}

float * IMU::getRawGyroData()
{
	return gyroData;
}

IMU & IMU::getInstance()
{
	static IMU imu;
	
	return imu;
}

void IMU::timerProc()
{
	IMU & imu = IMU::getInstance();
	
	imu.readStep();
	imu.restartTimer();
	
	if (imu.newData)
	{
		//Timer2.pause();
		imu.updatePressure();
		imu.updateAcceleration(); // read acceleration every (8.5 + 8.5) ms
		//Serial.print(imu.getPressure());
		//Serial.print(",");
		//Serial.println(imu.getAcceleration());
		//Timer2.resume();

		#if 1
		if (imu.imuState == IMU_RUNNING && imu.iVelocity)
		{
			if (! imu.iVelocity->ready())
			{
				imu.iVelocity->begin(imu.getPressure(),
									imu.getAcceleration(),
									Config.kalman_sigmaP,
									Config.kalman_sigmaA,
									millis());
			}
			else
			{
				imu.iVelocity->update(imu.getPressure(),
									imu.getAcceleration(),
									millis());
			}
		}
		#endif 
	}	
}

uint16_t IMU::readPROMValue(int address)
{
	uint8_t data[2];
	
	I2CDevice::readBytes(MS5611_ADDRESS, MS5611_CMD_READ_PROM+(address*2), 2, data, I2CDevice::readTimeout, false);

	return ((uint16_t)data[0] << 8) + (uint16_t)data[1];	
}

uint32_t IMU::readDigitalValue()
{
	uint32_t value;
	uint8_t data[3];
	
	I2CDevice::readBytes(MS5611_ADDRESS, MS5611_CMD_ADC_READ, 3, data, I2CDevice::readTimeout, false);
	
	value = (uint32_t)data[0];
	value <<= 8;
	value += data[1];
	value <<= 8;
	value += data[2];

	return value;
}

void IMU::convertD1()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_CONV_D1, 0, NULL, false);
}

void IMU::convertD2()
{
	I2CDevice::writeBytes(MS5611_ADDRESS, MS5611_CMD_CONV_D2, 0, NULL, false);
}

void IMU::readTempStep()
{
	// read raw temp
	d1i = readDigitalValue();

	// convert d2
	convertD2();		
}

void IMU::readPressureStep()
{
	// read raw pressure
	d2i = readDigitalValue();

	// convert d1
	convertD1();

	// now we have new data
	newData = true;	
}

void IMU::updatePressure()
{
	// lock the mutex to get the values
	uint32_t d1 = d1i;
	uint32_t d2 = d2i;
	
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
	newData = false;
}

// altimeter read step
// MUST BE DONE AT STABLE FREQUENCY
// here using interrupts
void IMU::readStep()
{
	if( measureStep == MS5611_STEP_READ_TEMP )
	{
		readTempStep();
		measureStep = MS5611_STEP_READ_PRESSURE;
	}
	else
	{
		readPressureStep();
		measureStep = MS5611_STEP_READ_TEMP;
	}	
}

void IMU::startTimer()
{
	Timer2.pause();
	Timer2.setMode(MS5611_TIMER_CHANNEL, TIMER_OUTPUT_COMPARE);
	Timer2.setPrescaleFactor(MS5611_TIMER_PRESCALER);
	Timer2.setCompare(MS5611_TIMER_CHANNEL, MS5611_INTERRUPT_COMPARE);
	Timer2.setCount(0);
	Timer2.attachInterrupt(MS5611_TIMER_CHANNEL, timerProc);
	Timer2.resume();	
}

void IMU::stopTimer()
{
	Timer2.pause();
}

void IMU::restartTimer()
{
	Timer2.refresh();
}
