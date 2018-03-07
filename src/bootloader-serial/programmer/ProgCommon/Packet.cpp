// Packet.cpp
//

#include "stdafx.h"
#include "Packet.h"



/////////////////////////////////////////////////////////////////////////////////////////
//

BPacketMaker::BPacketMaker() : dataLen(0)
{
}

int BPacketMaker::push_u8(uint8_t data)
{
	dataBuf[dataLen++] = data;
	checkSum ^= data;

	return dataLen;
}

int BPacketMaker::push_u16(uint16_t data)
{
	dataBuf[dataLen++] = (data & 0x00FF);
	checkSum ^= (data & 0x00FF);

	dataBuf[dataLen++] = (data >> 8);
	checkSum ^= (data >> 8);

	return dataLen;
}

int BPacketMaker::push_u32(uint32_t data)
{
	dataBuf[dataLen++] = (data & 0x00FF);
	checkSum ^= (data & 0x00FF);

	dataBuf[dataLen++] = ((data >> 8) & 0x00FF);
	checkSum ^= ((data >> 8) & 0x00FF);

	dataBuf[dataLen++] = ((data >> 16) & 0x00FF);
	checkSum ^= ((data >> 16) & 0x00FF);

	dataBuf[dataLen++] = ((data >> 24) & 0x00FF);
	checkSum ^= ((data >> 24) & 0x00FF);

	return dataLen;
}

int BPacketMaker::push_mem(uint8_t * data, uint16_t len)
{
	for (int i = 0; i < len; i++)
	{
		dataBuf[dataLen++] = data[i];
		checkSum ^= data[i];
	}

	return dataLen;
}

void BPacketMaker::start(uint8_t code)
{
	checkSum = 0x00;
	dataLen = 0;

	push_u8(PACKET_SOF);
	push_u8(code);
	push_u16(0x0000);
}

int BPacketMaker::finish()
{
	//
	uint16_t payload = dataLen - (1 + 1 + 2);

	if (payload > 0)
	{
		dataBuf[2] = (payload & 0x00FF);
		checkSum ^= (payload & 0x00FF);

		dataBuf[3] = (payload >> 8);
		checkSum ^= (payload >> 8);
	}

	//
	dataBuf[dataLen++] = checkSum;

	return dataLen;
}



/////////////////////////////////////////////////////////////////////////////////////////
//

BPacketParser::BPacketParser()
{
	reset();
}

int32_t BPacketParser::push(uint8_t ch)
{
	//
	switch (state)
	{
	case _SOF:
		if (ch == PACKET_SOF)
		{
			state = _CODE;
			checkSum = (uint8_t)ch;
		}
		break;
	case _CODE:
		switch (ch)
		{
		case DCODE_IDENTIFY :
		case DCODE_DUMP_MEM:
		case DCODE_ACK:
		case DCODE_NACK:
			code = ch;
			checkSum ^= (uint8_t)ch;
			state = _LEN_LSB;
			break;
		default:
			state = _SOF;
			break;
		}
		break;
	case _LEN_LSB:
		payloadMax = ch;
		checkSum ^= (uint8_t)ch;
		state = _LEN_MSB;
		break;
	case _LEN_MSB:
		payloadMax = payloadMax + (ch << 8);
		payloadLen = 0;
		checkSum ^= (uint8_t)ch;

		if (payloadMax > 0)
			state = _PAYLOAD;
		else
			state = _CHECKSUM;
		break;
	case _PAYLOAD:
		payload[payloadLen] = ch;
		payloadLen++;
		checkSum ^= (uint8_t)ch;

		if (payloadLen == payloadMax)
			state = _CHECKSUM;
		break;
	case _CHECKSUM:
		if (checkSum == ch)
			state = _DONE;
		else
			state = _SOF;
		break;
	case _DONE:
		break;
	}

	return state == _DONE ? 1 : 0;
}

void BPacketParser::reset()
{
	state = _SOF;
}

int32_t BPacketParser::getPacket(BPacket * packet)
{
	//
	if (state != _DONE)
		return -1;

	memset(packet, 0, sizeof(BPacket));

	//
	packet->code = code;

	switch (packet->code)
	{
	case DCODE_ACK:
		if (payloadLen != 0)
			packet->code = 0;
		break;
	case DCODE_NACK:
		if (payloadLen != 2)
			packet->code = 0;

		memcpy(&packet->data[0], &payload[0], payloadLen);
		packet->payloadLen = payloadLen;
		break;
	case DCODE_IDENTIFY:
		if (payloadLen != sizeof(PAYLOAD_IDENTIFY))
		{
			packet->code = 0;
			break;
		}

		memcpy(&packet->data[0], &payload[0], payloadLen);
		packet->payloadLen = payloadLen;
		break;
	case DCODE_DUMP_MEM:
		if (payloadLen < 5) // address + data
		{
			packet->code = 0;
			break;
		}

		memcpy(&packet->data[0], &payload[0], payloadLen);
		packet->payloadLen = payloadLen;
		break;
	}

	//
	this->state = _SOF;

	return packet->code != 0 ? 0 : -1;
}



/////////////////////////////////////////////////////////////////////////////////////////
//


//=======================================================================================
//  class VCommandMaker

VCommandMaker::VCommandMaker() : dataLen(0)
{
}

void VCommandMaker::start(uint16_t code)
{
	//
	dataLen = 0;

	push_uchar('#');
	push_uchar(code >> 8);
	push_uchar(code & 0xFF);
}

int VCommandMaker::finish()
{
	push_uchar('\r');
	push_uchar('\n');

	return dataLen;
}

int VCommandMaker::push_number(long long n)
{
	push_uchar(',');
	dataLen += sprintf_s((char *)&dataBuf[dataLen], sizeof(dataBuf) - dataLen, "%lld", n);

	return dataLen;
}

int VCommandMaker::push_float(double d)
{
	push_uchar(',');
	dataLen += sprintf_s((char *)&dataBuf[dataLen], sizeof(dataBuf) - dataLen, "%.4lf", d);

	return dataLen;
}

int VCommandMaker::push_uchar(unsigned char c)
{
	dataBuf[dataLen++] = c;

	return dataLen;
}

//=======================================================================================
//  class VResponse

VResponse::VResponse()
{
	code = VCMD_INVALID;
	param = (uint32_t)-1;

	dataCount = 0;
}

VResponse::VResponse(const VResponse & r)
{
	*this = r;
}

VResponse &	VResponse::operator = (const VResponse & r)
{
	memcpy(this, &r, sizeof(r));

	return *this;
}

VCMD_TYPE VResponse::getCode(const char * str)
{
	if (strlen(str) != 2)
		return VCMD_INVALID;

	if (str[0] < 'A' || 'Z' < str[0])
		return VCMD_INVALID;

	if (str[1] < 'A' || 'Z' < str[1])
		return VCMD_INVALID;

	return (VCMD_TYPE)((str[0] << 8) + str[1]);
}

VResponse::DATA_TYPE VResponse::verifyType(const char *str)
{
	DATA_TYPE type = _NONE;

	while (*str && type != _STRING)
	{
		int c = *str++;

		switch (type)
		{
		case _NONE		:
			if (('0' <= c && c <= '9') || c == '-')
				type = _NUMBER;
			else
				type = _STRING;
			break;
		case _NUMBER	:
			if (c == '.')
				type = _FLOAT;
			else if (c < '0' || '9' < c)
				type = _STRING;
			break;
		case _FLOAT		:
			if (c < '0' || '9' < c)
				type = _STRING;
			break;
		}
	}

	return type;
}

VResponse VResponse::parse(const char * data, int dataLen)
{
	//
	VResponse r;

	// minimal line-length
	if (dataLen < 2)
		return r;

	// tokenize
	char * buf = (char *)_alloca(dataLen + 1);
	char * ptr = buf, * next = NULL, * tok[10];
	int tokLen;

	strcpy_s(buf, dataLen + 1, &data[0]);
	for (tokLen = 0; tokLen < sizeof(tok) / sizeof(tok[0]); tokLen++)
	{
		tok[tokLen] = strtok_s(ptr, ",", &next);
		ptr = NULL;

		if (!tok[tokLen])
			break;
	}

	if ((r.code = (VCMD_TYPE)getCode(tok[0])) != VCMD_INVALID)
	{
		if (tokLen > 1)
		{
			r.param = atol(tok[1]);

			if (tokLen > 2)
			{
				r.dataCount = __min(tokLen - 2, 4);
				r.dataType = verifyType(tok[2]);

				for (int i = 1; i < r.dataCount; i++)
				{
					DATA_TYPE type = verifyType(tok[i + 2]);

					// n+n->n, n+f->f, f+s->s
					if (r.dataType < type)
						r.dataType = type; 
				}

				for (int i = 0; i < r.dataCount; i++)
				{
					switch (r.dataType)
					{
					case _NUMBER :
						r.data.n[i] = atoll(tok[i + 2]);
						break;
					case _FLOAT :
						r.data.d[i] = atof(tok[i + 2]);
						break;
					case _STRING :
						if (i > 0)
							strcat_s(r.data.s, ",");
						else
							strcpy_s(r.data.s, "");
						strcat_s(&r.data.s[0], sizeof(r.data.s), tok[i + 2]);
						break;
					}
				}

				if (r.dataType == _STRING)
					r.dataCount = 1;
			}
		}
	}

	return r;
}

int VResponse::toString(char * buf, int bufLen)
{
	char * ptr = buf;

	ptr += sprintf_s(ptr, bufLen - (ptr - buf), "%%%c%c", code >> 8, code & 0xFF);
	if (param != (uint32_t)-1)
	{
		ptr += sprintf_s(ptr, bufLen - (ptr - buf), ",%d", param);

		for (int i = 0; i < dataCount; i++)
		{
			switch (dataType)
			{
			case _NUMBER :
				ptr += sprintf_s(ptr, bufLen - (ptr - buf), ",%lld", data.n[i]);
				break;
			case _FLOAT :
				ptr += sprintf_s(ptr, bufLen - (ptr - buf), ",%.4lf", data.d[i]);
				break;
			case _STRING :
				ptr += sprintf_s(ptr, bufLen - (ptr - buf), ",%s", data.s);
				break;
			}
		}
	}
	strcat_s(ptr, bufLen - (ptr - buf), "\r\n");

	return strlen(buf);
}

int64_t	VResponse::getNumber(int index)
{
	if (index < dataCount)
	{
		switch (dataType)
		{
		case _NUMBER :
			return data.n[index];
		case _FLOAT :
			return (int64_t)data.d[index];
		case _STRING :
			return atoll(data.s);
		}
	}

	return 0;
}

double VResponse::getFloat(int index)
{
	if (index < dataCount)
	{
		switch (dataType)
		{
		case _NUMBER:
			return (double)data.n[index];
		case _FLOAT:
			return data.d[index];
		case _STRING:
			return atof(data.s);
		}
	}

	return 0;
}

int	VResponse::getString(char * buf, int bufLen)
{
	char * ptr;
	int n;

	switch (dataType)
	{
	case _NUMBER:
		ptr = buf;

		for (int i = 0; i < dataCount; i++)
		{
			if (i != 0)
				ptr += sprintf_s(buf, bufLen - (ptr - buf), ",");

			ptr += sprintf_s(buf, bufLen - (ptr - buf), "%lld", data.n[i]);
		}
		break;
	case _FLOAT:
		ptr = buf;

		for (int i = 0; i < dataCount; i++)
		{
			if (i != 0)
				ptr += sprintf_s(buf, bufLen - (ptr - buf), ",");

			ptr += sprintf_s(buf, bufLen - (ptr - buf), "%.4lf", data.d[i]);
		}
		break;
	case _STRING:
		n = __min(strlen(data.s), bufLen);

		memset(buf, 0, bufLen);
		memcpy(buf, data.s, n);

		return n;
	}

	return 0;
}

// use-cases
//
// VLineBuffer  buf;
// VResponse r = VResponse::parse(buf.getLine(), buf.getLineLength());


//=======================================================================================
//  class VLineBuffer

VLineBuffer::VLineBuffer()
{
	reset();
}

int	VLineBuffer::push(char c)
{
	switch (state)
	{
	case _DONE:
	case _SOF:
		if (c == '$' || c == '%')
		{
			state = _BODY;
			data[0] = '\0';
			dataLen = 0;
		}
		break;
	case _BODY :
		if (c == '\n')
		{
			// we allows both case; \n & \r\n 
			state = _DONE;
		}
		else if (c == '\r')
		{
			// first line end character
			state = _TERMINATE;
		}
		else if (dataLen < sizeof(data))
		{
			// normal body data
			data[dataLen++] = c;
			data[dataLen] = '\0';
		}
		else
		{
			// too long
			state = _SOF;
			dataLen = 0;
		}
		break;
	case _TERMINATE:
		if (c == '\n')
		{
			// second line end character
			state = _DONE;
		}
		else
		{
			// invalid line
			state = _SOF;
			dataLen = 0;
		}
		break;
	}

	//if (state == _DONE)
	//{
	//	if (dataLen > 2)
	//		return 1;
	//
	//	state = _SOF;
	//	dataLen = 0;
	//}

	return state == _DONE ? 1 : 0;
}

void VLineBuffer::reset()
{
	state = _SOF;
	dataLen = 0;
}
