// SensorReporter.h
//

#ifndef __SENSORREPORTER_H__
#define __SENSORREPORTER_H__

#include <DefaultSettings.h>
#include <Arduino.h>


/////////////////////////////////////////////////////////////////////////////
// class SensorReporter

class SensorReporter
{
public:
	SensorReporter();
	
public:	
	int					available();
	int					read();
};


#endif // __SENSORREPORTER_H__