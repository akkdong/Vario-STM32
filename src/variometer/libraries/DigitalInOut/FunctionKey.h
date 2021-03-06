// FunctionKey.h
//

#ifndef __FUNCTIONKEY_H__
#define __FUNCTIONKEY_H__

#include <DefaultSettings.h>
#include <ToneFrequency.h>
#include <TonePlayer.h>


///////////////////////////////////////////////////////////////////////////////////////////////
// class FunctionKey

class FunctionKey
{
public:
	FunctionKey();
	
public:
	void			begin(int pin, int active = FKEY_INPUT_ACTIVE);
	void			update();
	
	boolean			fired();
	
	uint16_t		getValue();
	
	Tone *			getTone();
	int				getToneCount();

private:
	int				getInput();
	
private:
	uint32_t		lastTick;
	
	int				inputPin;
	int				activeState;
	
	int				inputIndex;
	int				inputState;
	
	uint16_t		inputValue;
	uint16_t 		returnValue;
	
	Tone			tone[10*2];
	int				toneCount;
};


// inlines members
//

inline Tone * FunctionKey::getTone()
	{ return &tone[0]; }
	
inline int FunctionKey::getToneCount()
	{ return toneCount; }


/*

//
// FunctionKey usage example
//
 
FunctionKey	funcKey;

void setup()
{
	//
	funcKey.begin(PIN_FUNC_INPUT);
}

void loop()
{
	funcKey.update();

	if (funcKey.fired())
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
		
		uint8_t value = funcKey.getValue();
	}
}

*/

#endif // __FUNCTIONKEY_H__
