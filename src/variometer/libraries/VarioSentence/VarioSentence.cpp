// VarioSentence.cpp
//

#include <Arduino.h>

#include "VarioSentence.h"
//#include "LxNavSentence.h"
//#include "Lk8Sentence.h"


/////////////////////////////////////////////////////////////////////////////
// class VarioSentence

VarioSentence::VarioSentence(char type) : sentenceType(type)
{
}
	
void VarioSentence::update(float v, float h, float t)
{
	// never update if avaiable sentence exists
	if (available())
		return;
	
	// ...
}
	
int VarioSentence::available()
{
	return 0;
}
	
int VarioSentence::read()
{
	return -1;
}