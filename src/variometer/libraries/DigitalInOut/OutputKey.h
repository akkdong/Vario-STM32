// OutputKey.h
//

#ifndef __OUTPUTKEY_H__
#define __OUTPUTKEY_H__

#include <DefaultSettings.h>
#include <Arduino.h>


///////////////////////////////////////////////////////////////////////////////////////////////
// class OutputKey

class OutputKey
{
public:
	void			begin(uint8_t pin, uint8_t active, uint8_t init = OUTPUT_HIGH);
	
	void			enable();
	void			disable();
	
private:
	uint8_t			outputPin;
	uint8_t			activeState;
};

#endif // __OUTPUTKEY_H__
