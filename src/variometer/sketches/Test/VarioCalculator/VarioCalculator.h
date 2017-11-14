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
	int							begin();
	void						end();
	
	int							available();
	void						flush();
	
	int							update();

public:
	static VarioCalculator &	getInstance();
	
	static void					unlockI2C_();
	void						unlockI2C();
	
	static void					timerProc_();
	void						timerProc();

private:
	// baro
	int							initBaro();
	
	void						resetBaro();
	uint16_t					getPROMValue(int address);
	uint32_t					getDigitalValue();
	
	void						convertNext();
	void						convertD1(); // read temperature
	void						convertD2(); // read pressure
	
	void						updateBaro();
	
	void						startTimer();
	void						resetTimer();
	void						stopTimer();
	
	
	// motion tracking
	int							initAccel();

	void						updateVerticalAccel();
	
	// kalman
	int							initKalman();
	
	int							updateKalman();
	
private:
	volatile uint32_t			d1i;
	volatile uint32_t			d2i;
	
	volatile int				baroState; // CONVERT_D1 or CONVERT_D2
	volatile int				baroUpdated;
	
	volatile int				interruptWait;
	
	uint16_t					c1, c2, c3, c4, c5, c6; // PROM factors
	float						compensatedTemperature;
	float						compensatedPressure;

private:
	float						vertAccel;
	
	int							accumulateCount;
	float						accumulateAccel;
	
	float						accelData[3];
	float						gyroData[3];
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
};

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


