// csender.c
//

#include "common/csender.h"
#include "driver/serial.h"


/////////////////////////////////////////////////////////////////////////////////////////
//

static uint8_t	_checkSum;


/////////////////////////////////////////////////////////////////////////////////////////
//

void csender_start(uint8_t code, uint16_t len)
{
	serial_write(PACKET_SOF);
	_checkSum = PACKET_SOF;

	serial_write(code);
	_checkSum ^= code;

	serial_write(len & 0x00FF);
	_checkSum ^= (len & 0x00FF);

	serial_write(len >> 8);
	_checkSum ^= (len >> 8);
}

void csender_send_u8(uint8_t data)
{
	serial_write(data);
	_checkSum ^= data;
}

void csender_send_u16(uint16_t data)
{
	serial_write(data & 0x00FF);
	_checkSum ^= (data & 0x00FF);

	serial_write(data >> 8);
	_checkSum ^= (data >> 8);
}

void csender_send_u32(uint32_t data)
{
	uint8_t sendData;

	sendData = (data & 0x00FF);
	serial_write(sendData);
	_checkSum ^= sendData;

	sendData = ((data >> 8) & 0x00FF);
	serial_write(sendData);
	_checkSum ^= sendData;

	sendData = ((data >> 16) & 0x00FF);
	serial_write(sendData);
	_checkSum ^= sendData;

	sendData = ((data >> 24) & 0x00FF);
	serial_write(sendData);
	_checkSum ^= sendData;
}

void csender_send_mem(uint32_t address, uint16_t size)
{
	uint32_t start = address;
	uint32_t end = address + size;

	while (start < end)
	{
		uint32_t data = *((volatile uint32_t *)start);
		csender_send_u32(data);

		start += 4;
	}
}

void csender_finish(void)
{
	serial_write(_checkSum);
}

void csender_send_ack(void)
{
	csender_start(DCODE_ACK, 0);
	csender_finish();
}

void csender_send_nack(uint16_t error)
{
	csender_start(DCODE_NACK, 2);
	csender_send_u16(error);
	csender_finish();

}
