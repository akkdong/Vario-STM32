// MPU60X0.cpp
//

#include "MPU60X0.h"
#include "MahonyAHRS.h"

#include <GlobalConfig.h>
#include <inv_mpu.h>


///////////////////////////////////////////////////////////////////////////////////////////////
//

MPU60X0 mpu60x0_;


///////////////////////////////////////////////////////////////////////////////////////////////
// class MPU60X0

MPU60X0::MPU60X0()
{
}

void MPU60X0::begin()
{
	// setting imu
	mpu_init(NULL);
	mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL); 
	mpu_set_gyro_fsr(MPU60X0_GYRO_FSR);
	mpu_set_accel_fsr(MPU60X0_ACCEL_FSR);
	mpu_set_sample_rate(MPU60X0_FIFO_RATE);
	mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL);	

	// init calibration settings
	//readCalibration();

	// init data variables 
	accelUpdated = false;
	
	memset(gyroData, 0, sizeof(gyroData));
	memset(accelData, 0, sizeof(accelData));
}

void MPU60X0::end()
{
	// nop
}
	
int MPU60X0::available()
{
	return accelUpdated;
}

void MPU60X0::flush()
{
	accelUpdated = false;
	
	accumulateAccel = 0.0;
	accumulateCount = 0;
}

void MPU60X0::update(int calibrateAccel)
{
	short iaccel[3], igyro[3];
	unsigned long timestamp;
	unsigned char sensors;
	unsigned char fifoCount;

	/// check if we have new data from imu
	if(mpu_read_fifo(igyro,iaccel,&timestamp,&sensors,&fifoCount))
		return;
	
	//
	if (! sensors)
		return;

	// if new data compute vertical acceleration 
	gyroData[0] = ((float)igyro[0])/MPU60X0_GYRO_SCALE;
	gyroData[1] = ((float)igyro[1])/MPU60X0_GYRO_SCALE;
	gyroData[2] = ((float)igyro[2])/MPU60X0_GYRO_SCALE;

	accelData[0] = ((float)iaccel[0])/MPU60X0_ACCEL_SCALE;
	accelData[1] = ((float)iaccel[1])/MPU60X0_ACCEL_SCALE;
	accelData[2] = ((float)iaccel[2])/MPU60X0_ACCEL_SCALE;
	
	if (calibrateAccel)
	{
		gyroData[0] += Config.gyro_calData[0];
		gyroData[1] += Config.gyro_calData[1];
		gyroData[2] += Config.gyro_calData[2];		
		
		accelData[0] += Config.accel_calData[0];
		accelData[1] += Config.accel_calData[1];
		accelData[2] += Config.accel_calData[2];		
	}
	
	// CJMCU-117 board is placed upside down in the case (when speaker is pointing up). We arbitrarily decide that 
	// the CJMCU-117 board silkscreen +X points "forward" or "north"  (in our case, the side with the power switch), 
	// silkscreen +Y points "right" or "east", and silkscreen +Z points down. This is the North-East-Down (NED) 
	// right-handed coordinate frame used in our AHRS algorithm implementation.
	// The required mapping from sensor samples to NED frame for our specific board orientation is : 
	// gxned = gx, gyned = gy, gzned = gz (clockwise rotations about the axis must result in +ve readings on the axis)
	// axned = -ax, ayned = -ay, azned = -az (when the axis points down, axis reading must be +ve)
	// The AHRS algorithm expects rotation rates in radians/second
	imu_MahonyAHRSupdateIMU(0.01, 
						gyroData[0] * DEG_TO_RAD, 
						gyroData[1] * DEG_TO_RAD, 
						gyroData[2] * DEG_TO_RAD, 
						-accelData[0], 
						-accelData[1], 
						-accelData[2]);
	
	float gravityCompensatedAccel = imu_GravityCompensatedAccel(-accelData[0], -accelData[1], -accelData[2], q0, q1, q2, q3);
	accumulateAccel += gravityCompensatedAccel; // one earth-z acceleration value computed every 5mS, accumulate
	accumulateCount += 1;

	//
	accelUpdated = true;
}

int MPU60X0::read(float * va)
{
	if (accelUpdated)
	{
		//
		//*va = vertAccel * MPU6050_G_TO_MS;
		*va = (accumulateAccel / accumulateCount) * MPU60X0_G_TO_MS;
		
		//
		flush();
		
		return 1;
	}
	
	return 0;
}

int MPU60X0::read(float * accel, float * gyro)
{
	if (accelUpdated)
	{
		//
		if (gyro)
			memcpy(gyro, gyroData, sizeof(gyroData));
		if (accel)
			memcpy(accel, accelData, sizeof(accelData));	
		
		//
		flush();
		
		return 1;
	}
	
	return 0;
}

void MPU60X0::calibateGyro()
{	
	#define GYRO_MAX_EXPECTED_OFFSET		16
	#define GYRO_NUM_CALIB_SAMPLES			100
	#define ABS(x)                	 		((x) < 0 ? -(x) : (x))
	
	float gyro[3], sum[3] = {0, 0, 0};
	int count = 0;
	
	do
	{
		mpu60x0_.update(false);

		if (mpu60x0_.available())
		{
			mpu60x0_.read(0, gyro);
			
			if ((ABS(gyro[0]) > GYRO_MAX_EXPECTED_OFFSET) || 
				(ABS(gyro[1]) > GYRO_MAX_EXPECTED_OFFSET) || 
				(ABS(gyro[2]) > GYRO_MAX_EXPECTED_OFFSET))
			{
				count = 0;
				break;
			}
			
			sum[0] += gyro[0];
			sum[1] += gyro[1];
			sum[2] += gyro[2];
			
			count += 1;
		}
		
	} while (count < GYRO_NUM_CALIB_SAMPLES);
	
	//
	if (count)
	{
		Config.gyro_calData[0] = -(sum[0] / count);
		Config.gyro_calData[1] = -(sum[1] / count);
		Config.gyro_calData[2] = -(sum[2] / count);
	}
}