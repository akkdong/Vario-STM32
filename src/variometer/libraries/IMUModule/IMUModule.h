// IMUModule.h
//

#ifndef __IMUMODULE_H__
#define __IMUMODULE_H__

#include "IMUSensor.h"


///////////////////////////////////////////////////////////////////////////////////////////////
//

class IMUModule
{
public:
	IMUModule();
	
public:
	void				init(boolean calibrateGyro = true);
	
	boolean				dataReady();
	boolean				rawReady(double * accel, double * upVector, double*  vertAccel);
	
	void				updateData();
	
	double				getAltitude();
	double				getTemperature();

	double				getVelocity();
	
	//
	void				initCalibration();
	
	double *			getCalibration();
	void				saveCalibration(double * calData);
	
	
private:
	SensorMPU6050 &		mpu6050;
	SensorMS5611 &		ms5611;
};


#endif // __IMUMODULE_H__
