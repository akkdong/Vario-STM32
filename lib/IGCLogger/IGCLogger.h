// IGCLogger.h
//

#ifndef __IGCLOGGER_H__
#define __IGCLOGGER_H__

#include <DefaultSettings.h>
#include <FixedLenDigit.h>
#include <SdFat.h>
#include <FreeStack.h>
#include <time.h>


/////////////////////////////////////////////////////////////////////////////
// class IGCLogger

class IGCLogger
{
public:
	IGCLogger();
	
public:
	int					init();
	
	#if 0
	int					begin(uint32_t date); // date = DDMMYY
	#endif
	int					begin(time_t date); // date = seconds since 1900
	void				end(time_t date);
	
	int					write(uint8_t ch);

	void				update(float varioAlt);
	
	int					isInitialized();
	int					isLogging();

private:
	void				reset();
	
	#if 0
	const char *		makeFileName(char * buf, uint32_t date);
	#endif
	const char *		makeFileName(char * buf, time_t date);
	#if 0
	void				writeHeader(uint32_t date);
	#endif
	void				writeHeader(time_t date);

private:
	SdFat				sdCard;
	SdFile				sdFile;
	
	uint8_t				logState;
	int					columnCount;
	
	float				varioAltitude;
	FixedLenDigit		digit;
};

#endif // __IGCLOGGER_H__
