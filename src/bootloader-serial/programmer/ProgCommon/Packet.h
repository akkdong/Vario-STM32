#pragma once

#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>


/////////////////////////////////////////////////////////////////////////////////////////
//

#define MAX_PAYLOAD_SIZE		(1024+8)
#define MAX_DATA_SIZE			(1024)

// Host to Device command
#define HCODE_IDENTIFY			0x55
#define HCODE_ERASE				0x10
#define HCODE_ERASE_ALL			0x11
#define HCODE_WRITE				0x21
#define HCODE_READ				0x22
#define HCODE_RESET				0x80
#define HCODE_START				0x81

// Device to Host response
#define DCODE_IDENTIFY			0xAA
#define DCODE_DUMP_MEM			0xB2
#define DCODE_ACK				0xE0
#define DCODE_NACK				0xE1

#define PACKET_SOF				0xFE


#define ERROR_OK				(0)
#define ERROR_GENERIC			(0x8000)
#define ERROR_OUT_OF_MEMORY		(0x8001)
#define ERROR_FLASH_BUSY		(0x8002)
#define ERROR_FLASH_ERROR_PG	(0x8003)
#define ERROR_FLASH_ERROR_WRP	(0x8004)
#define ERROR_FLASH_TIMEOUT		(0x8005)

#define ERROR_MEMORY_COLLAPSE	(0xC001)
#define ERROR_RESPONSE_TIMEOUT	(0xC002)
#define ERROR_INVALID_PARAM		(0xC003)


/////////////////////////////////////////////////////////////////////////////////////////
//

class BPacketMaker
{
public:
	BPacketMaker();

public:
	int			push_u8(uint8_t data);
	int			push_u16(uint16_t data);
	int			push_u32(uint32_t data);
	int			push_mem(uint8_t * data, uint16_t len);

	void		start(uint8_t code);
	int			finish();

	//
	uint8_t *	get_data() { return &dataBuf[0]; }
	uint16_t	get_size() { return dataLen; }

private:
	uint8_t		dataBuf[MAX_PAYLOAD_SIZE + 1 + 1 + 2 + 1 + 4]; // + sof + code + len + chk + extra
	uint16_t	dataLen;

	uint8_t		checkSum;
};


/////////////////////////////////////////////////////////////////////////////////////////
//

#pragma pack(push, 1)

typedef struct _PAYLOAD_IDENTIFY
{
	uint16_t	devId;
	uint16_t	fwVer;
} PAYLOAD_IDENTIFY;

typedef struct _PAYLOAD_ADDRESS
{
	uint32_t	addr;
} PAYLOAD_ADDRESS;

typedef struct _PAYLOAD_ERROR
{
	uint16_t	error;
} PAYLOAD_ERROR;

typedef struct _PAYLOAD_READMEM
{
	uint32_t	addr;
	uint16_t	size;
} PAYLOAD_READMEM;

typedef struct _PAYLOAD_DUMPMEM
{
	uint32_t	addr;
	uint8_t		data[MAX_DATA_SIZE];
} PAYLOAD_DUMPMEM;

#pragma pack(pop)

typedef struct _BootloaderPacket
{
	//
	uint8_t			code;

	//
	uint16_t		payloadLen;

	union
	{
		PAYLOAD_IDENTIFY	i;
		PAYLOAD_ADDRESS		a;
		PAYLOAD_READMEM		r;
		PAYLOAD_DUMPMEM		d;
		PAYLOAD_ERROR		e;

		uint8_t				data[MAX_PAYLOAD_SIZE];
	};

} BPacket;

/////////////////////////////////////////////////////////////////////////////////////////
//

class BPacketParser
{
public:
	BPacketParser();

	enum STATE
	{
		_SOF,
		_CODE,
		_LEN_MSB,
		_LEN_LSB,
		_PAYLOAD,
		_CHECKSUM,
		_DONE
	};

public:
	int32_t		getPacket(BPacket * packet);

	int32_t		push(uint8_t c);
	void		reset();

private:
	STATE		state;

	uint8_t		code;

	uint16_t	payloadLen;
	uint16_t	payloadMax;
	uint8_t		payload[MAX_PAYLOAD_SIZE];

	//
	uint8_t		checkSum;
};


/////////////////////////////////////////////////////////////////////////////////////////
// BPacketListener

class BPacketListener
{
public:
	virtual void OnBPacketReceived(BPacket * pPacket) = 0;
};




/////////////////////////////////////////////////////////////////////////////////////////
// Variometer stubs

enum VCMD_TYPE
{
	VCMD_INVALID			= 0,
	VCMD_STATUS				= 'ST',
	VCMD_RESET				= 'RS',
	VCMD_SHUTDOWN			= 'SD',
	VCMD_FIRMWARE_VERSION	= 'FV',
	VCMD_MODE_SWITCH		= 'MS',
	VCMD_SOUND_LEVEL		= 'SL',
	VCMD_TONE_TEST			= 'TT',
	VCMD_DUMP_SENSOR		= 'DS',
	VCMD_DUMP_PROPERTY		= 'DP',
	VCMD_DUMP_CONFIG		= 'DC',
	VCMD_BLOCK_GPS_NMEA		= 'BG',
	VCMD_BLOCK_VARIO_NMEA	= 'BV',
	VCMD_FACTORY_RESET		= 'FR',
	VCMD_RESTORE_PROPERTY	= 'RP',
	VCMD_SAVE_PROPERTY		= 'SP',
	VCMD_QUERY_PROPERTY		= 'QP',
	VCMD_UPDATE_PROPERTY	= 'UP',
	VCMD_ACCEL_CALIBRATION	= 'AC',
	VCMD_RUN_BOOTLOADER		= 'RB',

};


//=======================================================================================
//

class VCommandMaker
{
public:
	VCommandMaker();

public:
	void			start(uint16_t code);
	int				finish();

	int				push_number(long long n);
	int				push_float(double d);

	uint8_t *		get_data() { return &dataBuf[0]; }
	uint16_t		get_size() { return dataLen; }

private:
	int				push_uchar(unsigned char c);

private:
	uint8_t			dataBuf[1024];
	uint16_t		dataLen;
};


//=======================================================================================
//

class VResponse
{
protected:
	VResponse();
public:
	VResponse(const VResponse & r);

	enum DATA_TYPE
	{
		_NONE,
		_NUMBER,
		_FLOAT,
		_STRING
	};

public:
	//
	int					toString(char * buf, int bufLen);

	VCMD_TYPE			getCode() { return code;  }
	DATA_TYPE			getDataType() { return dataType; }
	int					getDataCount() { return dataCount; }
	int64_t				getNumber(int index = 0);
	double				getFloat(int index = 0);
	int					getString(char * buf, int bufLen);

	VResponse &			operator = (const VResponse & r);

	//
	static VResponse 	parse(const char * data, int dataLen);

protected:
	static DATA_TYPE	verifyType(const char * str);
	static VCMD_TYPE	getCode(const char * str);

protected:
	VCMD_TYPE			code;
	uint32_t			param;

	DATA_TYPE			dataType;
	int					dataCount;

	union _DATA
	{
		int64_t		n[4];
		double		d[4];
		char		s[64];
	} data;
};


//=======================================================================================
//

class VLineBuffer
{
public:
	VLineBuffer();

	enum STATE
	{
		_SOF,
		_BODY,
		_TERMINATE,
		_DONE
	};

public:
	int				push(char c); // return > 0 : ready a line
	void			reset();

	char *			getLine()		{ return &data[0]; }
	int				getLineLength() { return dataLen; } // except new-line characters

private:
	STATE			state;

	char			data[2048];
	int				dataLen;
};


#endif // __PACKET_H__

