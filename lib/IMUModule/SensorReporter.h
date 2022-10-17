// SensorReporter.h
//

#ifndef __SENSORREPORTER_H__
#define __SENSORREPORTER_H__

#include <DefaultSettings.h>
#include <Digit.h>

#define ENABLE_NONE					(0)
#define ENABLE_ACCELEROMETER		(1 << 0)
#define ENABLE_GYROSCOPE			(1 << 1)
#define ENABLE_PRESSURE				(1 << 2)
#define ENABLE_TEMPERATURE			(1 << 3)
#define ENABLE_ASYNC				(1 << 7)

/////////////////////////////////////////////////////////////////////////////
// class SensorReporter

class SensorReporter
{
public:
	SensorReporter();
	
public:	
//	int					begin(uint8_t enable);
//	void				end();
	
	int					available();
	int					read();
	
	void				update(float * a, float * g, float p, float t);
	
	void				enable(uint8_t flags);
	
private:
	float				gyro[3];
	float				accel[3];
	float				pressure;
	float				temperature;
	
	uint8_t				enableFlag;
	uint8_t				syncFlag;
	
	Digit 				valueDigit;
	HexDigit 			parityDigit;
	
	unsigned char		tagPos;
	unsigned char		parity;
	
	static const char *	tags;
};


#endif // __SENSORREPORTER_H__