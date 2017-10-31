// DigitalInput.h
//

#ifndef __DIGITALINPUT_H__
#define __DIGITALINPUT_H__

#include <DefaultSettings.h>


///////////////////////////////////////////////////////////////////////////////////////////////
// class DigitalInput

class DigitalInput
{
public:
	DigitalInput();
	
public:
	void			begin(int pin);
	void			update();
	
	boolean			fired();
	
	unsigned char	getValue();

private:
	int				getInput();
	
private:
	long			lastTick;
	
	int				inputPin;
	
	int				inputIndex;
	int				inputState;
	unsigned char	inputValue;
	
	unsigned char 	returnValue;
};


/*

//
// DigitalInput usage example
//
 
DigitalInput	funcInput;

void setup()
{
	//
	funcInput.begin(PIN_FUNC_INPUT);
}

void loop()
{
	funcInput.update();

	if (funcInput.fired())
	{
		// value format :
		// b7 b6 b5 b4 b3 b2 b1 b0
		// C2 C1 C0 I4 I3 I2 I1 I0
		//
		// Cx : count of bits (valid: 1~5, forbidden(ignore): 0, 6, 7)
		// Ix : input value
		//      each bit represents LONG(1) or SHORT(0) input, default is SHORT
		//		MSB first, RIGHT aligned
		//
		
		uint8_t value = funcInput.getValue();
	}
}

*/

#endif // __DIGITALINPUT_H__
