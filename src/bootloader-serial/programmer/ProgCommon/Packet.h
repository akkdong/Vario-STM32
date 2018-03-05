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

class CommandMaker
{
public:
	CommandMaker();

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

typedef struct _PACKET
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

} PACKET;

/////////////////////////////////////////////////////////////////////////////////////////
//

class PacketParser
{
public:
	PacketParser();

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
	int32_t		getPacket(PACKET * packet);

	int32_t		push(uint8_t c);

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
// PacketListener

class PacketListener
{
public:
	virtual void OnPacketReceived(PACKET * pPacket) = 0;
};


#endif // __PACKET_H__

