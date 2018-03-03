// cparser.c
//

#include "common/cparser.h"
#include "driver/serial.h"
#include "diag/Trace.h"


/////////////////////////////////////////////////////////////////////////////////////////
//

typedef enum
{
	_SOF,
	_CODE,
	_LEN_MSB,
	_LEN_LSB,
	_PAYLOAD,
	_CHECKSUM,
	_DONE

} PARSER_STATE;

typedef struct
{
	//
	PARSER_STATE	state;

	//
	uint8_t			code;

	uint16_t		payloadLen;
	uint16_t		payloadMax;
	uint8_t			payload[MAX_PAYLOAD_SIZE];

	//
	uint8_t			checkSum;

} CPARSER;


static CPARSER _cp;


void cparser_zeroMemroy(uint8_t * ptr, int32_t size);
void cparser_copyMemory(uint8_t * dst, uint8_t * src, int32_t size);


/////////////////////////////////////////////////////////////////////////////////////////
//

void cparser_init(void)
{
	_cp.state = _SOF;
}

int32_t	cparser_update(void)
{
	while (serial_available() && _cp.state != _DONE)
	{
		//
		int ch = serial_read();

		switch (_cp.state)
		{
		case _SOF :
			if (ch == PACKET_SOF)
			{
				_cp.state = _CODE;
				_cp.checkSum = (uint8_t)ch;
			}
			break;
		case _CODE :
			switch(ch)
			{
			case HCODE_IDENTIFY: // Identify
			case HCODE_ERASE : // Erase
			case HCODE_ERASE_ALL : // Erase All
			case HCODE_WRITE : // Write
			case HCODE_READ : // Read
			case HCODE_RESET : // Reset
			case HCODE_START : // Start (Jump to User Program)
				_cp.code = ch;
				_cp.checkSum ^= (uint8_t)ch;
				_cp.state = _LEN_LSB;
				break;
			default :
				_cp.state = _SOF;
				break;
			}
			break;
		case _LEN_LSB :
			_cp.payloadMax = ch;
			_cp.checkSum ^= (uint8_t)ch;
			_cp.state = _LEN_MSB;
			break;
		case _LEN_MSB :
			_cp.payloadMax += (ch << 8);
			_cp.payloadLen = 0;
			_cp.checkSum ^= (uint8_t)ch;

			if (_cp.payloadMax > 0)
				_cp.state = _PAYLOAD;
			else
				_cp.state = _CHECKSUM;
			break;
		case _PAYLOAD :
			_cp.payload[_cp.payloadLen] = ch;
			_cp.payloadLen ++;
			_cp.checkSum ^= (uint8_t)ch;

			if (_cp.payloadLen == _cp.payloadMax)
				_cp.state = _CHECKSUM;
			break;
		case _CHECKSUM :
			if (_cp.checkSum == ch)
			{
				//trace_printf("New command : %02Xh (%d) %02h\n", _cp.code, _cp.payloadMax, ch);
				_cp.state = _DONE;
			}
			else
			{
				//trace_printf("Invalid Checksum : %02Xh -> %02Xh\n", _cp.checkSum, ch);
				_cp.state = _SOF;
			}
			break;
		case _DONE :
			break;
		}
	}

	return _cp.state == _DONE ?  1 : 0;
}

int32_t	cparser_available(void)
{
	return _cp.state == _DONE ? 1 : 0;
}

int32_t	cparser_getBootCommand(BOOT_CMD * cmd)
{
	if (_cp.state != _DONE)
		return -1;

	//
	cparser_zeroMemroy((uint8_t *)cmd, sizeof(BOOT_CMD));

	//
	cmd->code = _cp.code;

	switch (cmd->code)
	{
	case HCODE_IDENTIFY : // Identify
	case HCODE_ERASE_ALL : // Erase All
	case HCODE_RESET : // Reset
	case HCODE_START : // Start
		if (_cp.payloadLen != 0)
			cmd->code = 0;
		break;

	case HCODE_ERASE : // Erase
		if (_cp.payloadLen != 4 && _cp.payloadLen != 8)
		{
			cmd->code = 0;
			break;
		}

		cmd->payloadLen = _cp.payloadLen;
		cparser_copyMemory(&cmd->data[0], &_cp.payload[0], _cp.payloadLen);
		//cmd->a.addr = ((uint32_t)_cp.payload[3] << 24) + ((uint32_t)_cp.payload[2] << 16) + ((uint32_t)_cp.payload[1] << 8) + ((uint32_t)_cp.payload[0]);
		break;

	case HCODE_WRITE : // Write
		if (_cp.payloadLen < 6)
		{
			cmd->code = 0;
			break;
		}

		cmd->payloadLen = _cp.payloadLen;
		cparser_copyMemory(&cmd->data[0], &_cp.payload[0], _cp.payloadLen);
		//cmd->w.addr = ((uint32_t)_cp.payload[3] << 24) + ((uint32_t)_cp.payload[2] << 16) + ((uint32_t)_cp.payload[1] << 8) + ((uint32_t)_cp.payload[0]);
		//cparser_copyMemory((uint8_t *)&cmd->w.data[0], (uint8_t *)&_cp.payload[4], _cp.payloadLen - 4);
		break;

	case HCODE_READ : // Read
		if (_cp.payloadLen != 6)
		{
			cmd->code = 0;
			break;
		}

		cmd->payloadLen = _cp.payloadLen;
		cparser_copyMemory(&cmd->data[0], &_cp.payload[0], _cp.payloadLen);
		//cmd->r.addr = ((uint32_t)_cp.payload[3] << 24) + ((uint32_t)_cp.payload[2] << 16) + ((uint32_t)_cp.payload[1] << 8) + ((uint32_t)_cp.payload[0]);
		//cmd->r.size = ((uint32_t)_cp.payload[5] << 8) + ((uint32_t)_cp.payload[4]);
		break;
	}

	//
	_cp.state = _SOF;

	return cmd->code != 0 ? 0 : -1;
}

void cparser_zeroMemroy(uint8_t * ptr, int32_t size)
{
	while (size-- > 0)
		*ptr++ = 0;
}

void cparser_copyMemory(uint8_t * dst, uint8_t * src, int32_t size)
{

	while (size-- > 0)
		*dst++ = *src++;
}
