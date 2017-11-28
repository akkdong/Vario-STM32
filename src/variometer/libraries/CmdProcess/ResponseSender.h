// ResponseSender.h
//

#ifndef __RESPONSESENDER_H__
#define __RESPONSESENDER_H__

#include <DefaultSettings.h>
#include <Arduino.h>


/////////////////////////////////////////////////////////////////////////////
// class ResponseSender

class ResponseSender
{
public:
	ResponseSender();
	
public:	
	int					available();
	int					read();
};


#endif // __RESPONSESENDER_H__