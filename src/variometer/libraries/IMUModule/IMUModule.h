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
	
	SensorMS5611 &		getPressureSensor();
	SensorMPU6050 &		getAccelerometerSensor();
	
private:
//	SensorMPU6050 &		mpu6050_;
//	SensorMS5611 &		ms5611_;
};

// inline members

inline SensorMS5611 & IMUModule::getPressureSensor()
	{ return ms5611; }
	
inline SensorMPU6050 & IMUModule::getAccelerometerSensor()
	{ return mpu6050; }
	
#endif // __IMUMODULE_H__
