// IMUModule.h
//

#ifndef __IMUMODULE_H__
#define __IMUMODULE_H__

/* the pressure normalized sea level pressure */  
#define MS5611_BASE_SEA_PRESSURE 1013.25


/* accelerometer parameters */
#define VERTACCEL_G_TO_MS 		9.80665

#define VERTACCEL_GIRO_FSR 		2000
#define VERTACCEL_ACCEL_FSR 	4
#define VERTACCEL_FIFO_RATE 	100

/* 4G ~= 2^15 */
#define VERTACCEL_ACCEL_SCALE 	8192.0


/* 2^30 */
#define VERTACCEL_QUAT_SCALE 	1073741824.0



///////////////////////////////////////////////////////////////////////////////////////////////
//

class InertialMeasurementUnit
{
public:
	InertialMeasurementUnit();
	
public:
	void				initSensor(boolean calibrateGyro = true);
	
	boolean				dateReady();
	void				updateData();
	
	double				getAltitude();
	double				getTemperature();

	double				getVelocity();
};


#endif // __IMUMODULE_H__
