// IMUModule.h
//

#ifndef __IMUMODULE_H__
#define __IMUMODULE_H__

#include "IMUSensor.h"


///////////////////////////////////////////////////////////////////////////////////////////////
//

class InertialMeasurementUnit
{
public:
	InertialMeasurementUnit();
	
public:
	void				init(boolean calibrateGyro = true);
	
	boolean				dataReady();
	void				updateData();
	
	double				getAltitude();
	double				getTemperature();

	double				getVelocity();
	
private:
	SensorMPU6050 &		mpu6050;
	SensorMS5611 &		ms5611;
};


#endif // __IMUMODULE_H__
