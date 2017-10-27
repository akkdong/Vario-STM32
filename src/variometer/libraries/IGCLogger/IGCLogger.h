// IGCLogger.h
//

#ifndef __IGCLOGGER_H__
#define __IGCLOGGER_H__

#include <FixedLenDigit.h>
#include <SdFat.h>
#include <FreeStack.h>

#define SDCARD_CHANNEL			(2)
#define SDCARD_CS				(PB12) // (PA4)
#define SDCARD_CLOCK			(18)


/////////////////////////////////////////////////////////////////////////////
// class IGCLogger

class IGCLogger
{
public:
	IGCLogger();
	
public:
	int					init();
	
	int					begin(uint32_t date); // date = DDMMYY
	void				end();
	
	int					write(uint8_t ch);

	void				update(uint32_t varioAlt);
	
	int					isLogging();

private:
	void				reset();
	
	const char *		makeFileName(char * buf, uint32_t date);
	void				writeHeader(uint32_t date);

private:
	SdFat				sdCard;
	SdFile				sdFile;
	
	uint8_t				logState;
	uint8_t				columnCount;
	
	uint32_t			varioAltitude;
	FixedLenDigit		digit;
};

#endif // __IGCLOGGER_H__
