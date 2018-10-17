// CommandParser.cpp
//

#include "CommandParser.h"

#define PARSE_STEP_WAIT			(-1)	// wait command
#define PARSE_STEP_CMD_1ST
#define PARSE_STEP_

/////////////////////////////////////////////////////////////////////////////
// class CommandParser

CommandParser::CommandParser(uint8_t src, Stream & strm, CommandStack & stack) : StrmSouce(src), Strm(strm), Stack(stack)
{
}

int CommandParser::readLine(Stream & strm, char * buf, int len, int timeout)
{
	uint32_t tick = millis();
	int index = 0;

	while ((millis() - tick) < timeout && index < len - 1)
	{
		if (strm.available())
		{
			int c = strm.read();

			if (c == '\r')
				continue;
			if (c == '\n')
				return index;

			buf[index++] = c;
			buf[index] = 0;
		}
	}

	return -1;
}

void CommandParser::update()
{
	while (Strm.available())
	{
		int c = Strm.read();
		
		// command format
		// #XX[,PARAM[,VALUE]]\r\n
		//
		
		if (parseStep < 0 && c != '#')
			continue;
		
		if (c == '#')
		{
			// leading character... start parsing
			parseStep = 0;
			
			fieldIndex = 0;
			fieldData[0] = '\0';
			
			cmdCode = 0;
			cmdParam = 0;
			valueLen = 0;
		}
		else
		{
			if (parseStep == 0)
			{
				// first command identifier
				cmdCode = c;
				//
				parseStep = 1;
			}
			else if (parseStep == 1)
			{
				// second command identifier
				cmdCode = (cmdCode << 8) + c;
				//
				parseStep = 2;
			}
			else if (parseStep == 2)
			{
				if (c == ',' || c == '\r' || c == '\r')
				{
					if (c == ',')
						parseStep = 3; // next is param field
					else if (c == '\r')
						parseStep = 5; // next is '\n'
					else
						parseStep = 6; // end of command string
				}
				else
				{
					// invalid command string
					parseStep = -1;
				}
			}
			else if (parseStep == 3)
			{
				// param field
				if (c == '\r' || c == '\n' || c == ',')
				{
					cmdParam = toNum(fieldData);

					fieldIndex = 0;
					fieldData[0] = '\0';
					
					if (c == ',')
						parseStep = 4; // next is vale field
					else if (c == '\r')
						parseStep = 5; // next is '\n'
					else
						parseStep = 6; // end of command string
				}
				else
				{
					if (fieldIndex < MAX_FIELD_LEN - 1)
					{
						fieldData[fieldIndex++] = c;
						fieldData[fieldIndex] = '\0';
					}
					else
					{
						// too long field : invalid command string
						parseStep = -1;
					}
				}
			}
			else if (parseStep == 4)
			{
				// param field
				if (c == '\r' || c == '\n')
				{
					memcpy(cmdValue, fieldData, fieldIndex);
					valueLen = fieldIndex;

					if (c == '\r')
						parseStep = 5; // next is '\n'
					else
						parseStep = 6; // end of command string
				}
				else
				{
					if (fieldIndex < MAX_FIELD_LEN - 1)
					{
						fieldData[fieldIndex++] = c;
						fieldData[fieldIndex] = '\0';
					}
					else
					{
						// too long field : invalid command string
						parseStep = -1;
					}
				}
			}
			else if (parseStep == 5)
			{
				if (c == '\n')
					parseStep = 6; // end of command string
				else
					parseStep = -1; // invalid
			}
			
			if (parseStep == 6)
			{
				// parse field & enqueue command to stack
				Command cmd(StrmSouce, cmdCode, cmdParam, cmdValue, valueLen);

				Stack.enqueue(cmd);
				
				// begin again
				parseStep = -1;
			}
		}
	}
}

uint32_t CommandParser::toNum(const char * str)
{
	uint32_t value = 0;
	
	while (*str)
	{
		if (*str < '0' && *str < '9')
			break;
		
		value = value * 10 + (*str - '0');
		str++;
	}
	
	return value;
}
