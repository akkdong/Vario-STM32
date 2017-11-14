// VarioCalculator.h
//

#ifndef __VARIOCALCULATOR_H__
#define __VARIOCALCULATOR_H__

#include <Arduino.h>
#include <I2CDevice.h>


///////////////////////////////////////////////////////////////////////////////////////////////
// class VarioCalculator

class VarioCalculator
{
protected:
	VarioCalculator();
	
public:
	int							begin(float sigmaP, float sigmaA, int calibrateGyro = true);
	void						end();
	
	int							available();
	void						flush();
	
	int							update();
	
	float						getPressure();
	float						getTemperature();
	float						getAltitude();
	float						getCalibratedAltitude();
	float						getVelocity();
	float						getAcceleration();
	
	uint32_t					getTimestamp();
	
	void						calibrateAltitude(float altitudeRef);

public:
	static VarioCalculator &	getInstance();
	static void					unlockI2C_();
	static void					timerProc_();
	
protected:
	void						unlockI2C();
	void						timerProc();

private:
	// baro
	int							initBaro();
	
	void						resetBaro();
	
	uint16_t					getPROMValue(int address);
	uint32_t					getDigitalValue();
	float						getAltitude(float pressure);
	
	void						convertNext();
	void						convertD1(); // read temperature
	void						convertD2(); // read pressure
	
	void						updateBaro();
	
	void						startTimer();
	void						resetTimer();
	void						stopTimer();
	
	
	// motion tracking
	int							initAccel(int calibrateGyro);

	void						updateVerticalAccel();
	
	// kalman
	void						initKalman(float startP, float startA, float sigmaP, float sigmaA, uint32_t timestamp);
	
	void						updateKalman(float measureP, float measureA, uint32_t timestamp);
	
private:
	volatile uint32_t			d1i;
	volatile uint32_t			d2i;
	
	volatile int				baroState; // CONVERT_D1 or CONVERT_D2
	volatile int				baroUpdated;
	
	volatile int				interruptWait;

	uint16_t					c1, c2, c3, c4, c5, c6; // PROM factors
	float						compensatedTemperature;
	float						compensatedPressure;
//	float						baroAltitude;

private:
//	float						vertAccel;
	
	int							accumulateCount;
	float						accumulateAccel;
	
	float						accelData[3];
//	float						gyroData[3];
	float						quadData[4];
	
private:
	// position/acceleration variance
	float						varp, vara;
	
	// position, velocity, acceleration & timestamp
	float						p, v, a;
	uint32_t					t;
	
	// altitude calibration
	float						calibrationDrift;
	
	// covariance matrix
	float						p11, p12, p21, p22;
	
	//
	int							varioUpdated;
};


// inlines members

inline float VarioCalculator::getPressure()
	{ return compensatedPressure; }
	
inline float VarioCalculator::getTemperature()
	{ return compensatedTemperature; }

inline float VarioCalculator::getAltitude()
	{ return p; }

inline float VarioCalculator::getCalibratedAltitude()
	{ return (p + calibrationDrift); }

inline float VarioCalculator::getVelocity()
	{ return v; }

inline float VarioCalculator::getAcceleration()
	{ return a; }

inline uint32_t					getTimestamp();
inline void						calibrateAltitude(float altitudeRef);

#endif // __VARIOCALCULATOR_H__


/*
VarioCalculator varioCalc;


varioCalc.begin();

varioCalc.update();

if (varioCalc.available())
{
	varioCalc.getXXX();
	varioCalc.flush();
}

varioCalc.end();
*/


