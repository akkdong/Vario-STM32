// CommandStack.cpp
//

#include <Arduino.h>
#include "CommandStack.h"


/////////////////////////////////////////////////////////////////////////////
// class CommandStack

CommandStack::CommandStack() : front(0), rear(0)
{
}
	
void CommandStack::enqueue(Command cmd)
{
	if (isFull())
		return;
	
	fifo[front] = cmd;
	front = (front + 1) % MAX_STACKSIZE;
	Serial.print("enqueue : front = "); Serial.print(front);
	Serial.print(", rear = "); Serial.print(rear);
}

Command CommandStack::dequeue()
{
	Command cmd;
	
	if (! isEmpty())
	{
		cmd = fifo[rear];
		rear = (rear + 1) % MAX_STACKSIZE;
		
		Serial.print("dequeue : front = "); Serial.print(front);
		Serial.print(", rear = "); Serial.print(rear);		
	}
	
	return cmd;
}

int8_t CommandStack::getSize()
{
	return (rear <= front) ?  front - rear : (MAX_STACKSIZE - rear) + front;
}

int8_t CommandStack::isEmpty()
{
	return (front == rear);
}

int8_t CommandStack::isFull()
{
	return (((front + 1) % MAX_STACKSIZE) == rear);
}
