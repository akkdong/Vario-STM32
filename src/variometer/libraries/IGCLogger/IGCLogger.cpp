// IGCLogger.cpp
//

#include <Arduino.h>
#include <GlobalConfig.h>
#include <NmeaParserEx.h>

#include "IGCLogger.h"

#define CLEAR_STATE()		logState = 0
#define SET_STATE(bit)		logState |= (bit)
#define UNSET_STATE(bit)	logState &= ~(bit)

#define IS_SET(bit)			(logState & (bit)) == (bit)


#define LOGGER_INIT_FAILED		(1 << 0)
#define LOGGER_WORKING			(1 << 1)



/////////////////////////////////////////////////////////////////////////////
// class IGCLogger

IGCLogger::IGCLogger() : sdCard(SDCARD_CHANNEL)
{
	reset();
}


int IGCLogger::init()
{
	//
	reset();
	
	//
	if (! sdCard.begin(SDCARD_CS, SD_SCK_MHZ(SDCARD_CLOCK)))
	{
		SET_STATE(LOGGER_INIT_FAILED);
		
		return false;
	}
	
	//
	#if 0 // files is saved on the root : not support sub-folder
	if (! sdCard.exists("/logs"))
	{
		SET_STATE(LOGGER_INIT_FAILED);
		
		return false;
	}
	
	sd.chdir("/logs");
	#endif
	
	return true;
}

int	IGCLogger::begin(uint32_t date)
{
	if (IS_SET(LOGGER_INIT_FAILED))
		return false;
	
	// create new file 
	// YYYY-MM-DD-STM-AKK-nn.igc
	char * name = "YYYY-MM-DD-STM-AKK-nn.igc";
	
	if (sdFile.open(name, O_WRITE | O_CREAT | O_TRUNC))
	{
		//
		SET_STATE(LOGGER_WORKING);
		
		// write header
		// ....
		
		return true;
	}
	
	return false;
}

void IGCLogger::end()
{
	if (! IS_SET(LOGGER_WORKING))
		return;
	
	UNSET_STATE(LOGGER_WORKING);
	sdFile.close();
}

int IGCLogger::write(uint8_t ch)
{
	if (! IS_SET(LOGGER_WORKING))
		return 0;
	
	if (ch == 'B')
		columnCount = 0;
	
	// pressure altitude field is replaced by measured pressure altitude : NmeaParser returns null value
	if (columnCount == IGC_OFFSET_PRESS_ALT)
		digit.begin(varioAltitude, IGC_SIZE_PRESS_ALT);

	if (digit.available() )
		ch = digit.read();

	sdFile.write(&ch, 1);
	columnCount += 1;
	
	return 1;
}

void IGCLogger::update(uint32_t varioAlt)
{
	varioAltitude = varioAlt;
}

int IGCLogger::isLogging()
{
	return IS_SET(LOGGER_WORKING) ? true : false;
}

void IGCLogger::reset()
{
	logState		= 0;
	columnCount 	= -1;
	varioAltitude 	= 0;
}
