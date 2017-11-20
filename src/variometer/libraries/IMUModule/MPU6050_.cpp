// MPU6050_.cpp
//

#include "MPU6050_.h"

#include <GlobalConfig.h>
#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>


///////////////////////////////////////////////////////////////////////////////////////////////
//

MPU6050_ mpu6050_;


///////////////////////////////////////////////////////////////////////////////////////////////
// class MPU6050_

MPU6050_::MPU6050_()
{
}

void MPU6050_::begin(int calibrateGyro)
{
	init(calibrateGyro);
}

void MPU6050_::beginCalibration()
{
	init(false);
}

void MPU6050_::end()
{
	// nop
}
	
int MPU6050_::available()
{
	return accelUpdated;
}

void MPU6050_::flush()
{
	accelUpdated = false;
	
	accumulateAccel = 0.0;
	accumulateCount = 0;
}

void MPU6050_::update(int calibrateAccel)
{
	short iaccel[3], igyro[3];
	long iquat[4];
	unsigned long timestamp;
	short sensors;
	unsigned char fifoCount;

	/// check if we have new data from imu
	if(dmp_read_fifo(igyro,iaccel,iquat,&timestamp,&sensors,&fifoCount))
		return;
	
	//
	if (! sensors)
		return;

	// if new data compute vertical acceleration 
	gyroData[0] = ((float)igyro[0])/MPU6050_GYRO_SCALE;
	gyroData[1] = ((float)igyro[1])/MPU6050_GYRO_SCALE;
	gyroData[2] = ((float)igyro[2])/MPU6050_GYRO_SCALE;

	accelData[0] = ((float)iaccel[0])/MPU6050_ACCEL_SCALE;
	accelData[1] = ((float)iaccel[1])/MPU6050_ACCEL_SCALE;
	accelData[2] = ((float)iaccel[2])/MPU6050_ACCEL_SCALE;
	
	if (calibrateAccel)
	{
		accelData[0] += Config.accel_calData[0];
		accelData[1] += Config.accel_calData[1];
		accelData[2] += Config.accel_calData[2];
	}
	
	quadData[0] = ((float)iquat[0])/MPU6050_QUAT_SCALE;
	quadData[1] = ((float)iquat[1])/MPU6050_QUAT_SCALE;
	quadData[2] = ((float)iquat[2])/MPU6050_QUAT_SCALE;
	quadData[3] = ((float)iquat[3])/MPU6050_QUAT_SCALE;

	// compute upper direction from quaternions
	float ux, uy, uz;
	
	ux = 2*(quadData[1]*quadData[3]-quadData[0]*quadData[2]);
	uy = 2*(quadData[2]*quadData[3]+quadData[0]*quadData[1]);
	uz = 2*(quadData[0]*quadData[0]+quadData[3]*quadData[3])-1;

	// compute real acceleration (without gravity)
	float rax, ray, raz;
	
	rax = accelData[0] - ux;
	ray = accelData[1] - uy;
	raz = accelData[2] - uz;

	// compute vertical acceleration
	float va = (ux*rax + uy*ray + uz*raz);
	accumulateAccel += va;
	accumulateCount += 1;
	//Serial.print("va = "); Serial.println(va * MPU6050_G_TO_MS);

	//
	accelUpdated = true;
}

int MPU6050_::read(float * va)
{
	if (accelUpdated)
	{
		//
		//*va = vertAccel * MPU6050_G_TO_MS;
		*va = (accumulateAccel / accumulateCount) * MPU6050_G_TO_MS;
		
		//
		flush();
		
		return 1;
	}
	
	return 0;
}

int MPU6050_::read(float * accel, float * gyro)
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

void MPU6050_::init(int calibrateGyro)
{
	// setting imu
	mpu_init(NULL);
	mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL); 
	mpu_set_gyro_fsr(MPU6050_GYRO_FSR);
	mpu_set_accel_fsr(MPU6050_ACCEL_FSR);
	mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL);

	// setting dmp
	dmp_load_motion_driver_firmware();
	dmp_set_fifo_rate(MPU6050_FIFO_RATE);
	mpu_set_dmp_state(1);
	if( calibrateGyro ) {
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_GYRO|DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_GYRO_CAL); 
	} else {
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_GYRO|DMP_FEATURE_SEND_RAW_ACCEL);
	}

	// init calibration settings
	//readCalibration();

	// init data variables 
	accelUpdated = false;
	
	//vertAccel = 0.0;
	accumulateAccel = 0.0;
	accumulateCount = 0;
	
	memset(gyroData, 0, sizeof(gyroData));
	memset(accelData, 0, sizeof(accelData));
	memset(quadData, 0, sizeof(quadData));
}
