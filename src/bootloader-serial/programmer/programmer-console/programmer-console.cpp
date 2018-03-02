// programmer-console.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "HexFile.h"
#include "BinFile.h"
#include "Serial.h"
#include "Packet.h"


////////////////////////////////////////////////////////////////////////////////////
//

enum Operation
{
	_OP_UNDEF = 0,
	_OP_READ,
	_OP_WRITE,
	_OP_ERASE
};

enum FileType
{
	_FILE_BIN,
	_FILE_HEX
};

//CSerial				_serial;

CSerial::EBaudrate	_baudrate	= CSerial::EBaud57600;
CSerial::EDataBits	_databits	= CSerial::EData8;
CSerial::EParity	_parity		= CSerial::EParNone;
CSerial::EStopBits	_stopbits	= CSerial::EStop1;
CSerial::EHandshake	_handshake	= CSerial::EHandshakeOff;
int32_t				_portnum	= -1;

char				_filename[MAX_PATH] = { '\0' };
FileType			_filetype = _FILE_BIN;
Operation			_operation = _OP_UNDEF;
BOOL				_verify = TRUE;

uint32_t			_address = 0x08003000;
uint32_t			_length = 0x400;


////////////////////////////////////////////////////////////////////////////////////
//

void show_help(const char * name)
{
	printf("Usage: %s [-b rate] [-m mode] [-s address[:length]] [-v 0|1] -f filename -p portnum r|w|e\n", name);
	printf("   -b rate           serial baud rate (default 57600)\n");
	printf("   -m mode           serial port mode (default 8n1)\n");
	printf("   -s address:length start address and optionally length for read/write/erase\n");
	printf("                     address & length must be hexa-decimal\n");
	printf("   -v 0|1            verify writes (default true)\n");
	printf("   -f filename       filename for read or write\n");
	printf("   -p portnum        serial port number\n");
	printf("   r|w|e             operation: read or write or erase\n");
}

int parse_buadrate(const char * str)
{
	int baudrate = atoi(str);

	switch (baudrate)
	{
	case CSerial::EBaud2400 :
	case CSerial::EBaud4800:
	case CSerial::EBaud9600:
	case CSerial::EBaud14400:
	case CSerial::EBaud19200:
	case CSerial::EBaud38400:
	case CSerial::EBaud57600:
	case CSerial::EBaud115200:
		break;
	default :
		return - 1;
	}

	_baudrate = (CSerial::EBaudrate)baudrate;

	return 0;
}

int parse_serial_mode(const char * str)
{
	return -1;
}

int parse_start_address(const char * str)
{
	char buf[256];
	char * addr, * size;
	char * next;
	char * end = NULL;

	strcpy_s(buf, str);
	addr = strtok_s(buf, ":", &next);
	size = strtok_s(NULL, ":", &next);

	_address = strtol(addr, &end, 16); //  atoi(addr);

	if (size)
		_length = strtol(size, &end, 16); //  atoi(size);
	
	return 0;
}

int parse_verify(const char * str)
{
	if (strcmp(str, "0"))
		_verify = FALSE;
	else if (strcmp(str, "1"))
		_verify = TRUE;
	else
		return -1;

	return 0;
}

int parse_filename(const char * str)
{
	int len = strlen(str);

	if (len > 4)
	{
		strcpy_s(_filename, str);

		if (_stricmp(&str[len - 4], ".hex") == 0)
			_filetype = _FILE_HEX;
		else
			_filetype = _FILE_BIN;

		return 0;
	}

	return -1;
}

int parse_portnum(const char * str)
{
	int num = atoi(str);

	if (num > 0)
	{
		_portnum = num;
		return 0;
	}

	return -1;
}

int parse_opts(int argc, char * argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if (_stricmp(argv[i], "r") == 0)
		{
			_operation = _OP_READ;
		}
		else if (_stricmp(argv[i], "w") == 0)
		{
			_operation = _OP_WRITE;
		}
		else if (_stricmp(argv[i], "e") == 0)
		{
			_operation = _OP_ERASE;
		}
		else if (argv[i][0] == '-' && strlen(argv[i]) == 2 && i + 1 < argc)
		{
			switch (argv[i][1])
			{
			case 'b' :
				if (parse_buadrate(argv[i + 1]) < 0)
					return -1;
				break;
			case 'm' :
				if (parse_serial_mode(argv[i + 1]) < 0)
					return -1;
				break;
			case 's' :
				if (parse_start_address(argv[i + 1]) < 0)
					return -1;
				break;
			case 'v' :
				if (parse_verify(argv[i + 1]) < 0)
					return -1;
				break;
			case 'f' :
				if (parse_filename(argv[i + 1]) < 0)
					return -1;
				break;
			case 'p' :
				if (parse_portnum(argv[i + 1]) < 0)
					return -1;
				break;
			}

			i += 1;
		}
		else
		{
			return -1;
		}
	}

	return 0;
}

int check_opts()
{
	if (_operation == _OP_UNDEF)
		return -1;

	if (_portnum == 0)
		return -1;

	if (_filename[0] == NULL && _operation != _OP_ERASE)
		return -1;

	return 0;
}


#include "Flash.h"

////////////////////////////////////////////////////////////////////////////////////
//

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

	int				WriteToMemory(ImageFile * pFile, uint32_t address);
	int				ReadFromMemory(ImageFile * pFile, uint32_t address, uint32_t length);
	int				EraseMemory(uint32_t address, uint32_t length);

private:
	void			SendRebootCommand();
	void			SendIdentify();
	void			RequestMemory(uint32_t address, uint16_t size);

	int				WaitPacket(PACKET * pPacket, UINT nTimeout);

private:
	CSerial			mSerial;
	PacketParser	mParser;
};

Programmer::Programmer()
{
}

int Programmer::ConnectAndIdentify(int portNum, CSerial::EBaudrate baudRate, CSerial::EDataBits dataBits, CSerial::EParity parity, CSerial::EStopBits stopBits, CSerial::EHandshake handshake)
{
	//
	char port[32];
	PACKET packet;

	printf("> open serial port: ");
	wsprintf(port, "\\\\.\\COM%d", portNum);
	if (mSerial.Open(port) != ERROR_SUCCESS)
	{
		printf("FAIL\n\n");
		printf("  ERR!\n");
		printf("  ERR! serial(COM%d) open failed!\n", portNum);
		printf("  ERR!\n");

		return -1;
	}
	else
	{
		printf("OK\n\n");
	}

	//
	mSerial.Setup(_baudrate, _databits, _parity, _stopbits);
	mSerial.SetupHandshaking(_handshake);
	mSerial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);

	printf("> request reboot & identify device\n");
	for (int i = 0; i < 5; i++)
	{
		printf("  #%d send identify: ", i + 1);

		// send reboot command
		SendRebootCommand();
		// wait a moment
		Sleep(i == 0 ? 400 : 200);
		// send identify
		SendIdentify();

		//
		if (WaitPacket(&packet, 800) == 0)
		{
			if (packet.code == DCODE_IDENTIFY && packet.i.devId == 0x0414)
			{
				printf("OK!\n\n");
				break;
			}

			printf("invalid response!\n\n");
			return -1;
		}
		else
		{
			printf("no response!\n");
		}
	}

	return 0;
}

int Programmer::WriteToMemory(ImageFile * pFile, uint32_t address)
{
	return -1;
}

int Programmer::ReadFromMemory(ImageFile * pFile, uint32_t address, uint32_t length)
{
	uint32_t addr = address;
	uint32_t remain = length;
	PACKET packet;

	printf("> read memory at 0x%08X ", address);
	while (remain > 0)
	{
		uint32_t size = remain < PROGRAM_SIZE ? remain : PROGRAM_SIZE;
		RequestMemory(addr, size);

		//
		if (WaitPacket(&packet, 2000) == 0)
		{
			if (packet.code == DCODE_DUMP_MEM)
			{
				pFile->Write(&packet.d.data[0], size);

				remain = remain - size;
				printf(".");
			}
			/*
			else if (packet.code == DCODE_NACK)
			{
				printf("\n\n");
				printf("  ERR!\n");
				printf("  ERR! read failed at 0x%08X\n", addr);
				printf("  ERR!   -> code: %04X\n", packet.e.error);
				printf("  ERR!\n");

				return -1;
			}
			*/
			else
			{
				printf("\n\n");
				printf("  ERR!\n");
				printf("  ERR! read failed at 0x%08X\n", addr);
				printf("  ERR!   -> code: %04X\n", packet.e.error);
				printf("  ERR!\n");

				return -1;
			}
		}
		else
		{
			printf("\n\n");
			printf("  ERR!\n");
			printf("  ERR! read timeout.\n");
			printf("  ERR!\n");

			return -1;
		}
	}

	printf("\n  done!\n");
	return 0;
}

int Programmer::EraseMemory(uint32_t address, uint32_t length)
{
	return -1;
}

void Programmer::Disconnect()
{
	mSerial.Close();
}

void Programmer::SendRebootCommand()
{
	mSerial.Write("\x02\xAA\x00\x55", 4);
}

void Programmer::SendIdentify()
{
	CommandMaker maker;

	maker.start(HCODE_IDENTIFY);
	maker.finish();

	mSerial.Write(maker.get_data(), maker.get_size());
}

void Programmer::RequestMemory(uint32_t address, uint16_t size)
{
	CommandMaker maker;

	maker.start(HCODE_READ);
	maker.push_u32(address);
	maker.push_u16(size);
	maker.finish();

	mSerial.Write(maker.get_data(), maker.get_size());
}

int Programmer::WaitPacket(PACKET * pPacket, UINT nTimeout)
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



////////////////////////////////////////////////////////////////////////////////////
//

int main(int argc, char * argv[])
{
	//
	if (parse_opts(argc, argv) < 0)
	{
		show_help("iap_stm32");
		return 1;
	}

	if (check_opts() < 0)
	{
		show_help("iap_stm32");
		return 1;
	}

	//
	ImageFile * pFile = NULL;

	if (_operation == _OP_READ || _operation == _OP_WRITE)
	{
		if (_filetype == _FILE_HEX)
			pFile = new HexFile;
		else
			pFile = new BinFile;

		// _OP_WRITE -> read from file, _OP_READ -> write to file
		if (pFile->Open(_filename, _operation == _OP_WRITE ? 0 : 1) != ImageFile::PARSER_ERR_OK)
		{
			printf("file open failed : %s\n", _filename);
			return 1;
		}
	}

	// 
	Programmer prog;

	if (prog.ConnectAndIdentify(_portnum, _baudrate, _databits, _parity, _stopbits, _handshake) == 0)
	{
		if (_operation == _OP_WRITE)
		{
			prog.WriteToMemory(pFile, _address);
		}
		else if (_operation == _OP_READ)
		{
			prog.ReadFromMemory(pFile, _address, _length);
		}
		else if (_operation == _OP_ERASE)
		{
			prog.EraseMemory(_address, _length);
		}

		prog.Disconnect();
	}

	//
	if (pFile)
		delete pFile;


#if 0
	//
	char port[32];

	wsprintf(port, "\\\\.\\COM%d", _portnum);
	if (_serial.Open(port) == ERROR_SUCCESS)
	{
		_serial.Setup(_baudrate, _databits, _parity, _stopbits);
		_serial.SetupHandshaking(_handshake);
		_serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);

		_serial.Write("\x02\xAA\x00\x55", 4);
		Sleep(500);

		CommandMaker maker;

		maker.start(HCODE_IDENTIFY);
		maker.finish();

		_serial.Write(maker.get_data(), maker.get_size());

		char szData[16];
		DWORD dwRead;
		DWORD dwTick = GetTickCount();
		while (GetTickCount() - dwTick < 5000)
		{
			while (_serial.Read(&szData[0], sizeof(szData), &dwRead) == ERROR_SUCCESS && dwRead > 0)
			{
				for (DWORD i = 0; i < dwRead; i++)
					printf("%02X ", (unsigned char)szData[i]);
			}
		}
		_serial.Close();
	}
#endif


#if 0
	//
	HexFile hexFile;

	hexFile.Open("D:\\Work\\NosQuest\\eclipse-workspace\\SampleApp\\Debug\\SampleApp.hex", 0);

	if (1)
	{
		char sz[1024];
		uint8_t buf[1024];
		uint32_t offset = 0;



		while (1)
		{
			unsigned int len = sizeof(buf);
			memset(buf, 0xFF, sizeof(buf));

			if (hexFile.Read(buf, &len) != BinFile::PARSER_ERR_OK || len == 0)
				break;

			for (uint32_t i = 0; i < len; i += 16)
			{
				uint8_t * data = &buf[i];

				sprintf_s(sz, sizeof(sz), "%08X   %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X",
					offset,
					data[0x00], data[0x01], data[0x02], data[0x03], data[0x04], data[0x05], data[0x06], data[0x07],
					data[0x08], data[0x09], data[0x0A], data[0x0B], data[0x0C], data[0x0D], data[0x0E], data[0x0F]);
				printf("    %s\n", sz);

				offset += 16;
			}
		}
	}


	hexFile.Close();

	//
	BinFile binFile;

	binFile.Open("D:\\Work\\NosQuest\\eclipse-workspace\\SampleApp\\Debug\\SampleApp.bin", 0);
	printf("open: D:\\Work\\NosQuest\\eclipse-workspace\\SampleApp\\Debug\\SampleApp.bin\n");
	printf("  size -> %d\n", binFile.getSize());

	{
		char sz[1024];
		uint8_t buf[1024];
		uint32_t offset = 0;

		while (1)
		{
			unsigned int len = sizeof(buf);
			memset(buf, 0xFF, sizeof(buf));
			
			if (binFile.Read(buf, &len) != BinFile::PARSER_ERR_OK || len == 0)
			{
				//printf("r = %d, len = %d\n", r, len);
				break;
			}

			for (uint32_t i = 0; i < len; i += 16)
			{
				uint8_t * data = &buf[i];

				sprintf_s(sz, sizeof(sz), "%08X   %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X",
					offset,
					data[0x00], data[0x01], data[0x02], data[0x03], data[0x04], data[0x05], data[0x06], data[0x07],
					data[0x08], data[0x09], data[0x0A], data[0x0B], data[0x0C], data[0x0D], data[0x0E], data[0x0F]);
				printf("    %s\n", sz);

				offset += 16;
			}
		}
	}

	binFile.Close();
#endif

    return 0;
}

