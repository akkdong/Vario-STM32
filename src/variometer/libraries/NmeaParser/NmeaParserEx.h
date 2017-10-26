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
#define IGC_OFFSET_LATITUDE		(7)
#define IGC_OFFSET_LATITUDE_	(14)
#define IGC_OFFSET_LONGITUDE	(15)
#define IGC_OFFSET_LONGITUDE_	(23)
#define IGC_OFFSET_VALIDITY		(24)
#define IGC_OFFSET_PRESS_ALT	(25)
#define IGC_OFFSET_GPS_ALT		(30)
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
	
	//
	uint32_t			getDate();  	// DDMMYY
	uint32_t			getTime();		// HHMMSS
	double				getLatitude();
	double				getLongitude();
	uint32_t			getAltitude();
	uint32_t			getSpeed();
	uint32_t			getHeading();
	
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
	uint32_t			mHeading;
	
	// IGC sentence
	char				mIGCSentence[MAX_IGC_SENTENCE];
	int					mIGCNext;	// next = 0 ~ MAX_XXX -1 -> available
	int					mIGCSize;	// size = 0 -> empty, size = MAX_xx -> valid
	
};


// inline functions
//

inline uint32_t NmeaParserEx::getDate()
	{ return mDate; }
	
inline uint32_t	NmeaParserEx::getTime()
	{ return mTime; }
	
inline double NmeaParserEx::getLatitude()
	{ return mLatitude; }
	
inline double NmeaParserEx::getLongitude()
	{ return mLongitude; }
	
inline uint32_t NmeaParserEx::getAltitude()
	{ return mAltitude; }
	
inline uint32_t NmeaParserEx::getSpeed()
	{ return mSpeed; }
	
inline uint32_t NmeaParserEx::getHeading()
	{ return mHeading; }
	

#endif // __NMEAPARSEREX_H__
