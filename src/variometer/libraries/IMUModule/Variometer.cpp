// Variometer.cpp
//

#include "Variometer.h"


////////////////////////////////////////////////////////////////////////////////////
// class Variometer

Variometer::Variometer() : imu(mpu6050_), baro(ms5611_)
{
}

int Variometer::begin(float sigmaP, float sigmaA)
{
	//
	imu.begin(true);
	baro.begin();
	
	// read dummy data 10 times
	// it may be stabilize data...
	float prs, va;
	
	for (int i = 0; i < 10;)
	{
		imu.update();
		
		if (baro.available() && imu.available())
		{
			baro.read(&prs, 0);
			imu.read(&va);
			
			i++;
		}
	}

	// init base values
	p = MS5611::getAltitude(prs);
	a = va;
	v = 0;
	t = millis();
	altitudeDrift = 0.0;

	// init variance
	varp = sigmaP * sigmaP;
	vara = sigmaA * sigmaA;

	// init covariance matrix
	p11 = 0;
	p12 = 0;
	p21 = 0;
	p22 = 0;
	
	//
	varioUpdated = false;
}

void Variometer::end()
{
	imu.end();
	baro.end();
}

int	Variometer::available()
{
	return varioUpdated;
}

void Variometer::flush()
{
	varioUpdated = false;
}

void Variometer::update()
{
	imu.update();
	//baro.update();
	
	if (baro.available() && imu.available())
	{
		// read pressure & vertical acceleration
		float prs, va;
		
		baro.read(&prs, 0);
		imu.read(&va);
		
		// delta time
		unsigned long deltaTime = millis() - t;
		float dt = ((float)deltaTime)/1000.0;
		t = millis();

		//
		// prediction
		//

		// values
		a = va;  // we use the last acceleration value for prediction 
		float dtPower = dt * dt; //dt^2
		p += dt*v + dtPower*a/2;
		v += dt*a;
		//a = va; // uncomment to use the previous acceleration value 

		// covariance
		float inc;

		dtPower *= dt;  // now dt^3
		inc = dt*p22 + dtPower*vara / 2;
		dtPower *= dt; // now dt^4
		p11 += dt*(p12 + p21 + inc) - (dtPower*vara/4);
		p21 += inc;
		p12 += inc;
		p22 += dt*dt*vara;

		//
		// gaussian product
		//

		// kalman gain
		float s, k11, k12, y;

		s = p11 + varp;
		k11 = p11/s;
		k12 = p12/s;
		y = MS5611::getAltitude(prs) - p;

		// update
		p += k11 * y;
		v += k12 * y;
		
		p22 -= k12 * p21;
		p12 -= k12 * p11;
		p21 -= k11 * p21;
		p11 -= k11 * p11;
		
		//
		varioUpdated = true;		
	}
}
