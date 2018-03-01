#pragma once

#include "stdafx.h"
#include "Packet.h"



/////////////////////////////////////////////////////////////////////////////////////////
//

CommandMaker::CommandMaker()
{
}

int CommandMaker::push_u8(uint8_t data)
{
	dataBuf[dataLen++] = data;
	checkSum ^= data;

	return dataLen;
}

int CommandMaker::push_u16(uint16_t data)
{
	dataBuf[dataLen++] = (data & 0x00FF);
	checkSum ^= (data & 0x00FF);

	dataBuf[dataLen++] = (data >> 8);
	checkSum ^= (data >> 8);

	return dataLen;
}

int CommandMaker::push_u32(uint32_t data)
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

int CommandMaker::push_mem(uint8_t * data, uint16_t len)
{
	for (int i = 0; i < len; i++)
	{
		dataBuf[dataLen++] = data[i];
		checkSum ^= data[i];
	}

	return dataLen;
}

void CommandMaker::start(uint8_t code)
{
	checkSum = 0x00;
	dataLen = 0;

	push_u8(PACKET_SOF);
	push_u8(code);
	push_u16(0x0000);
}

int CommandMaker::finish()
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

PacketParser::PacketParser()
{
	state = _SOF;
}

int32_t PacketParser::push(uint8_t ch)
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

int32_t PacketParser::getPacket(PACKET * packet)
{
	//
	if (state != _DONE)
		return -1;

	memset(packet, 0, sizeof(PACKET));

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

	return packet != 0 ? 0 : -1;
}