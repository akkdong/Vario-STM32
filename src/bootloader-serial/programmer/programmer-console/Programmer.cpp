// Programmer.cpp
//

#include "stdafx.h"
#include "Programmer.h"


////////////////////////////////////////////////////////////////////////////////////
//

extern int Print(LPCTSTR format, ...);



////////////////////////////////////////////////////////////////////////////////////
//

static void printBar(int leading, int count, char c)
{
	for (int i = 0; i < leading; i++)
		Print(" ");
	for (int i = 0; i < count; i++)
		Print("%c", c);
}


////////////////////////////////////////////////////////////////////////////////////
//

Programmer::Programmer()
{
}

int Programmer::ConnectAndIdentify(int portNum, CSerial::EBaudrate baudRate, CSerial::EDataBits dataBits, CSerial::EParity parity, CSerial::EStopBits stopBits, CSerial::EHandshake handshake)
{
	//
	char port[32];
	BPacket packet;

	Print("> Open serial port COM%d: ", portNum);
	wsprintf(port, "\\\\.\\COM%d", portNum);
	if (mSerial.Open(port) != ERROR_SUCCESS)
	{
		Print("FAIL\n\n");

		Print("  ERR!\n");
		Print("  ERR! Serial(COM%d) open failed!\n", portNum);
		Print("  ERR!\n");

		return -1;
	}
	else
	{
		Print("OK!\n\n");
	}

	//
	mSerial.Setup(baudRate, dataBits, parity, stopBits);
	mSerial.SetupHandshaking(handshake);
	mSerial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);

	Print("> Request reboot & identify device\n");
	for (int i = 0; i < 5; i++)
	{
		Print("  #%d send identify: ", i + 1);

		// send reboot command
		SendRebootCommand();
		// wait a moment
		Sleep(i == 0 ? 400 : 200);
		// send identify
		SendIdentify();

		//
		if (WaitPacket(&packet, 800) == 0)
		{
			if (packet.code == DCODE_IDENTIFY && packet.i.fwVer == 0x0100) // packet.i.devId == 0x0414)
			{
				Print("OK!\n\n");				
				return 0;;
			}

			Print("Invalid response!\n");
			Print("    -> devId(%04X), fwVer(%04X)\n\n", packet.i.devId, packet.i.fwVer);
			break;
		}
		else
		{
			Print("No response!\n");
		}
	}

	return -1;
}

int Programmer::Program(ImageFile * pFile, uint32_t address, BOOL verify)
{
	// first erase...
	if (EraseMemory(address, pFile->getSize()) < 0)
		return -1;

	// programming
	if (WriteToMemory(pFile, address, verify) < 0)
		return -1;

	// verify programming
	if (verify)
	{
		pFile->Rewind();

		if (VerifyMemory(pFile, address) < 0)
			return -1;
	}

	return 0;
}

int Programmer::Dump(ImageFile * pFile, uint32_t address, uint32_t length)
{
	if (ReadFromMemory(pFile, address, length) < 0)
		return -1;

	return 0;
}

int Programmer::Erase(uint32_t address, uint32_t length)
{
	if (EraseMemory(address, length) < 0)
		return -1;

	return 0;
}


int Programmer::WriteToMemory(ImageFile * pFile, uint32_t address, BOOL verify)
{
	uint32_t addr = address;
	uint32_t remain = pFile->getSize();

	Print("> Start programming: 0x%08X\n", address);
	printBar(2, (pFile->getSize() + PROGRAM_SIZE - 1) / PROGRAM_SIZE, '=');
	Print("\n  ");

	while (remain > 0)
	{
		uint32_t prog_size = remain < PROGRAM_SIZE ? remain : PROGRAM_SIZE;
		uint32_t actual_size = (prog_size + sizeof(uint32_t) - 1) / sizeof(uint32_t) * sizeof(uint32_t);
		uint8_t data[PROGRAM_SIZE];
		BPacket packet;

		memset(data, 0xFF, sizeof(data));
		pFile->Read(data, &prog_size);
		RequestWrite(addr, data, actual_size);

		//
		if (WaitPacket(&packet, 2000) == 0)
		{
			if (packet.code == DCODE_ACK)
			{
				Print("*");
			}
			else if (packet.code == DCODE_NACK)
			{
				Print("\n\n");

				Print("  ERR!\n");
				Print("  ERR! Write failed!\n");
				Print("  ERR!   -> error: %04X\n", packet.e.error);
				Print("  ERR!\n");

				return -1;
			}
			else
			{
				Print("\n\n");

				Print("  ERR!\n");
				Print("  ERR! Invalid reponse\n");
				Print("  ERR!   -> code: %02X\n", packet.code);
				Print("  ERR!\n");

				return -1;
			}
		}
		else
		{
			Print("\n\n");

			Print("  ERR!\n");
			Print("  ERR! Write timeout.\n");
			Print("  ERR!\n");

			return -1;
		}

		remain -= prog_size;
		addr += prog_size;
	}

	Print("\n  Done!\n\n");

	return 0;
}

int Programmer::VerifyMemory(ImageFile * pFile, uint32_t address)
{
	uint32_t addr = address;
	uint32_t remain = pFile->getSize();

	Print("> Verify programming: 0x%08X\n", address);
	printBar(2, (pFile->getSize() + PROGRAM_SIZE - 1) / PROGRAM_SIZE, '=');
	Print("\n  ");

	while (remain > 0)
	{
		uint32_t prog_size = remain < PROGRAM_SIZE ? remain : PROGRAM_SIZE;
		uint32_t actual_size = (prog_size + sizeof(uint32_t) - 1) / sizeof(uint32_t) * sizeof(uint32_t);
		uint8_t data[PROGRAM_SIZE];
		BPacket packet;

		memset(data, 0xFF, sizeof(data));
		pFile->Read(data, &prog_size);
		RequestRead(addr, actual_size);

		//
		if (WaitPacket(&packet, 2000) == 0)
		{
			if (packet.code == DCODE_DUMP_MEM)
			{
				for (uint32_t i = 0; i < prog_size; i++)
				{
					if (data[i] != packet.d.data[i])
					{
						Print("\n\n");

						Print("  ERR!\n");
						Print("  ERR! Verify failed at 0x%08X\n", addr + i);
						Print("  ERR!\n");

						return -1;
					}
				}

				Print("*");
			}
			else if (packet.code == DCODE_NACK)
			{
				Print("\n\n");

				Print("  ERR!\n");
				Print("  ERR! Read failed at 0x%08X\n", addr);
				Print("  ERR!   -> error: %04X\n", packet.e.error);
				Print("  ERR!\n");

				return -1;
			}
			else
			{
				Print("\n\n");
				Print("  ERR!\n");
				Print("  ERR! Invalid reponse\n");
				Print("  ERR!   -> code: %02X\n", packet.code);
				Print("  ERR!\n");

				return -1;
			}
		}
		else
		{
			Print("\n\n");

			Print("  ERR!\n");
			Print("  ERR! Read timeout.\n");
			Print("  ERR!\n");

			return -1;
		}

		remain -= prog_size;
		addr += prog_size;
	}

	Print("\n  Done!\n\n");

	return 0;
}

int Programmer::ReadFromMemory(ImageFile * pFile, uint32_t address, uint32_t length)
{
	uint32_t addr = address;
	uint32_t remain = length;
	BPacket packet;

	Print("> Read memory at 0x%08X ", address);
	while (remain > 0)
	{
		uint32_t size = remain < PROGRAM_SIZE ? remain : PROGRAM_SIZE;
		RequestRead(addr, size);

		//
		if (WaitPacket(&packet, 2000) == 0)
		{
			if (packet.code == DCODE_DUMP_MEM)
			{
				pFile->Write(&packet.d.data[0], size);

				remain = remain - size;
				Print(".");
			}
			else if (packet.code == DCODE_NACK)
			{
				Print("\n\n");

				Print("  ERR!\n");
				Print("  ERR! Read failed at 0x%08X\n", addr);
				Print("  ERR!   -> error: %04X\n", packet.e.error);
				Print("  ERR!\n");

				return -1;
			}
			else
			{
				Print("\n\n");

				Print("  ERR!\n");
				Print("  ERR! Invalid reponse\n");
				Print("  ERR!   -> code: %02X\n", packet.code);
				Print("  ERR!\n");

				return -1;
			}
		}
		else
		{
			Print("\n\n");

			Print("  ERR!\n");
			Print("  ERR! Read timeout.\n");
			Print("  ERR!\n");

			return -1;
		}

		addr += size;
	}

	Print("\n  Done!\n\n");
	return 0;
}

int Programmer::EraseMemory(uint32_t address, uint32_t length)
{
	uint32_t addr_s = address / PAGE_SIZE * PAGE_SIZE;
	uint32_t addr_e = (address + length) / PAGE_SIZE * PAGE_SIZE;
	BPacket packet;

	Print("> Erase memory from 0x%08X to 0x%08X ...", addr_s, addr_e);
	RequestErase(addr_s, addr_e);

	//
	if (WaitPacket(&packet, 4000) == 0)
	{
		if (packet.code == DCODE_ACK)
		{
			Print("\n  Done!\n\n");

			return 0;
		}
		else if (packet.code == DCODE_NACK)
		{
			Print("\n\n");

			Print("  ERR!\n");
			Print("  ERR! Erase failed!\n");
			Print("  ERR!   -> error: %04X\n", packet.e.error);
			Print("  ERR!\n");
		}
		else
		{
			Print("\n\n");

			Print("  ERR!\n");
			Print("  ERR! Invalid reponse\n");
			Print("  ERR!   -> code: %02X\n", packet.code);
			Print("  ERR!\n");
		}
	}
	else
	{
		Print("\n\n");

		Print("  ERR!\n");
		Print("  ERR! Read timeout.\n");
		Print("  ERR!\n");
	}

	return -1;
}

void Programmer::Disconnect()
{
	mSerial.Close();
}

void Programmer::SendRebootCommand()
{
	mSerial.Write("#RB\r\n", 5);
}

void Programmer::SendIdentify()
{
	BPacketMaker maker;

	maker.start(HCODE_IDENTIFY);
	maker.finish();

	mSerial.Write(maker.get_data(), maker.get_size());
}

void Programmer::RequestWrite(uint32_t address, uint8_t * data, uint16_t dataLen)
{
	BPacketMaker maker;

	maker.start(HCODE_WRITE);
	maker.push_u32(address);
	maker.push_mem(data, dataLen);
	maker.finish();

	mSerial.Write(maker.get_data(), maker.get_size());
}

void Programmer::RequestRead(uint32_t address, uint16_t size)
{
	BPacketMaker maker;

	maker.start(HCODE_READ);
	maker.push_u32(address);
	maker.push_u16(size);
	maker.finish();

	mSerial.Write(maker.get_data(), maker.get_size());
}

void Programmer::RequestErase(uint32_t start, uint32_t end)
{
	BPacketMaker maker;

	maker.start(HCODE_ERASE);
	maker.push_u32(start);
	maker.push_u32(end);
	maker.finish();

	mSerial.Write(maker.get_data(), maker.get_size());
}

int Programmer::WaitPacket(BPacket * pPacket, UINT nTimeout)
{
	UCHAR ch;
	DWORD dwRead, dwTick = GetTickCount();

	while (GetTickCount() - dwTick < nTimeout)
	{
		while (mSerial.Read(&ch, sizeof(ch), &dwRead) == ERROR_SUCCESS && dwRead > 0)
		{
			if (mParser.push(ch))
			{
				mParser.getPacket(pPacket);
				return 0;
			}
		}
	}

	return -1; // timeout
}

void Programmer::RunUserApplication()
{
	BPacketMaker maker;

	maker.start(HCODE_START);
	maker.finish();

	mSerial.Write(maker.get_data(), maker.get_size());
}