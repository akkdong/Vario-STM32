// CommandParser.h
//

#ifndef __COMMANDPARSER_H__
#define __COMMANDPARSER_H__

#include <Arduino.h>
#include "CommandStack.h"


/////////////////////////////////////////////////////////////////////////////
// class CommandParser

class CommandParser
{
public:
	CommandParser(Stream & strm, CommandStack & stack);

public:
	void			update();

private:
	Stream &		Strm;
	CommandStack & 	Stack;
};

#endif // __COMMANDPARSER_H__
