// command.h
//

#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "stm32f10x.h"


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
#define ERROR_TIMEOUT			(0xC002)


/////////////////////////////////////////////////////////////////////////////////////////
//

#pragma pack(push, 1)

typedef struct
{
	uint16_t	devId;
	uint16_t	fwVer;
} PAYLOAD_IDENTIFY;

typedef struct
{
	uint32_t	addr;
} PAYLOAD_ADDRESS;

typedef struct
{
	uint32_t	addr;
	uint16_t	size;
} PAYLOAD_READ_MEM;

typedef struct
{
	uint32_t	addr;
	uint8_t		data[MAX_DATA_SIZE];
} PAYLOAD_WRITE_MEM;

#pragma pack(pop)


typedef struct
{
	//
	uint8_t			code;

	//
	uint16_t		payloadLen;

	union
	{
		PAYLOAD_IDENTIFY	i;
		PAYLOAD_ADDRESS		a;
		PAYLOAD_READ_MEM	r;
		PAYLOAD_WRITE_MEM	w;

		uint8_t				data[MAX_PAYLOAD_SIZE];
	};

} BOOT_CMD;


#endif // __COMMAND_H__
