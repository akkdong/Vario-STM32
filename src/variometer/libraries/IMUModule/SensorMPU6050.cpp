// SensorMPU6050.cpp
//

#include <GlobalConfig.h>
#include "IMUSensor.h"

#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>


/* eeprom calibration adresses */
#define VERTACCEL_EPROM_TAG 	(9806)
#define VERTACCEL_EPROM_ADDR 	(0x00)

/* accelerometer parameters */
#define VERTACCEL_G_TO_MS 		(9.80665)

#define VERTACCEL_GYRO_FSR 		(2000)
#define VERTACCEL_ACCEL_FSR 	(4)
#define VERTACCEL_FIFO_RATE 	(100)

/*  2G ~= 16384 2^14 */
/*  4G ~=  8192 2^13 */
/*  8G ~=  4096 2^12 */
/* 16G ~=  2048 2^11 */
#define VERTACCEL_ACCEL_SCALE 	(8192.0)
/*  250 degree/s ~= 131 */
/*  500 degree/s ~= 65.5 */
/* 1000 degree/s ~= 32.8 */
/* 2000 degree/s ~= 16.4 */
#define VERTACCEL_GYRO_SCALE	(16.4)


/* 2^30 */
#define VERTACCEL_QUAT_SCALE 	(1073741824.0)


///////////////////////////////////////////////////////////////////////////////////////////////
//

SensorMPU6050 mpu6050;


///////////////////////////////////////////////////////////////////////////////////////////////
// class SensorMPU6050

SensorMPU6050::SensorMPU6050()
{
	accelData[0] = accelData[1] = accelData[2] = 0.0;
	gyroData[0] = gyroData[1] = gyroData[2] = 0.0;
}

#if 0
SensorMPU6050 & SensorMPU6050::GetInstance()
{
	static SensorMPU6050 mpu6050;
	
	return mpu6050;
}
#endif

void SensorMPU6050::initSensor(boolean calibrateGyro)
{
	/* setting imu */
	//mpu_select_device(0);
	//mpu_init_structures();
	mpu_init(NULL);
	mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL); 
	mpu_set_gyro_fsr(VERTACCEL_GYRO_FSR);
	mpu_set_accel_fsr(VERTACCEL_ACCEL_FSR);
	mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL);

	/* setting dmp */
	//dmp_select_device(0);
	//dmp_init_structures();
	dmp_load_motion_driver_firmware();
	dmp_set_fifo_rate(VERTACCEL_FIFO_RATE);
	mpu_set_dmp_state(1);
	if( calibrateGyro ) {
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_GYRO|DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_GYRO_CAL); 
	} else {
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_GYRO|DMP_FEATURE_SEND_RAW_ACCEL);
	}

	/* init calibration settings */
	//readCalibration();

	/* init data variables */
	newData = false;	
}

boolean	SensorMPU6050::dataReady()
{
	short iaccel[3], igyro[3];
	long iquat[4];
	unsigned long timestamp;
	short sensors;
	unsigned char fifoCount;

	/* check if we have new data from imu */
	while( dmp_read_fifo(igyro,iaccel,iquat,&timestamp,&sensors,&fifoCount) == 0 ) {
		newData = true;
	}

	/* if new data compute vertical acceleration */
	if( newData ) {
		/***************************/
		/* normalize and calibrate */
		/***************************/
		//float accel[3]; 
		//float gyro[3];
		float quat[4]; 

		accelData[0] = ((float)iaccel[0])/VERTACCEL_ACCEL_SCALE + Config.accel_calData[0];
		accelData[1] = ((float)iaccel[1])/VERTACCEL_ACCEL_SCALE + Config.accel_calData[1];
		accelData[2] = ((float)iaccel[2])/VERTACCEL_ACCEL_SCALE + Config.accel_calData[2];
		
		gyroData[0] = ((float)igyro[0])/VERTACCEL_GYRO_SCALE;
		gyroData[1] = ((float)igyro[1])/VERTACCEL_GYRO_SCALE;
		gyroData[2] = ((float)igyro[2])/VERTACCEL_GYRO_SCALE;

		quat[0] = ((float)iquat[0])/VERTACCEL_QUAT_SCALE;
		quat[1] = ((float)iquat[1])/VERTACCEL_QUAT_SCALE;
		quat[2] = ((float)iquat[2])/VERTACCEL_QUAT_SCALE;
		quat[3] = ((float)iquat[3])/VERTACCEL_QUAT_SCALE;

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
	}

	return newData;	
}

boolean	SensorMPU6050::rawReady(float * accel, float * uv, float * va)
{
	short iaccel[3], igyro[3];
	long iquat[4];
	unsigned long timestamp;
	short sensors;
	unsigned char fifoCount;

	/* check if we have new data from imu */
	while( dmp_read_fifo(igyro,iaccel,iquat,&timestamp,&sensors,&fifoCount) == 0 ) {
		newData = true;
	}

	/* if new data compute vertical acceleration */
	if( newData ) {
		/*************/
		/* normalize */
		/*************/ 
		float quat[4]; 

		accel[0] = ((float)iaccel[0])/VERTACCEL_ACCEL_SCALE;
		accel[1] = ((float)iaccel[1])/VERTACCEL_ACCEL_SCALE;
		accel[2] = ((float)iaccel[2])/VERTACCEL_ACCEL_SCALE;

		quat[0] = ((float)iquat[0])/VERTACCEL_QUAT_SCALE;
		quat[1] = ((float)iquat[1])/VERTACCEL_QUAT_SCALE;
		quat[2] = ((float)iquat[2])/VERTACCEL_QUAT_SCALE;
		quat[3] = ((float)iquat[3])/VERTACCEL_QUAT_SCALE;


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
	}

	return newData;	
}

void SensorMPU6050::updateData()
{
	/* start a new acceleration computation */
	newData = false;
}

float SensorMPU6050::getVelocity()
{
	return vertAccel * VERTACCEL_G_TO_MS;
}

void SensorMPU6050::readCalibration()
{
	//calData[0] = Config.accel[0];
	//calData[1] = Config.accel[1];
	//calData[2] = Config.accel[2];
}

void SensorMPU6050::saveCalibration(float * data)
{
	Config.accel_calData[0] = data[0];
	Config.accel_calData[1] = data[1];
	Config.accel_calData[2] = data[2];
	
	Config.writeCalibrationData();
}

float * SensorMPU6050::getCalibration()
{
	return Config.accel_calData;
}
