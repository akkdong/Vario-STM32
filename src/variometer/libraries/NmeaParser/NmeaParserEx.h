// NmeaParserEx.h
//

#ifndef __NMEAPARSEREX_H__
#define __NMEAPARSEREX_H__

#include <Arduino.h>
#include <Stream.h>

/////////////////////////////////////////////////////////////////////////////
//

#define MAX_PARSER_BUFFER		(128)


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
};

#endif // __NMEAPARSEREX_H__
