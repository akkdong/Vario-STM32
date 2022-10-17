// ResponseStack.h
//

#ifndef __RESPONSESTACK_H__
#define __RESPONSESTACK_H__

#include <Arduino.h>
#include <DefaultSettings.h>
#include <Digit.h>

#define MAX_RESP_STRING			16
#define MAX_RESPONSE_COUNT		64

#define MAX_FLOAT_PRECISION		4

#define REPONSE_DEBUG			0


//
// response format
//
//     %{CODE}[,{PARAM}[,{VALUE1}[,{VALUE2}[,{VALUE3}[,{VALUE4}]]]]]\r\n
//     %{CODE}[,{PARAM}[,{STRING}]]\r\n
//

// response code --> command code
/*
#define RCODE_RESULT			'RS'
#define RCODE_OK				'OK'
#define RCODE_FAIL				'FA'
#define RCODE_ERROR				'ER'
#define RCODE_NOT_READY			'NR'
#define RCODE_UNAVAILABLE		'UA'
#define RCODE_DUMP_PARAM		'DP'
#define RCODE_QUERY_PARAM		'QP'
#define RCODE_UPDATE_PARAM		'UP'
*/

// PARAM : use in range 0x0000 ~ 0xFFFF, 0xFFFFFFFF(-1) means unused param
//     -> result of command processing              : 0x0000 ~ 0x0100
//     -> accelerometer calibration result/status   : 0x1000 ~ 0x1FFF
//     -> mode switch return                        : 0x2000 ~ 0x2FFF
//     -> id of property                            : 0x9000 ~ 0xFFFF
#define RPARAM_SUCCESS				(0)
#define RPARAM_OK					(0)

#define RPARAM_FAIL					(1)
#define RPARAM_INVALID_COMMAND		(2)
#define RPARAM_INVALID_PROPERTY		(3)
#define RPARAM_NOT_READY			(4)
#define RPARAM_UNAVAILABLE			(5)
#define RPARAM_NOT_ALLOWED			(6)
#define RPARAM_INVALID_PARAMETER	(7)
#define RPARAM_INVALID_DATA			(8)

#define RPARAM_CAL_START			(0x1000)
#define RPARAM_CAL_MODE_CHANGED		(0x1001)	// INIT->READY->MEASURE->CALIBATE->DONE or STOP
#define RPARAM_CAL_MEASURED_RESULT	(0x1003)	// validation, orient, accel standard deviation
#define RPARAM_CAL_DONE				(0x1002)	// calibration accel x/y/z
#define RPARAM_CAL_ACCELEROMETER 	(0x1004)	// calibrated accel x/y/z

#define RPARAM_SW_BASE				(0x2000)
#define RPARAM_SW_VARIO				(0x2001)
#define RPARAM_SW_UMS				(0x2002)
#define RPARAM_SW_CALIBRATION		(0x2003)

#define RPARAM_BT_BAUDRATE			(0x3001)
#define RPARAM_BT_NAME				(0x3002)
#define RPARAM_BT_KEY				(0x3003)

#define RPARAM_CAL_STATE			()	// MODE, MEASURE SIDE, ...


enum RDATA_TYPE
{
	RDATA_INVALID,
	RDATA_NUMBER_S8,
	RDATA_NUMBER_S16,
	RDATA_NUMBER_S32,
	RDATA_NUMBER_U8,
	RDATA_NUMBER_U16,
	RDATA_NUMBER_U32,
	RDATA_FLOAT,
	RDATA_NARRAY,
	RDATA_FARRAY,
	RDATA_STRING,
};


/////////////////////////////////////////////////////////////////////////////
//

class Response
{
public:
	Response();
	Response(const Response & res);
	Response(uint16_t code, uint32_t param = -1);

public:
	Response operator=(const Response & res);
	
private:
	#if REPONSE_DEBUG
	void		dump();
	#endif // REPONSE_DEBUG
	
public:
//	uint16_t	dest;		// this response is transmitted to BT or USB
	uint16_t	code;		// response code
	uint8_t 	dataType;	// response data type
	uint8_t		dataCount;	
	uint32_t	param;		// response specific paramater
	
	union tagData
	{
		int8_t		s8;
		int16_t		s16;
		int32_t		s32;
		uint8_t		u8;
		uint16_t	u16;
		uint32_t	u32;
		float		f32;
		uint32_t	nArray[4];
		float		fArray[4];
		char		sData[MAX_RESP_STRING];
	} data;
};



/////////////////////////////////////////////////////////////////////////////
// class ResponseStack

class ResponseStack
{
public:
	ResponseStack();
	
public:	
	int					available();
	int					read();

	int					push(const Response & res);
	int					push(uint16_t code, uint32_t param = 0);
	int					push(uint16_t code, uint32_t param, int8_t data);
	int					push(uint16_t code, uint32_t param, int16_t data);
	int					push(uint16_t code, uint32_t param, int32_t data);
	int					push(uint16_t code, uint32_t param, uint8_t data);
	int					push(uint16_t code, uint32_t param, uint16_t data);
	int					push(uint16_t code, uint32_t param, uint32_t data);
	int					push(uint16_t code, uint32_t param, uint32_t data1, uint32_t data2);
	int					push(uint16_t code, uint32_t param, uint32_t data1, uint32_t data2, uint32_t data3);
	int					push(uint16_t code, uint32_t param, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t data4);
	int					push(uint16_t code, uint32_t param, float data);
	int					push(uint16_t code, uint32_t param, float data1, float data2);
	int					push(uint16_t code, uint32_t param, float data1, float data2, float data3);
	int					push(uint16_t code, uint32_t param, float data1, float data2, float data3, float data4);
	int					push(uint16_t code, uint32_t param, char * str);

	boolean				isFull();
	boolean				isEmpty();
	
private:
	int					getNext(int dataNum);
		
private:
	//
	Response			stack[MAX_RESPONSE_COUNT];
	int					front;
	int					rear;
	
	//
	Response *			active;
	int					column;
	
	//
	Digit				digit;
	int					strIndex;
	
	//
	//static const char *	tags;
};

// inline member functions

inline boolean ResponseStack::isFull()
	{ return (((front + 1) % MAX_RESPONSE_COUNT) == rear); }
	
inline boolean ResponseStack::isEmpty()
	{ return (front == rear); }


#endif // __RESPONSESTACK_H__