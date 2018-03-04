// FuncKeyParser.h
//

#ifndef __FUNCKEYPARSER_H__
#define __FUNCKEYPARSER_H__

#include <Arduino.h>
#include <FunctionKey.h>
#include <TonePlayer.h>

#include "CommandStack.h"


/////////////////////////////////////////////////////////////////////////////
// class FuncKeyParser

class FuncKeyParser
{
public:
	FuncKeyParser(FunctionKey & input, CommandStack & stack, TonePlayer & player);

public:
	void			update();

private:
	FunctionKey &	Input;
	CommandStack & 	Stack;
	TonePlayer &	Player;
	
	// 
	uint32_t		inputTick;
	uint16_t		inputCmd;
	uint16_t		inputParam;
};

#endif // __FUNCKEYPARSER_H__
