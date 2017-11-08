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
	void			begin(uint8_t pin); // only one pin can be used in same timer
	void			end();

	void			setTone(uint16_t hz, uint8_t volume = 100);
	
private:
	uint8_t			tonePin;
};


#endif // __PWMGENERATOR_H__
