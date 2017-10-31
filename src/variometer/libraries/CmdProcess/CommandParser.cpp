// CommandParser.cpp
//

#include "CommandParser.h"


/////////////////////////////////////////////////////////////////////////////
// class CommandParser

CommandParser::CommandParser(Stream & strm, CommandStack & stack) : Strm(strm), Stack(stack)
{
}

void CommandParser::update()
{
	while (Strm.available())
	{
		int c = Strm.read();
		
		// ...
		//
		// Stack.enqueue(cmd);
	}
}
