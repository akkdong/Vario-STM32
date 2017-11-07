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

#define VERTACCEL_GIRO_FSR 		(2000)
#define VERTACCEL_ACCEL_FSR 	(4)
#define VERTACCEL_FIFO_RATE 	(100)

/* 4G ~= 2^15 */
#define VERTACCEL_ACCEL_SCALE 	(8192.0)


/* 2^30 */
#define VERTACCEL_QUAT_SCALE 	(1073741824.0)


///////////////////////////////////////////////////////////////////////////////////////////////
// class SensorMPU6050

SensorMPU6050::SensorMPU6050()
{
}

SensorMPU6050 & SensorMPU6050::GetInstance()
{
	static SensorMPU6050 mpu6050;
	
	return mpu6050;
}

void SensorMPU6050::initSensor(boolean calibrateGyro)
{
	/* setting imu */
	//mpu_select_device(0);
	//mpu_init_structures();
	mpu_init(NULL);
	mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL); 
	mpu_set_gyro_fsr(VERTACCEL_GIRO_FSR);
	mpu_set_accel_fsr(VERTACCEL_ACCEL_FSR);
	mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL);

	/* setting dmp */
	//dmp_select_device(0);
	//dmp_init_structures();
	dmp_load_motion_driver_firmware();
	dmp_set_fifo_rate(VERTACCEL_FIFO_RATE);
	mpu_set_dmp_state(1);
	if( calibrateGyro ) {
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_GYRO_CAL); 
	} else {
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_ACCEL);
	}

	/* init calibration settings */
	readCalibration();

	/* init data variables */
	newData = false;	
}

boolean	SensorMPU6050::dataReady()
{
	short iaccel[3];
	long iquat[4];
	unsigned long timestamp;
	short sensors;
	unsigned char fifoCount;

	/* check if we have new data from imu */
	while( dmp_read_fifo(NULL,iaccel,iquat,&timestamp,&sensors,&fifoCount) == 0 ) {
		newData = true;
	}

	/* if new data compute vertical acceleration */
	if( newData ) {
		/***************************/
		/* normalize and calibrate */
		/***************************/
		double accel[3]; 
		double quat[4]; 

		accel[0] = ((double)iaccel[0])/VERTACCEL_ACCEL_SCALE + calData[0];
		accel[1] = ((double)iaccel[1])/VERTACCEL_ACCEL_SCALE + calData[1];
		accel[2] = ((double)iaccel[2])/VERTACCEL_ACCEL_SCALE + calData[2];

		quat[0] = ((double)iquat[0])/VERTACCEL_QUAT_SCALE;
		quat[1] = ((double)iquat[1])/VERTACCEL_QUAT_SCALE;
		quat[2] = ((double)iquat[2])/VERTACCEL_QUAT_SCALE;
		quat[3] = ((double)iquat[3])/VERTACCEL_QUAT_SCALE;

		/******************************/
		/* real and vert acceleration */
		/******************************/

		/* compute upper direction from quaternions */
		double ux, uy, uz;
		ux = 2*(quat[1]*quat[3]-quat[0]*quat[2]);
		uy = 2*(quat[2]*quat[3]+quat[0]*quat[1]);
		uz = 2*(quat[0]*quat[0]+quat[3]*quat[3])-1;

		/* compute real acceleration (without gravity) */
		double rax, ray, raz;
		rax = accel[0] - ux;
		ray = accel[1] - uy;
		raz = accel[2] - uz;


		/* compute vertical acceleration */
		vertAccel = (ux*rax + uy*ray + uz*raz);
	}

	return newData;	
}

boolean	SensorMPU6050::rawReady(double * accel, double * uv, double * va)
{
	short iaccel[3];
	long iquat[4];
	unsigned long timestamp;
	short sensors;
	unsigned char fifoCount;

	/* check if we have new data from imu */
	while( dmp_read_fifo(NULL,iaccel,iquat,&timestamp,&sensors,&fifoCount) == 0 ) {
		newData = true;
	}

	/* if new data compute vertical acceleration */
	if( newData ) {
		/*************/
		/* normalize */
		/*************/ 
		double quat[4]; 

		accel[0] = ((double)iaccel[0])/VERTACCEL_ACCEL_SCALE;
		accel[1] = ((double)iaccel[1])/VERTACCEL_ACCEL_SCALE;
		accel[2] = ((double)iaccel[2])/VERTACCEL_ACCEL_SCALE;

		quat[0] = ((double)iquat[0])/VERTACCEL_QUAT_SCALE;
		quat[1] = ((double)iquat[1])/VERTACCEL_QUAT_SCALE;
		quat[2] = ((double)iquat[2])/VERTACCEL_QUAT_SCALE;
		quat[3] = ((double)iquat[3])/VERTACCEL_QUAT_SCALE;


		/******************************/
		/* real and vert acceleration */
		/******************************/

		/* compute upper direction from quaternions */
		double ux, uy, uz;
		ux = 2*(quat[1]*quat[3]-quat[0]*quat[2]);
		uy = 2*(quat[2]*quat[3]+quat[0]*quat[1]);
		uz = 2*(quat[0]*quat[0]+quat[3]*quat[3])-1;

		/* compute real acceleration (without gravity) */
		double rax, ray, raz;
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

double SensorMPU6050::getVelocity()
{
	return vertAccel * VERTACCEL_G_TO_MS;
}

void SensorMPU6050::readCalibration()
{
	calData[0] = Config.accel[0];
	calData[1] = Config.accel[1];
	calData[2] = Config.accel[2];
}

void SensorMPU6050::saveCalibration(double * data)
{
	calData[0] = data[0];
	calData[1] = data[1];
	calData[2] = data[2];
	
	Config.updateAccelCalibration(data);
}

double * SensorMPU6050::getCalibration()
{
	return calData;
}
