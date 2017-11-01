// FuncKeyParser.h
//

#ifndef __FUNCKEYPARSER_H__
#define __FUNCKEYPARSER_H__

#include <Arduino.h>
#include <DigitalInput.h>
#include <TonePlayer.h>

#include "CommandStack.h"


/////////////////////////////////////////////////////////////////////////////
// class FuncKeyParser

class FuncKeyParser
{
public:
	FuncKeyParser(DigitalInput & input, CommandStack & stack, TonePlayer & player);

public:
	void			update();

private:
	DigitalInput &	Input;
	CommandStack & 	Stack;
	TonePlayer &	Player;
	
	// 
	uint32_t		inputTick;
	uint16_t		inputCmd;
	uint8_t			inputParam;
};

#endif // __FUNCKEYPARSER_H__
