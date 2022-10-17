// KalmanVVelocity.h
//

#include <Arduino.h>
#include <IVerticalVelocity.h>


////////////////////////////////////////////////////////////////////////////////////
// class KalmanVVelocity

class KalmanVVelocity : public IVerticalVelocity
{
public:
	KalmanVVelocity();

public:
	int			begin(float p, float a, float sigmaP, float sigmaA, uint32_t timestamp);
	int 		update(float p, float a, uint32_t timestamp);
	
	int			ready();
	
	uint32_t 	getTimestamp();
	float		getVelocity();
	float		getAltitude();
	float		getCalibratedAltitude();
	
	float		calibrateAltitude(float refAltitude);
	
private:
	// position variance, acceleration variance
	float varp, vara;

	// position, velocity, acceleration, timestamp
	float p, v, a;
	unsigned long t;

	// calibration
	float calibrationDrift;

	// covariance matrix
	float p11, p21, p12, p22;
	
	// begin flag
	volatile uint8_t flag;
};
