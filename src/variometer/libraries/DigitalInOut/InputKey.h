// InputKey.h
//

#ifndef __INPUTKEY_H__
#define __INPUTKEY_H__

#include <DefaultSettings.h>


///////////////////////////////////////////////////////////////////////////////////////////////
// class InputKey

class InputKey
{
public:
	void			begin(uint8_t pin, uint8_t active);
	
	int				read();	// INPUT_HIGH or INPUT_LOW
	void			interrupt();
	
private:
	uint8_t			inptuPin;
	uint8_t			activeState;
};

#endif // __INPUTKEY_H__
