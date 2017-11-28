// SensorReporter.cpp
//

#include "SensorReporter.h"


//                                      "0123456789012345678901234567"
#define SENSOR_SENTENCE_TAG 			"$SENSOR,x,y,z,x,y,z,p,t,*P\r\n"
#define SENSOR_SENTENCE_TAG_SIZE		28

#define SENSOR_SENTENCE_AX_POS			8
#define SENSOR_SENTENCE_AY_POS			10
#define SENSOR_SENTENCE_AZ_POS			12
#define SENSOR_SENTENCE_GX_POS			14
#define SENSOR_SENTENCE_GY_POS			16
#define SENSOR_SENTENCE_GZ_POS			18
#define SENSOR_SENTENCE_PRS_POS			20
#define SENSOR_SENTENCE_TEMP_POS		22
#define SENSOR_SENTENCE_PARITY_POS		25


/////////////////////////////////////////////////////////////////////////////
// class SensorReporter

const char * SensorReporter::tags = SENSOR_SENTENCE_TAG;

SensorReporter::SensorReporter()
{
	enableFlag 	= ENABLE_NONE;
	syncFlag	= ENABLE_NONE;
	tagPos		= (uint8_t)-1;
}

#if 0
int SensorReporter::begin(uint8_t enable)
{
	enableFlag 	= enable;
	tagPos		= (uint8_t)-1;
	
	return 1;
}

void SensorReporter::end()
{
	enableFlag 	= 0x00;
	tagPos		= (uint8_t)-1;
}
#endif

void SensorReporter::enable(uint8_t flags)
{
	if (available())
		syncFlag = flags | ENABLE_ASYNC;
	else
		enableFlag = flags;
}

int SensorReporter::read()
{
	uint8_t outc = 0;
	
	// check digits 
	if (valueDigit.available())
	{
		outc = valueDigit.get();
	}
	else if(parityDigit.available())
	{
		outc =  parityDigit.get();
	}	
	else // else write tag
	{
		outc = tags[tagPos];
		tagPos++;
		
		if (tagPos == SENSOR_SENTENCE_AX_POS)
		{
			if (enableFlag & ENABLE_ACCELEROMETER)
				valueDigit.begin(accel[0], 6);
			tagPos++;
		}
		else if (tagPos == SENSOR_SENTENCE_AY_POS)
		{
			if (enableFlag & ENABLE_ACCELEROMETER)
				valueDigit.begin(accel[1], 6);
			tagPos++;
		}
		else if (tagPos == SENSOR_SENTENCE_AZ_POS)
		{
			if (enableFlag & ENABLE_ACCELEROMETER)
				valueDigit.begin(accel[2], 6);
			tagPos++;
		}
		else if (tagPos == SENSOR_SENTENCE_GX_POS)
		{
			if (enableFlag & ENABLE_GYROSCOPE)
				valueDigit.begin(gyro[0], 6);
			tagPos++;
		}
		else if (tagPos == SENSOR_SENTENCE_GY_POS)
		{
			if (enableFlag & ENABLE_GYROSCOPE)
				valueDigit.begin(gyro[1], 6);
			tagPos++;
		}
		else if (tagPos == SENSOR_SENTENCE_GZ_POS)
		{
			if (enableFlag & ENABLE_GYROSCOPE)
				valueDigit.begin(gyro[2], 6);
			tagPos++;
		}		
		else if (tagPos == SENSOR_SENTENCE_PRS_POS)
		{
			if (enableFlag & ENABLE_PRESSURE)
				valueDigit.begin(pressure, 2);
			tagPos++;
		}
		else if (tagPos == SENSOR_SENTENCE_TEMP_POS)
		{
			if (enableFlag & ENABLE_TEMPERATURE)
				valueDigit.begin(temperature, 0);
			tagPos++;
		}
		else if (tagPos == SENSOR_SENTENCE_PARITY_POS)
		{
			parityDigit.begin(parity);
			tagPos++;
		}
	}

	// parity
	parity ^= outc;
	
	if (! available() && (syncFlag & ENABLE_ASYNC))
		enableFlag = syncFlag & (~ENABLE_ASYNC);
		
	return outc;
}

int SensorReporter::available()
{
	return (enableFlag && tagPos < SENSOR_SENTENCE_TAG_SIZE) ? true : false;
}

void SensorReporter::update(float * a, float * g, float p, float t)
{
	if (enableFlag && ! available())
	{
		if (enableFlag & ENABLE_ACCELEROMETER)
			memcpy(accel, a, sizeof(float) * 3);
		
		if (enableFlag & ENABLE_GYROSCOPE)
			memcpy(gyro, g, sizeof(float) * 3);
		
		pressure = p;
		temperature = t;
		
		tagPos = 0;
		parity = '$';
	}
}