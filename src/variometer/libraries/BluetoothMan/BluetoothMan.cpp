// BluetoothMan.cpp
//

#include <Arduino.h>
#include <SerialEx.h>
#include <NmeaParserEx.h>
#include <VarioSentence.h>

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
	BTMAN_LOCKED_BY_GPS
};


/////////////////////////////////////////////////////////////////////////////
// class BluetoothMan

BluetoothMan::BluetoothMan(SerialEx & serial, NmeaParserEx & nmea, VarioSentence & vario) :
	serialBT(serial), nmeaParser(nmea), varioSentence(vario), lockState(BTMAN_UNLOCKED)
{
}


void BluetoothMan::update()
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
	else
	{
		if (varioSentence.available())
		{
			lockState = BTMAN_LOCKED_BY_VARIO;
			
			writeVarioSentence();
		}
		else if (nmeaParser.available())
		{
			lockState = BTMAN_LOCKED_BY_GPS;
			
			writeGPSSentence();
		}
	}
}

void BluetoothMan::writeGPSSentence()
{
	while (serialBT.availableForWrite())
	{
		int c = nmeaParser.read();
		serialBT.write(c);
		
		if (c == '\n') // last setence character : every sentence end with '\r\n'
		{			
			lockState = BTMAN_UNLOCKED;
			break;
		}
	}
}

void BluetoothMan::writeVarioSentence()
{
	while (serialBT.availableForWrite())
	{
		int c = varioSentence.read();
		serialBT.write(c);
		
		if (c == '\n') // last setence character : every sentence end with '\r\n'
		{
			lockState = BTMAN_UNLOCKED;
			break;
		}
	}
}

int BluetoothMan::available()
{
	return serialBT.available();
}

int BluetoothMan::read()
{
	return serialBT.read();
}
