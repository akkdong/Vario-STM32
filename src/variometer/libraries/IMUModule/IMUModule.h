// IMUModule.h
//

#ifndef __IMUMODULE_H__
#define __IMUMODULE_H__

#include "IMUSensor.h"
#include "SensorReporter.h"


///////////////////////////////////////////////////////////////////////////////////////////////
//

class IMUModule
{
public:
	IMUModule();
	
public:
	void				init(boolean calibrateGyro = true);
	
	boolean				dataReady(/*SensorReporter & reporter*/);
	boolean				rawReady(float * accel, float * upVector, float*  vertAccel);
	
	void				updateData();
	
	float				getAltitude();
	float				getTemperature();

	float				getVelocity();
	
	//
	void				initCalibration();
	
	float *				getCalibration();
	void				saveCalibration(float * calData);
	
	
private:
	SensorMPU6050 &		mpu6050;
	SensorMS5611 &		ms5611;
};


#endif // __IMUMODULE_H__
