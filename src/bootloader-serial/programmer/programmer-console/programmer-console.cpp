// programmer-console.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Programmer.h"
#include "HexFile.h"
#include "BinFile.h"


////////////////////////////////////////////////////////////////////////////////////
//

int Print(LPCTSTR format, ...)
{
	TCHAR buf[2048];
	va_list	va;

	va_start(va, format);
	vsprintf_s(buf, format, va);
	va_end(va);

	int ret = fprintf(stdout, buf);
	fflush(stdout);

	return ret;
}



////////////////////////////////////////////////////////////////////////////////////
//

enum Operation
{
	_OP_UNDEF = 0,
	_OP_WRITE,
	_OP_DUMP,
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

uint32_t			_address = ADDRESS_USER_APPLICATION;
uint32_t			_length = 0x400;


////////////////////////////////////////////////////////////////////////////////////
//

void show_help(const char * name)
{
	Print("Usage: %s [-b rate] [-m mode] [-s address[:length]] [-v 0|1] -f filename -p portnum (w)rite|(d)ump|(e)rase\n", name);
	Print("   -b rate           serial baud rate (default 57600)\n");
	Print("   -m mode           serial port mode (default 8n1)\n");
	Print("   -s address:length start address and optionally length for read/write/erase\n");
	Print("                     address & length must be hexa-decimal\n");
	Print("   -v 1|0            verify writes (default true), it accepts yes/no or true/false\n");
	Print("   -f filename       filename for read or write\n");
	Print("   -p portnum        serial port number\n");
	Print("   w|d|e             operation: write(program) or dump or erase\n");
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
	if (_stricmp(str, "0") == 0 || _stricmp(str, "no") == 0 || _stricmp(str, "false") == 0)
		_verify = FALSE;
	else if (_stricmp(str, "1") == 0 || _stricmp(str, "yes") == 0 || _stricmp(str, "true") == 0)
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

int parse_portname(const char * str)
{
	if (_strnicmp(str, "COM", 3) == 0)
		return parse_portnum(&str[3]);

	return -1;
}

int parse_opts(int argc, char * argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if (_stricmp(argv[i], "w") == 0 || _stricmp(argv[i], "write") == 0 || _stricmp(argv[i], "program") == 0)
		{
			_operation = _OP_WRITE;
		}
		else if (_stricmp(argv[i], "d") == 0 || _stricmp(argv[i], "dump") == 0)
		{
			_operation = _OP_DUMP;
		}
		else if (_stricmp(argv[i], "e") == 0 || _stricmp(argv[i], "erase") == 0)
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
			case 'P':
				if (parse_portname(argv[i + 1]) < 0)
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

	if (_portnum <= 0)
		return -1;

	if (_filename[0] == NULL && _operation != _OP_ERASE)
		return -1;

	return 0;
}



////////////////////////////////////////////////////////////////////////////////////
//

int main(int argc, char * argv[])
{
	//
	if (parse_opts(argc, argv) < 0)
	{
		show_help("iap_stm32");
		return -1;
	}

	if (check_opts() < 0)
	{
		show_help("iap_stm32");
		return -1;
	}

	//
	ImageFile * pFile = NULL;

	if (_operation == _OP_DUMP || _operation == _OP_WRITE)
	{
		if (_filetype == _FILE_HEX)
			pFile = new HexFile;
		else
			pFile = new BinFile;

		// _OP_WRITE -> read from file, _OP_DUMP -> write to file
		if (pFile->Open(_filename, _operation == _OP_WRITE ? 0 : 1) != ImageFile::PARSER_ERR_OK)
		{
			Print("File open failed : %s\n", _filename);
			return -1;
		}
	}

	// 
	Programmer prog;
	int ret = -1;

	if (prog.ConnectAndIdentify(_portnum, _baudrate, _databits, _parity, _stopbits, _handshake) == 0)
	{
		if (_operation == _OP_WRITE)
		{
			ret = prog.Program(pFile, _address, _verify);
		}
		else if (_operation == _OP_DUMP)
		{
			ret = prog.Dump(pFile, _address, _length);
		}
		else if (_operation == _OP_ERASE)
		{
			ret = prog.Erase(_address, _length);
		}

		prog.RunUserApplication();
		prog.Disconnect();
	}

	//
	if (pFile)
		delete pFile;

    return ret;
}

