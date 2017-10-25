// ToneGenerator.h
//

#ifndef __TONEGENERATOR_H__
#define __TONEGENERATOR_H__

#include <Arduino.h>
#include <HardwareTimer.h>


////////////////////////////////////////////////////////////////////////////////////
// class ToneGenerator

class ToneGenerator
{
public:
	ToneGenerator();
	
public:
	void			begin(uint8 pin); // only one pin can be used in same timer
	void			end();

	void			setTone(uint16 hz, uint8 volume = 100);
	
private:
	uint8			tonePin;
};


#endif // __PWMGENERATOR_H__
