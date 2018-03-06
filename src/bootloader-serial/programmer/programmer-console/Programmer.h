#pragma once

#include <stdint.h>

#include "Serial.h"
#include "ImageFile.h"
#include "Packet.h"
#include "Flash.h"


////////////////////////////////////////////////////////////////////////////////////
// class Programmer

class Programmer
{
public:
	Programmer();

public:
	int				ConnectAndIdentify(int portNum,
										CSerial::EBaudrate baudRate = CSerial::EBaud57600,
										CSerial::EDataBits dataBits = CSerial::EData8,
										CSerial::EParity parity = CSerial::EParNone,
										CSerial::EStopBits stopBits = CSerial::EStop1,
										CSerial::EHandshake handshake = CSerial::EHandshakeOff);
	void			Disconnect();

	int				Program(ImageFile * pFile, uint32_t address, BOOL verify);
	int				Dump(ImageFile * pFile, uint32_t address, uint32_t length);
	int				Erase(uint32_t address, uint32_t length);

	int				WriteToMemory(ImageFile * pFile, uint32_t address, BOOL verify);
	int				ReadFromMemory(ImageFile * pFile, uint32_t address, uint32_t length);
	int				EraseMemory(uint32_t address, uint32_t length);
	int				VerifyMemory(ImageFile * pFile, uint32_t address);

	void			RunUserApplication();

private:
	void			SendRebootCommand();
	void			SendIdentify();

	void			RequestWrite(uint32_t address, uint8_t * data, uint16_t dataLen);
	void			RequestRead(uint32_t address, uint16_t size);
	void			RequestErase(uint32_t start, uint32_t end);

	int				WaitPacket(BPacket * pPacket, UINT nTimeout);

private:
	CSerial			mSerial;
	BPacketParser	mParser;
};
