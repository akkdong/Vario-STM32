// BluetoothMan.cpp
//

#include <Arduino.h>
#include <SerialEx.h>
#include <NmeaParserEx.h>
#include <VarioSentence.h>
#include <SensorReporter.h>
#include <ResponseStack.h>

#include "BluetoothMan.h"

/*
#define CLEAR_STATE()		lockState = 0;
#define SET_STATE(bit)		lockState |= (1 << bit)
#define UNSET_STATE(bit)	lockState &= ~(1 << bit)

#define IS_SET(bit)			(lockState & (1 << bit))


#define LOCKED_BY_VARIO		1
#define LOCKED_BY_GPS			2
*/

enum BTMAN_LOCK_STATE
{
	BTMAN_UNLOCKED = 0,
	BTMAN_LOCKED_BY_VARIO,
	BTMAN_LOCKED_BY_GPS,
	BTMAN_LOCKED_BY_SENSOR,
	BTMAN_LOCKED_BY_RESPONSE,
};


/////////////////////////////////////////////////////////////////////////////
// class BluetoothMan

BluetoothMan::BluetoothMan(SerialEx & serial, NmeaParserEx & nmea, VarioSentence & vario, SensorReporter & sensor, ResponseStack & resp) :
	SerialBT(serial), nmeaParser(nmea), varioSentence(vario), sensorReporter(sensor), responseStack(resp),
	lockState(BTMAN_UNLOCKED)/*, blockTransfer(BTMAN_BLOCK_NONE)*/
{
}


void BluetoothMan::update()
{
#if 0
	if (lockState == BTMAN_LOCKED_BY_VARIO)
	{
		// send Vario NMEA setence
		writeVarioSentence();
	}
	else if (lockState == BTMAN_LOCKED_BY_GPS)
	{
		// send GPS NMEA sentence
		writeGPSSentence();
	}
	else if (lockState == BTMAN_LOCKED_BY_SENSOR)
	{
		// send Sensor Data sentence
		writeSensorData();
	}
	else if (lockState == BTMAN_LOCKED_BY_RESPONSE)
	{
		// send Sensor Data sentence
		writeResponse();
	}
	else
	{
		if (varioSentence.available() /*&& ! (blockTransfer & BTMAN_BLOCK_NMEA)*/)
		{
			lockState = BTMAN_LOCKED_BY_VARIO;			
			writeVarioSentence();
		}
		else if (nmeaParser.available() /*&& ! (blockTransfer & BTMAN_BLOCK_NMEA)*/)
		{
			lockState = BTMAN_LOCKED_BY_GPS;
			writeGPSSentence();
		}
		else if (sensorReporter.available() /*&& ! (blockTransfer & BTMAN_BLOCK_SENSOR)*/)
		{
			lockState = BTMAN_LOCKED_BY_SENSOR;
			writeSensorData();
		}
		else if (responseStack.available())
		{
			lockState = BTMAN_LOCKED_BY_RESPONSE;
			writeResponse();
		}
	}
#else	
	if (lockState == BTMAN_UNLOCKED)
	{
		if (varioSentence.available() /*&& ! (blockTransfer & BTMAN_BLOCK_NMEA)*/)
		{
			lockState = BTMAN_LOCKED_BY_VARIO;			
			writeVarioSentence();
		}
		else if (nmeaParser.available() /*&& ! (blockTransfer & BTMAN_BLOCK_NMEA)*/)
		{
			lockState = BTMAN_LOCKED_BY_GPS;
			writeGPSSentence();
		}
		else if (sensorReporter.available() /*&& ! (blockTransfer & BTMAN_BLOCK_SENSOR)*/)
		{
			lockState = BTMAN_LOCKED_BY_SENSOR;
			writeSensorData();
		}
		else if (responseStack.available())
		{
			lockState = BTMAN_LOCKED_BY_RESPONSE;
			writeResponse();
		}
	}
	else
	{
		if (lockState == BTMAN_LOCKED_BY_VARIO)
		{
			// send Vario NMEA setence
			writeVarioSentence();
		}
		else if (lockState == BTMAN_LOCKED_BY_GPS)
		{
			// send GPS NMEA sentence
			writeGPSSentence();
		}
		else if (lockState == BTMAN_LOCKED_BY_SENSOR)
		{
			// send Sensor Data sentence
			writeSensorData();
		}		
		else if (lockState == BTMAN_LOCKED_BY_RESPONSE)
		{
			// send Sensor Data sentence
			writeResponse();
		}		
	}
#endif
}

void BluetoothMan::writeGPSSentence()
{
	while (SerialBT.availableForWrite())
	{
		//
		int c = nmeaParser.read();
		if (c < 0)
			break;
		
		//
		SerialBT.write(c);
		
		if (c == '\n') // last setence character : every sentence end with '\r\n'
		{			
			lockState = BTMAN_UNLOCKED;
			break;
		}
	}
}

void BluetoothMan::writeVarioSentence()
{
	while (SerialBT.availableForWrite())
	{
		//
		int c = varioSentence.read();
		if (c < 0)
			break;
		
		//
		SerialBT.write(c);
		
		if (c == '\n') // last setence character : every sentence end with '\r\n'
		{
			lockState = BTMAN_UNLOCKED;
			break;
		}
	}
}

void BluetoothMan::writeSensorData()
{
	while (SerialBT.availableForWrite())
	{
		//
		int c = sensorReporter.read();
		if (c < 0)
			break;
		
		//
		SerialBT.write(c);
		
		if (c == '\n') // last setence character : every sentence end with '\r\n'
		{
			lockState = BTMAN_UNLOCKED;
			break;
		}
	}
}

void BluetoothMan::writeResponse()
{
	while (SerialBT.availableForWrite())
	{
		//
		int c = responseStack.read();
		if (c < 0)
			break;
		
		//
		SerialBT.write(c);
		
		if (c == '\n') // last setence character : every sentence end with '\r\n'
		{
			lockState = BTMAN_UNLOCKED;
			break;
		}
	}
}

#if 0
void BluetoothMan::blockNmeaSentence(uint8_t block)
{
	if (block)
		blockTransfer |= BTMAN_BLOCK_NMEA;
	else
		blockTransfer &= ~(BTMAN_BLOCK_NMEA);
}

void BluetoothMan::blockSensorData(uint8_t block)
{
	// block all or block each sensor value ??????
	// 
}
#endif

int BluetoothMan::available()
{
	return SerialBT.available();
}

int BluetoothMan::read()
{
	return SerialBT.read();
}
