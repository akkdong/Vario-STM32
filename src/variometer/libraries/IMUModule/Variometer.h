// Variometer.h
//

#ifndef __VARIOMETER_H__
#define __VARIOMETER_H__

#include "MPU6050_.h"
#include "MS5611.h"


////////////////////////////////////////////////////////////////////////////////////
// class Variometer

class Variometer
{
public:
	Variometer();
	
public:
	int					begin(float sigmaP, float sigmaA);
	void				end();
	
	int					available();
	void				flush();
	
	void				update();
	
	float				getPressure();
	float				getTemperature();
	float				getAltitude2();
	float				getAltitude();
	float				getCalibratedAltitude();
	float				getVelocity();
	float				getAcceleration();
	
	uint32_t			getTimestamp();
	
	void				calibrateAltitude(float altitudeRef);
	
private:
	// position/acceleration variance
	float				varp, vara;
	
	// position, velocity, acceleration & timestamp
	float				p, v, a;
	uint32_t			t;
	
	// altitude calibration
	float				altitudeDrift;
	
	// covariance matrix
	float				p11, p12, p21, p22;
	
	//
	int					varioUpdated;
	
	//
	MPU6050_ &			imu;
	MS5611 &			baro;
};


// inline members
inline float Variometer::getPressure()
	{ return baro.getPressure(); }

inline float Variometer::getTemperature()
	{ return baro.getTemperature(); }

inline float Variometer::getAltitude2()
	{ return p; }

inline float Variometer::getAltitude()
//	{ return p; }
	{ return MS5611::getAltitude(baro.getPressure()); }

inline float Variometer::getCalibratedAltitude()
//	{ return (p + altitudeDrift); }
	{ return (getAltitude() + altitudeDrift); }

inline float Variometer::getVelocity()
	{ return v; }

inline float Variometer::getAcceleration()
	{ return a; }

inline uint32_t Variometer::getTimestamp()
	{ return t; }

inline void Variometer::calibrateAltitude(float altitudeRef)
	{ altitudeDrift = altitudeRef - getAltitude(); }


#endif // __VARIOMETER_H__
