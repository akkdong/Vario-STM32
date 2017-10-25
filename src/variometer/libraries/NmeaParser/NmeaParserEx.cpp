// NmeaParserEx.cpp
//

#include "NmeaParserEx.h"

/////////////////////////////////////////////////////////////////////////////
//

#define NMEA_TAG_SIZE 		(5)

#define CLEAR_STATE()		mParseState = 0;
#define SET_STATE(bit)		mParseState |= (1 << bit)
#define UNSET_STATE(bit)	mParseState &= ~(1 << bit)

#define IS_SET(bit)			(mParseState & (1 << bit))


#define SEARCH_RMC_TAG		1
#define SEARCH_GGA_TAG		2
//#define PARSE_RMC			3
//#define PARSE_GGA			4

#define RMC_INVALID			5
#define GGA_INVALID			6

/////////////////////////////////////////////////////////////////////////////
//

const char tagRMC[] PROGMEM = {"GPRMC"};
const char tagGGA[] PROGMEM = {"GPGGA"};


/////////////////////////////////////////////////////////////////////////////
// class NmeaParserEx

NmeaParserEx::NmeaParserEx(Stream & stm) : mStream(stm)
{
	//
	reset();
	
	// initialize un-chagned characters
	memset(mIGCSentence, '0', sizeof(mIGCSentence));
	
	mIGCSentence[IGC_OFFSET_START] = 'B';
	mIGCSentence[IGC_OFFSET_VALIDITY] = 'A';
	mIGCSentence[IGC_OFFSET_RETURN] = '\r';
	mIGCSentence[IGC_OFFSET_NEWLINE] = '\n';
	mIGCSentence[IGC_OFFSET_TERMINATE] = '\0';
}

void NmeaParserEx::update()
{
	while (mStream.available())
	{
		//
		int c = mStream.read();
		
		if (isFull())
			break;
		
		if (mParseStep < 0 && c != '$')
		{
			//Serial.print("* "); Serial.write(c); Serial.println("");
			continue; // skil bad characters(find first sentence character)
		}
		
		//if (mParseStep < 0)
		//	Serial.print("+ "); Serial.write(c); Serial.println("");
		
		mBuffer[mWrite] = c;
		mWrite = (mWrite + 1) % MAX_PARSER_BUFFER;

		//
		if (c == '$')
		{
			mParseStep 	= 0;
			mParseState	= 0;
			mParity		= '*';	// '*' remove by twice xor
			
			SET_STATE(SEARCH_RMC_TAG);
			SET_STATE(SEARCH_GGA_TAG);
			
			//Serial.println("start sentence");
		}
		else
		{
			// $XXXXX,...*CCrn
			//  01234566667891   : parse step
			
			// calculate parity : before checksum(include '*')
			if (mParseStep <= NMEA_TAG_SIZE + 1)
				mParity ^= c;

			if (mParseStep < NMEA_TAG_SIZE) // 0 ~ 4, sentence identifier
			{
				if (c != pgm_read_byte_near(tagRMC + mParseStep))
					UNSET_STATE(SEARCH_RMC_TAG);
				if (c != pgm_read_byte_near(tagGGA + mParseStep))
					UNSET_STATE(SEARCH_GGA_TAG);
				
				mParseStep += 1;
				//Serial.println("search tag");
			}
			else if (mParseStep == NMEA_TAG_SIZE) // start of data
			{
				if (c != ',' || (! IS_SET(SEARCH_RMC_TAG) && ! IS_SET(SEARCH_GGA_TAG)))
				{
					// bad sentence : reset parsing state
					//Serial.println("[unknown tag");
					//for(int i = mHead; i != mWrite; )
					//{
					//	Serial.write(mBuffer[i]);
					//	i = (i + 1) % MAX_PARSER_BUFFER;
					//}
					//Serial.println("]");
					
					mParseStep = -1;
					mWrite = mHead;
				}
				else
				{
					mFieldStart = mWrite;
					mFieldIndex = 0;
					
					mParseStep += 1;
					//Serial.println("start data field");
					
					// new entry : set to invalid 
					if (IS_SET(SEARCH_RMC_TAG))
						SET_STATE(RMC_INVALID);
					if (IS_SET(SEARCH_GGA_TAG))
						SET_STATE(GGA_INVALID);
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE + 1) // data
			{
				if (c == ',' || c == '*') // field delimiter
				{
					// parse field
					parseField(mFieldIndex, mFieldStart);
					
					mFieldStart = mWrite;
					mFieldIndex = mFieldIndex + 1;
				}
				
				if (c == '*') // start of checksum
				{
					mParseStep += 1;
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE + 2) // checksum high-nibble
			{
				int n = (c >= 'A') ? (c - 'A' + 10) : (c - '0');
				
				if (n != ((mParity & 0xF0) >> 4)) // bad checksum
				{
					mParseStep = -1;
					mWrite = mHead;
				}
				else
				{
					mParseStep += 1;
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE + 3) // checksum low-nibble
			{
				int n = (c >= 'A') ? (c - 'A' + 10) : (c - '0');
				
				if (n != (mParity & 0x0F)) // bad checksum
				{
					mParseStep = -1;
					mWrite = mHead;
				}
				else
				{
					mParseStep += 1;
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE + 4) // carrage-return
			{
				if (c != '\r')
				{
					mParseStep = -1;
					mWrite = mHead;
				}
				else
				{
					mParseStep += 1;
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE + 5) // newline
			{
				if (c != '\n')
				{
					mParseStep = -1;
					mWrite = mHead;
				}
				else
				{
					// complete a sentence
					mParseStep = -1;
					mHead = mWrite;
					
					//
					if (IS_SET(SEARCH_GGA_TAG) && ! IS_SET(GGA_INVALID))
					{
						// IGC sentence become available
						mIGCSize = MAX_IGC_SENTENCE;
						mIGCNext = 0;
					}
				}
			}			
		}
	}
}

int NmeaParserEx::available()
{
	return (! isEmpty());
}

int NmeaParserEx::read()
{
	if (isEmpty())
		return -1;
	
	int c = mBuffer[mTail];
	mTail = (mTail + 1) % MAX_PARSER_BUFFER;
	
	return c;
}

int NmeaParserEx::availableIGC()
{
	return (mIGCSize == MAX_IGC_SENTENCE);
}

int NmeaParserEx::readIGC()
{
	if (mIGCSize == MAX_IGC_SENTENCE && mIGCNext < MAX_IGC_SENTENCE)
	{
		int ch = mIGCSentence[mIGCNext++];
		
		if (mIGCNext == MAX_IGC_SENTENCE) // end of sentence
		{
			// empty
			mIGCSize = 0;
			mIGCNext = 0; 
		}
		
		return ch;
	}
	
	return -1;
}

void NmeaParserEx::reset()
{
	mHead = mTail = mWrite = 0;
	
	mDate = 0;
	mTime = 0;
	mLatitude = 0.0;
	mLongitude = 0.0;	
	mSpeed = 0;
	mAltitude = 0;
	
	mParseStep = -1;
	mParseState = 0;
	
	SET_STATE(RMC_INVALID);
	SET_STATE(GGA_INVALID);
	
	mIGCSize = 0;
	mIGCNext = 0;
}

void NmeaParserEx::parseField(int fieldIndex, int startPos)
{
	if (IS_SET(SEARCH_RMC_TAG))
	{
		switch(fieldIndex)
		{
		case 0 : // Time (HHMMSS.sss UTC)
			/*
			// update IGC sentence
			for(int i = 0; i < IGC_SIZE_TIME; i++)
			{
				if ('0' > mBuffer[startPos+i] || mBuffer[startPos+i] > '9')
					break;
				
				mIGCSentence[IGC_OFFSET_TIME+i] = mBuffer[startPos+i];
			}
			
			// save current time
			// ...
			*/
			break;
		case 1 : // Navigation receiver warning A = OK, V = warning
			if (mBuffer[startPos] == 'A')
				UNSET_STATE(RMC_INVALID);
			break;
		case 2 : // Latitude (DDMM.mmm)
			/*
			// update IGC sentence
			for(int i = 0, j = 0; i < IGC_SIZE_LATITUDE; i++, j++)
			{
				if ('0' <= mBuffer[startPos+i] && mBuffer[startPos+i] <= '9')
					mIGCSentence[IGC_OFFSET_LATITUDE+i] = mBuffer[startPos+j];
				else if (mBuffer[startPos+i] == '.')
					i -= 1;
				else
					break;
			}

			// save latitude
			// ...
			*/
			break;
		case 3 : // Latitude (N or S)
			/*
			// update IGC sentence
			if (mBuffer[startPos] != 'N' && mBuffer[startPos] != 'S')
					break;
			mIGCSentence[IGC_OFFSET_LATITUDE_] = mBuffer[startPos];
			
			// save latitude
			// ...
			*/
			break;
		case 4 : // Longitude (DDDMM.mmm)
			/*
			// update IGC sentence
			for(int i = 0, j = 0; i < IGC_SIZE_LONGITUDE; i++, j++)
			{
				if ('0' <= mBuffer[startPos+i] && mBuffer[startPos+i] <= '9')
					mIGCSentence[IGC_OFFSET_LONGITUDE+i] = mBuffer[startPos+j];
				else if (mBuffer[startPos+i] == '.')
					i -= 1;
				else
					break;
			}
			
			// save latitude
			// ...
			*/
			break;
		case 5 : // Longitude (E or W)
			/*
			// update IGC sentence
			if (mBuffer[startPos] != 'W' && mBuffer[startPos] != 'E')
					break;
			mIGCSentence[IGC_OFFSET_LONGITUDE_] = mBuffer[startPos];
			
			// save latitude
			// ...
			*/
			break;
		case 6 : // Speed over ground, Knots
			//Serial.print("Speed : ");
			//Serial.print(strToFloat(startPos) * 1.852);
			//Serial.println(" Km/h");
			break;
		case 7 : // Course Made Good, True
			break;
		case 8 : // Date of fix  (DDMMYY)
			//Serial.print("Date : ");
			//Serial.print(strToNum(startPos));
			//Serial.println("");
			break;
		}
	}
	else if (IS_SET(SEARCH_GGA_TAG))
	{
		switch(fieldIndex)
		{
		case 0 : // Time (HHMMSS.sss UTC)
			// update IGC sentence
			for(int i = 0; i < IGC_SIZE_TIME; i++)
			{
				if ('0' > mBuffer[startPos+i] || mBuffer[startPos+i] > '9')
					break;
				
				mIGCSentence[IGC_OFFSET_TIME+i] = mBuffer[startPos+i];
			}
			
			// save current time
			// ...
			break;
		case 1 : // Latitude (DDMM.mmm)
			// update IGC sentence
			for(int i = 0, j = 0; i < IGC_SIZE_LATITUDE; i++, j++)
			{
				if ('0' <= mBuffer[startPos+i] && mBuffer[startPos+i] <= '9')
					mIGCSentence[IGC_OFFSET_LATITUDE+i] = mBuffer[startPos+j];
				else if (mBuffer[startPos+i] == '.')
					i -= 1;
				else
					break;
			}

			// save latitude
			// ...
			break;
		case 2 : // Latitude (N or S)
			// update IGC sentence
			if (mBuffer[startPos] != 'N' && mBuffer[startPos] != 'S')
					break;
			mIGCSentence[IGC_OFFSET_LATITUDE_] = mBuffer[startPos];
			
			// save latitude
			// ...
			break;
		case 3 : // Longitude (DDDMM.mmm)
			// update IGC sentence
			for(int i = 0, j = 0; i < IGC_SIZE_LONGITUDE; i++, j++)
			{
				if ('0' <= mBuffer[startPos+i] && mBuffer[startPos+i] <= '9')
					mIGCSentence[IGC_OFFSET_LONGITUDE+i] = mBuffer[startPos+j];
				else if (mBuffer[startPos+i] == '.')
					i -= 1;
				else
					break;
			}
			
			// save latitude
			// ...
			break;
		case 4 : // Longitude (E or W)
			// update IGC sentence
			if (mBuffer[startPos] != 'W' && mBuffer[startPos] != 'E')
					break;
			mIGCSentence[IGC_OFFSET_LONGITUDE_] = mBuffer[startPos];
			
			// save latitude
			// ...
			break;
		case 5 : // GPS Fix Quality (0 = Invalid, 1 = GPS fix, 2 = DGPS fix)
			if (mBuffer[startPos] == '1' || mBuffer[startPos] == '2')
				UNSET_STATE(GGA_INVALID);
			break;
		case 6 : // Number of Satellites
			//Serial.print("Satellites : ");
			//Serial.print(strToNum(startPos));
			//Serial.println("");
			break;
		case 7 : // Horizontal Dilution of Precision
			break;
		case 8 : // Altitude(above measn sea level)
			// update IGC sentence
			{
				int i, j;
				
				for(i = 0; i < IGC_SIZE_GPS_ALT; i++)
				{
					if (! ('0' <= mBuffer[startPos+i] && mBuffer[startPos+i] <= '9') && mBuffer[startPos+i] != '-')
						break;
					
					mIGCSentence[IGC_OFFSET_GPS_ALT+i] = mBuffer[startPos+i];
				}
				
				for (j = IGC_SIZE_GPS_ALT - 1, i -= 1; i >= 0; i--, j--)
					mIGCSentence[IGC_OFFSET_GPS_ALT+j] = mIGCSentence[IGC_OFFSET_GPS_ALT+i];
				for ( ; j >= 0; j--)
					mIGCSentence[IGC_OFFSET_GPS_ALT+j] =  '0';
			}
			
			// save current time
			// ...
			break;
		case 9 : // Altitude unit (M: meter)
			break;
		}
	}
}

double NmeaParserEx::strToFloat(int startPos)
{
	double value = 0.0, div = 0;
	
	for (int i = startPos; ;)
	{
		if (mBuffer[i] == '.')
		{
			div = 1;
		}
		else if ('0' <= mBuffer[i] && mBuffer[i] <= '9')
		{
			value *= 10;
			value += mBuffer[i] - '0';

			if (div)
				div *= 10;
		}
		else
		{
			// end of converting
			break;
		}
		
		i = (i + 1) % MAX_PARSER_BUFFER;
	}
	
	if (div)
		value /= div;
	
	return value;
}

long NmeaParserEx::strToNum(int startPos)
{
	long value = 0;
	
	for (int i = startPos; ;)
	{
		if ('0' <= mBuffer[i] && mBuffer[i] <= '9')
		{
			value *= 10;
			value += mBuffer[i] - '0';
		}
		else
		{
			// end of converting
			break;
		}
		
		i = (i + 1) % MAX_PARSER_BUFFER;
	}
	
	return value;
}
