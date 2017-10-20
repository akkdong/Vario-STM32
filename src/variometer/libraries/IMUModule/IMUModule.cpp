// IMUModule.cpp
//

#include <Arduino.h>
#include "IMUModule.h"

///////////////////////////////////////////////////////////////////////////////////////////////
//

IMUModule::IMUModule() :
	mpu6050(SensorMPU6050::GetInstance()),
	ms5611(SensorMS5611::GetInstance())
{
}

void IMUModule::init(boolean calibrateGyro)
{
	mpu6050.initSensor(calibrateGyro);
	ms5611.initSensor();
}

boolean IMUModule::dataReady()
{
	return ms5611.dataReady() && mpu6050.dataReady();
}

void IMUModule::updateData()
{
	mpu6050.updateData();
	ms5611.updateData();
}

double IMUModule::getAltitude()
{
	return ms5611.getAltitude();
}

double IMUModule::getTemperature()
{
	return ms5611.getTemperature();
}

double IMUModule::getVelocity()
{
	return mpu6050.getVelocity();
}
