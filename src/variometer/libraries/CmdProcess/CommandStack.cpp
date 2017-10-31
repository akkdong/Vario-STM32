// CommandStack.cpp
//

#include <Arduino.h>
#include "CommandStack.h"


/////////////////////////////////////////////////////////////////////////////
// class CommandStack

CommandStack::CommandStack()
{
}
	
void CommandStack::enqueue(Command cmd)
{
}

Command CommandStack::dequeue()
{
	Command cmd;
	
	return cmd;
}

int8_t CommandStack::getSize()
{
	return 0;
}

int8_t CommandStack::isEmpty()
{
	return 0;
}

int8_t CommandStack::isFull()
{
	return 0;
}
