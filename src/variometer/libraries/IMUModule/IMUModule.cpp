// IMUModule.cpp
//

#include <Arduino.h>
#include "IMUModule.h"

///////////////////////////////////////////////////////////////////////////////////////////////
//

InertialMeasurementUnit::InertialMeasurementUnit() :
	mpu6050(SensorMPU6050::GetInstance()),
	ms5611(SensorMS5611::GetInstance())
{
}

void InertialMeasurementUnit::init(boolean calibrateGyro)
{
	mpu6050.initSensor(calibrateGyro);
	ms5611.initSensor();
}

boolean InertialMeasurementUnit::dataReady()
{
	return ms5611.dataReady() && mpu6050.dataReady();
}

void InertialMeasurementUnit::updateData()
{
	mpu6050.updateData();
	ms5611.updateData();
}

double InertialMeasurementUnit::getAltitude()
{
	return ms5611.getAltitude();
}

double InertialMeasurementUnit::getTemperature()
{
	return ms5611.getTemperature();
}

double InertialMeasurementUnit::getVelocity()
{
	return mpu6050.getVelocity();
}
