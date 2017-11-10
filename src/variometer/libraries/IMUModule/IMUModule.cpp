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
	ms5611.initSensor();
	mpu6050.initSensor(calibrateGyro);
}

boolean IMUModule::dataReady()
{
	return ms5611.dataReady() && mpu6050.dataReady();
}

boolean IMUModule::rawReady(float* accel, float* upVector, float* vertAccel)
{
	return mpu6050.rawReady(accel, upVector, vertAccel);
}

void IMUModule::updateData(/*SensorReporter & reporter*/)
{
	mpu6050.updateData();
	ms5611.updateData();
	
	//Serial.print(ms5611.getPressure());
	//Serial.print(", ");
	//Serial.print(ms5611.getTemperature());
	//Serial.println("");
	//reporter.setData(ms5611.getPressure(), ms5611.getTemperature(), 
	//				mpu6050.getRawAccel(), mpu6050.getRawGyro());
}

float IMUModule::getAltitude()
{
	return ms5611.getAltitude();
}

float IMUModule::getTemperature()
{
	return ms5611.getTemperature();
}

float IMUModule::getVelocity()
{
	return mpu6050.getVelocity();
}

//
void IMUModule::initCalibration()
{
	mpu6050.initSensor(false);
}

float * IMUModule::getCalibration()
{
	return mpu6050.getCalibration();
}

void IMUModule::saveCalibration(float * calData)
{
	mpu6050.saveCalibration(calData);
}
