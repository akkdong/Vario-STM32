// CommandParser.h
//

#ifndef __COMMANDPARSER_H__
#define __COMMANDPARSER_H__

#include <Arduino.h>
#include "CommandStack.h"

#define MAX_FIELD_LEN	(32)


/////////////////////////////////////////////////////////////////////////////
// class CommandParser

class CommandParser
{
public:
	CommandParser(uint8_t src, Stream & strm, CommandStack & stack);

public:
	static int		readLine(Stream & strm, char * buf, int len, int timeout = 1000);

	void			update();

private:
	uint32_t		toNum(const char * str);

private:
	//
	uint8_t			StrmSource;
	Stream &		Strm;
	
	CommandStack & 	Stack;
	
	//
	int32_t			parseStep;
	
	char			fieldData[MAX_FIELD_LEN];
	uint8_t			fieldIndex;
	
	uint16_t		cmdCode;
	uint32_t		cmdParam;
	
	uint8_t			cmdValue[MAX_VALUE_STRING];
	uint16_t		valueLen;
};

#endif // __COMMANDPARSER_H__
