// ResponseStack.cpp
//

#include "ResponseStack.h"




/////////////////////////////////////////////////////////////////////////////
// class Response

Response::Response() : code(0), param(0), dataType(RDATA_INVALID)
{
}

Response::Response(const Response & res) :
	code(res.code), param(res.param), dataType(res.dataType)
{
	memcpy(&data, &res.data, sizeof(data));
	
	//
	#if REPONSE_DEBUG
	dump();
	#endif // REPONSE_DEBUG
}

Response::Response(uint16_t c, uint32_t p) :
	code(c), param(p), dataType(RDATA_INVALID)
{
	memset(&data, 0, sizeof(data));
}

Response Response::operator=(const Response & res)
{
	memcpy(this, &res, sizeof(res));
	
	#if REPONSE_DEBUG
	dump();
	#endif // REPONSE_DEBUG
}

#if REPONSE_DEBUG
void Response::dump()
{
	Serial.print("Response : "); Serial.write((code >> 8) & 0x00FF); Serial.write(code & 0x00FF);
	Serial.println("");

	if (param)
	{
		Serial.print("  param : "); Serial.println(param, HEX);
		
		switch (dataType)
		{
		case RDATA_NUMBER_S8	:
			Serial.print("  s8 : "); Serial.println(data.s8);
			break;
		case RDATA_NUMBER_S16	:
			Serial.print("  s16 : "); Serial.println(data.s16);
			break;
		case RDATA_NUMBER_S32	:
			Serial.print("  s32 : "); Serial.println(data.s32);
			break;
		case RDATA_NUMBER_U8	:
			Serial.print("  u8 : "); Serial.println(data.u8);
			break;
		case RDATA_NUMBER_U16	:
			Serial.print("  u16 : "); Serial.println(data.u16);
			break;
		case RDATA_NUMBER_U32	:
			Serial.print("  u32 : "); Serial.println(data.u32);
			break;
		case RDATA_FLOAT		:
			Serial.print("  f32 : "); Serial.println(data.f32);
			break;
		case RDATA_NARRAY		:
			Serial.print("  nArray : "); 
			for (int i = 0; i < dataCount; i++)
			{
				Serial.print(data.nArray[i]);
				if (i < dataCount - 1)
					Serial.print(",");
			}
			Serial.println("");
			break;
		case RDATA_FARRAY		:
			Serial.print("  fArray : "); 
			for (int i = 0; i < dataCount; i++)
			{
				Serial.print(data.fArray[i]);
				if (i < dataCount - 1)
					Serial.print(",");
			}
			Serial.println("");
			break;
		case RDATA_STRING		:
			Serial.print("  str : ");
			for (int i = 0; i < MAX_RESP_STRING && data.sData[i]; i++)
				Serial.write((char)data.sData[i]);
			Serial.println("");
			break;
		}		
	}	
}
#endif // REPONSE_DEBUG

/////////////////////////////////////////////////////////////////////////////
// class ResponseStack

ResponseStack::ResponseStack() : front(0), rear(0), active(0), column(0)
{
}

// 012345678901234"
// %XX,P,1,2,3,4\r\n"

#define COLUMN_SENTENCE_LEADING			0x00
#define COLUMN_CODE_H_NIBBLE			0x01
#define COLUMN_CODE_L_NIBBLE			0x02
#define COLUMN_PARAM_LEADING			0x03
#define COLUMN_PARAM_POS				0x04
#define COLUMN_DATA_1_POS				0x05
#define COLUMN_DATA_2_POS				0x06
#define COLUMN_DATA_3_POS				0x07
#define COLUMN_DATA_4_POS				0x08
#define COLUMN_CARRAGE_RETURN			0x09
#define COLUMN_NEW_LINE					0x10


int ResponseStack::getNext(int dataNum)
{
	int outc;
		
	if (strIndex < 0)
	{
		//
		if (digit.available())
			return digit.get();
		
		if (dataNum < active->dataCount)
		{
			if (active->dataType == RDATA_NARRAY)
				digit.begin(active->data.nArray[dataNum]);
			else if (active->dataType == RDATA_FARRAY)
				digit.begin(active->data.fArray[dataNum], MAX_FLOAT_PRECISION);
			
			outc = ',';
			column = COLUMN_DATA_1_POS + dataNum;
		}
		else
		{
			outc = '\r';
			column = COLUMN_NEW_LINE;
		}

		return outc;
	}
	else
	{
		if (strIndex < MAX_RESP_STRING && active->data.sData[strIndex])
		{
			outc = active->data.sData[strIndex];
			strIndex += 1;
		}
		else
		{
			outc = '\r';
			column = COLUMN_NEW_LINE;
		}
	}
	
	return outc;
}

int ResponseStack::read()
{
	//
	if (! active)
		return -1;
	
	//
	uint8_t outc = 0;
	
	switch (column)
	{
	case COLUMN_SENTENCE_LEADING	:
		outc = '%';
		column = COLUMN_CODE_H_NIBBLE;
		break;
	case COLUMN_CODE_H_NIBBLE		:
		//outc = toHex((active->code & 0xF0) >> 4);	// high_nibble(code);
		outc = active->code >> 8;
		column = COLUMN_CODE_L_NIBBLE;
		break;
	case COLUMN_CODE_L_NIBBLE		:
		//outc = toHex(active->code &0x0F);			// low_nibble(code);
		outc = active->code & 0x00FF;
		
		if (active->param == 0)
		{
			column = COLUMN_CARRAGE_RETURN;
		}
		else
		{
			digit.begin(active->param);
			column = COLUMN_PARAM_LEADING;
		}
		break;
	case COLUMN_PARAM_LEADING		:
		outc = ',';
		column = COLUMN_PARAM_POS;
		break;
	case COLUMN_PARAM_POS			:
		if (digit.available())
		{
			outc = digit.get();
		}
		else
		{
			if (active->dataType == RDATA_INVALID)
			{
				outc = '\r';
				column = COLUMN_NEW_LINE;
			}
			else
			{
				strIndex = -1;
				
				switch (active->dataType)
				{
				case RDATA_NUMBER_S8	:
					digit.begin((int32_t)active->data.s8);
					break;
				case RDATA_NUMBER_S16	:
					digit.begin((int32_t)active->data.s16);
					break;
				case RDATA_NUMBER_S32	:
					digit.begin(active->data.s32);
					break;
				case RDATA_NUMBER_U8	:
					digit.begin((uint32_t)active->data.u8);
					break;
				case RDATA_NUMBER_U16	:
					digit.begin((uint32_t)active->data.u16);
					break;
				case RDATA_NUMBER_U32	:
					//digit.begin(active->data.u32);
					//break;
				case RDATA_NARRAY		:
					digit.begin(active->data.nArray[0]);
					break;
				case RDATA_FLOAT		:
					//digit.begin(active->data.f32, MAX_FLOAT_PRECISION);
					//break;
				case RDATA_FARRAY		:
					digit.begin(active->data.fArray[0], MAX_FLOAT_PRECISION);
					break;
				case RDATA_STRING		:
					strIndex = 0;
					break;
				}
				
				outc = ',';
				column = COLUMN_DATA_1_POS;
			}
		}
		break;
	case COLUMN_DATA_1_POS			:
		outc = getNext(1);
		break;
	case COLUMN_DATA_2_POS			:
		outc = getNext(2);
		break;
	case COLUMN_DATA_3_POS			:
		outc = getNext(3);
		break;		
	case COLUMN_DATA_4_POS			:
		outc = getNext(4);
		break;

	case COLUMN_CARRAGE_RETURN		:
		outc = '\r';
		column = COLUMN_NEW_LINE;
		break;
	case COLUMN_NEW_LINE			:
		outc = '\n';
		
		active = 0;
		column = 0;
		break;
	}
	
	return outc;
}

int ResponseStack::available()
{
	if (! active)
	{
		if (front != rear)
		{
			active = &stack[rear];
			column = COLUMN_SENTENCE_LEADING;
			
			rear = (rear + 1) % MAX_RESPONSE_COUNT;
		}
	}
	
	return active != 0;
}

int ResponseStack::push(const Response & res)
{
	//
	int next = (front + 1) % MAX_RESPONSE_COUNT;
	
	if (next == rear)
		return -1; // full
	
	stack[front] =  res;
	front = next;
	
	return ((front - rear) & (MAX_RESPONSE_COUNT - 1));
}

int ResponseStack::push(uint16_t code, uint32_t param)
{
	Response res(code, param);
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, int8_t data)
{
	Response res(code, param);

	res.data.s8 = data;
	res.dataType = RDATA_NUMBER_S8;
	res.dataCount = 1;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, int16_t data)
{
	Response res(code, param);

	res.data.s16 = data;
	res.dataType = RDATA_NUMBER_S16;
	res.dataCount = 1;
	
	return push(res);	
}

int ResponseStack::push(uint16_t code, uint32_t param, int32_t data)
{
	Response res(code, param);

	res.data.s32 = data;
	res.dataType = RDATA_NUMBER_S32;
	res.dataCount = 1;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, uint8_t data)
{
	Response res(code, param);

	res.data.u8 = data;
	res.dataType = RDATA_NUMBER_U8;
	res.dataCount = 1;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, uint16_t data)
{
	Response res(code, param);

	res.data.u16 = data;
	res.dataType = RDATA_NUMBER_U16;
	res.dataCount = 1;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, uint32_t data)
{
	Response res(code, param);

	res.data.u32 = data;
	res.dataType = RDATA_NUMBER_U32;
	res.dataCount = 1;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, uint32_t data1, uint32_t data2)
{
	Response res(code, param);

	res.data.nArray[0] = data1;
	res.data.nArray[1] = data2;
	res.dataType = RDATA_NARRAY;
	res.dataCount = 2;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, uint32_t data1, uint32_t data2, uint32_t data3)
{
	Response res(code, param);

	res.data.nArray[0] = data1;
	res.data.nArray[1] = data2;
	res.data.nArray[2] = data3;
	res.dataType = RDATA_NARRAY;
	res.dataCount = 3;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t data4)
{
	Response res(code, param);

	res.data.nArray[0] = data1;
	res.data.nArray[1] = data2;
	res.data.nArray[2] = data3;
	res.data.nArray[3] = data4;
	res.dataType = RDATA_NARRAY;
	res.dataCount = 4;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, float data)
{
	Response res(code, param);

	res.data.f32 = data;
	res.dataType = RDATA_FLOAT;
	res.dataCount = 1;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, float data1, float data2)
{
	Response res(code, param);

	res.data.fArray[0] = data1;
	res.data.fArray[1] = data2;
	res.dataType = RDATA_FARRAY;
	res.dataCount = 2;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, float data1, float data2, float data3)
{
	Response res(code, param);

	res.data.fArray[0] = data1;
	res.data.fArray[1] = data2;
	res.data.fArray[2] = data3;
	res.dataType = RDATA_FARRAY;
	res.dataCount = 3;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, float data1, float data2, float data3, float data4)
{
	Response res(code, param);

	res.data.fArray[0] = data1;
	res.data.fArray[1] = data2;
	res.data.fArray[2] = data3;
	res.data.fArray[3] = data4;
	res.dataType = RDATA_FARRAY;
	res.dataCount = 4;
	
	return push(res);
}

int ResponseStack::push(uint16_t code, uint32_t param, char * str)
{
	Response res(code, param);

	int n = strlen(str);
	if (n > MAX_RESP_STRING)
		n = MAX_RESP_STRING;
	
	memset(res.data.sData, 0, n);
	memcpy(res.data.sData, str, n);
	res.dataType = RDATA_STRING;
	res.dataCount = 1;
	
	return push(res);
}
