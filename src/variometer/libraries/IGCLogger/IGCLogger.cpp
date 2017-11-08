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


const char * nameManufacture = "NRC";
const char * serialNumber = "STM";
const char * tailOfFileName = "01.igc";

const char * logsFolder = "/TrackLogs";


// ANRCSTM Variometer & GPS Loggger
// HFDTExxxx
// HPPLTPILOT:xxxxxx
// HPCCLCOMPETITION CLASS:FAI-3 (PG)
// HPGTYGLIDERTYPE:xxxxx
// HODTM100GPSDATUM:WGS-84
const char * igcHeader[] =
{
	"ANRCSTM Variometer & GPS Loggger", 
	"\r\nHFDTE",
	NULL,
	"\r\nHPPLTPILOT:",
	NULL,
	"\r\nHPCCLCOMPETITION CLASS:",
	NULL,
	"\r\nHPGTYGLIDERTYPE:",
	NULL,
	"\r\nHODTM100GPSDATUM:",
	NULL,
	"\r\n"
};

#define IGC_HEADER_DATE			(2)
#define IGC_HEADER_PILOT		(4)
#define IGC_HEADER_CLASS		(6)
#define IGC_HEADER_GLIDER		(8)
#define IGC_HEADER_GPSDATUM		(10)


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
		//sdCard.initErrorHalt(F("sdCard:"));
		SET_STATE(LOGGER_INIT_FAILED);
		
		return false;
	}
	
	//
	#if 0 // files is saved on the root : not support sub-folder
	if (! sdCard.exists(logsFolder))
	{
		SET_STATE(LOGGER_INIT_FAILED);
		
		return false;
	}
	
	sd.chdir(logsFolder);
	#endif
	
	return true;
}

int	IGCLogger::begin(uint32_t date)
{
	if (IS_SET(LOGGER_INIT_FAILED))
		return false;
	
	// create new file // YYYY-MM-DD-NRC-STM-nn.igc
	char name[26];
	
	if (! makeFileName(name, date))
	{
		// no valid file room
		SET_STATE(LOGGER_INIT_FAILED);
		return false;
	}

	if (sdFile.open(name, O_CREAT | O_WRITE | O_EXCL))
	{
		//
		SET_STATE(LOGGER_WORKING);
		
		// write header
		writeHeader(date);
		
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
	
	if (digit.available())
		ch = digit.read();
	
	sdFile.write(&ch, 1);
	columnCount += 1;
	
	return 1;
}

void IGCLogger::update(uint32_t varioAlt)
{
	varioAltitude = varioAlt;
}

int IGCLogger::isInitialized()
{
	return IS_SET(LOGGER_INIT_FAILED) ? false : true;
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

const char * IGCLogger::makeFileName(char * buf, uint32_t date)
{
	// name format : YYYY-MM-DD-NRC-STM-nn.igc
	// ...
	int pos = 0;
	int i, num;
	const char * ptr;
	
	// year prefix : 20xx, we can use this code until 2099~~ Hmmm
	buf[pos++] = '2'; buf[pos++] = '0';
	
	// year, month, day : DDMMYY -> YY-MM-DD
	for (i = 0, num = date; i < 3; i++)
	{
		int d = num % 100;
		num = num / 100;
		
		buf[pos++] = (d / 10) + '0';
		buf[pos++] = (d % 10) + '0';
		buf[pos++] = '-';
	}

	// manufacture code
	for (i = 0, ptr = nameManufacture; i < 3; i++)
		buf[pos++] = (*ptr) ? *ptr++ : 'X';
	buf[pos++] = '-';
		
	// FR serial number
	for (i = 0, ptr = serialNumber; i < 3; i++)
		buf[pos++] = (*ptr) ? *ptr++ : 'X';
	buf[pos++] = '-';
	
	// flight number of the day & file extension
	for (ptr = tailOfFileName; *ptr; ptr++)
		buf[pos++] = *ptr;
	buf[pos] = '\0'; // null-terminate
	
	for (i = 2; i < 101; i++) // valid number : 01 ~ 99
	{
		if (! sdCard.exists(buf))
			return buf;
		
		buf[19] = (i / 10) + '0';
		buf[20] = (i % 10) + '0';
	}
	
	return NULL;
}

void IGCLogger::writeHeader(uint32_t date)
{
	if(! IS_SET(LOGGER_WORKING))
		return;

	//
	for (int i = 0; i < sizeof(igcHeader) / sizeof(igcHeader[0]); i++)
	{
		switch (i)
		{
		case IGC_HEADER_DATE	 :
			{
				digit.begin(date, 6);
				
				while (digit.available())
					sdFile.write(digit.read());
			}
			break;
		case IGC_HEADER_PILOT	 :
		case IGC_HEADER_CLASS	 :
		case IGC_HEADER_GLIDER	 :
		case IGC_HEADER_GPSDATUM :
			// leave it empty!!
			break;
		default 				 :
			sdFile.write(igcHeader[i]);
			break;
		}
	}
}
