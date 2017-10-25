// NmeaParserEx.h
//

#ifndef __NMEAPARSEREX_H__
#define __NMEAPARSEREX_H__

#include <Arduino.h>
#include <Stream.h>

/////////////////////////////////////////////////////////////////////////////
//

#define MAX_PARSER_BUFFER		(128)

#define MAX_IGC_SENTENCE		(38)		// B-sentence max size : include null termination
											// ex: B1602405407121N00249342WA0028000421 CR LF
											//     BHHMMSSDDMMmmmNDDDMMmmmWAPPPPPGGGGGCL
											//                   S        EV          RF

#define IGC_OFFSET_START		(0)
#define IGC_OFFSET_TIME			(1)
#define IGC_OFFSET_LATITUDE		(8)
#define IGC_OFFSET_LATITUDE_	(15)
#define IGC_OFFSET_LONGITUDE	(16)
#define IGC_OFFSET_LONGITUDE_	(24)
#define IGC_OFFSET_VALIDITY		(25)
#define IGC_OFFSET_PRESS_ALT	(26)
#define IGC_OFFSET_GPS_ALT		(31)
#define IGC_OFFSET_RETURN		(35)
#define IGC_OFFSET_NEWLINE		(36)
#define IGC_OFFSET_TERMINATE	(37)

#define IGC_SIZE_TIME			(6)
#define IGC_SIZE_LATITUDE		(7)
#define IGC_SIZE_LATITUDE_		(1)
#define IGC_SIZE_LONGITUDE		(8)
#define IGC_SIZE_LONGITUDE_		(1)
#define IGC_SIZE_VALIDITY		(1)
#define IGC_SIZE_PRESS_ALT		(5)
#define IGC_SIZE_GPS_ALT		(5)


/////////////////////////////////////////////////////////////////////////////
// class NmeaParserEx

class NmeaParserEx
{
public:
	NmeaParserEx(Stream & stm);
	
public:
	void				update();
	
	int					available();
	int					read();
	
	int					availableIGC();
	int					readIGC();
	
	void				reset();	
	
private:
	int					isFull() 	{ return ((mWrite + 1) % MAX_PARSER_BUFFER) == mTail; }
	int					isEmpty()	{ return mHead == mTail; }
	
	void				parseField(int fieldIndex, int startPos);
	
	double				strToFloat(int startPos);
	long				strToNum(int startPos);
	
private:
	//
	Stream &			mStream;
	char				mBuffer[MAX_PARSER_BUFFER];
	volatile int		mHead;
	volatile int		mTail;
	volatile int		mWrite;
	volatile int		mFieldStart;
	volatile int		mFieldIndex;
	
	//
	volatile char		mParseStep; 		// -1 or 0 ~ 12
	volatile uint8_t	mParseState;		// 
	volatile uint8_t	mParity;
	
	//
	uint32_t			mDate;
	uint32_t			mTime;
	double				mLatitude;
	double				mLongitude;
	uint32_t			mAltitude;
	uint32_t			mSpeed;
	
	// IGC sentence
	char				mIGCSentence[MAX_IGC_SENTENCE];
	int					mIGCNext;	// next = 0 ~ MAX_XXX -1 -> available
	int					mIGCSize;	// size = 0 -> empty, size = MAX_xx -> valid
	
};

#endif // __NMEAPARSEREX_H__
