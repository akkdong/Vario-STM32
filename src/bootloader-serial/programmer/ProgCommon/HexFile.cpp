// HexFile.cpp
//

#include "stdafx.h"
#include "HexFile.h"


////////////////////////////////////////////////////////////////////////////////////
//

HexFile::HexFile() : m_nDataLen(0), m_nOffset(0), m_pData(NULL), m_nBase(0)
{

}

ImageFile::ParserError HexFile::Open(const char * filename, const char write)
{
	if (write)
		return PARSER_ERR_RDONLY;

	char mark;
	int fd;
	uint8_t checksum;
	unsigned int c;
	uint32_t base = 0;
	unsigned int last_address = (unsigned int)-1;

	errno_t err = _sopen_s(&fd, filename, O_RDONLY, _SH_DENYWR, _S_IREAD);
	if (err != 0)
		return PARSER_ERR_SYSTEM;

	// read in the file
	while (_read(fd, &mark, 1) != 0) 
	{
		if (mark == '\n' || mark == '\r') continue;
		if (mark != ':')
			return PARSER_ERR_INVALID_FILE;

		char buffer[9];
		unsigned int reclen, address, type;
		uint8_t *record = NULL;

		// get the reclen, address, and type
		buffer[8] = 0;
		if (_read(fd, &buffer, 8) != 8) return PARSER_ERR_INVALID_FILE;
		if (sscanf_s(buffer, "%2x%4x%2x", &reclen, &address, &type) != 3)
		{
			_close(fd);
			return PARSER_ERR_INVALID_FILE;
		}

		// setup the checksum 
		checksum = reclen + ((address & 0xFF00) >> 8) + ((address & 0x00FF) >> 0) + type;

		switch (type)
		{
		// data record
		case 0:
			if (last_address == (unsigned int)-1)
				last_address = address;

			c = address - last_address;
			m_pData = (uint8_t *)realloc(m_pData, m_nDataLen + c + reclen);

			// if there is a gap, set it to 0xFF and increment the length
			if (c > 0)
			{
				memset(&m_pData[m_nDataLen], 0xFF, c);
				m_nDataLen += c;
			}

			last_address = address + reclen;
			record = &m_pData[m_nDataLen];
			m_nDataLen += reclen;
			break;

		// extended segment address record
		//case 2:
		//	base = 0;
		//	break;

		// extended linear address record
		//case 4:
		//	base = address;
		//	break;
		}

		buffer[2] = 0;
		for (unsigned int i = 0; i < reclen; ++i)
		{
			if (_read(fd, &buffer, 2) != 2 || sscanf_s(buffer, "%2x", &c) != 1)
			{
				_close(fd);
				return PARSER_ERR_INVALID_FILE;
			}

			// add the byte to the checksum 
			checksum += c;

			switch (type)
			{
			case 0:
				if (record != NULL)
					record[i] = c;
				else
					return PARSER_ERR_INVALID_FILE;
				break;

			case 2:
			case 4:
				base = (base << 8) | c;
				break;
			}
		}

		// read, scan, and verify the checksum
		if (_read(fd, &buffer, 2) != 2 ||
			sscanf_s(buffer, "%2x", &c) != 1 ||
			(uint8_t)(checksum + c) != 0x00)
		{
			_close(fd);
			return PARSER_ERR_INVALID_FILE;
		}

		switch (type)
		{
		// EOF
		case 1:
			_close(fd);
			return PARSER_ERR_OK;

		// address record
		//case 2: 
		//	base = base << 4;
		case 4:	
#if 0
			base = be_u32(base);
			// Reset last_address since our base changed 
			last_address = 0;

			if (m_nBase == 0)
			{
				m_nBase = base;
				break;
			}
			
			if (m_nBase == 0)
				m_nBase = base << 16;

			// we cant cope with files out of order 
			if (base < m_nBase)
			{
				_close(fd);
				return PARSER_ERR_INVALID_FILE;
			}

			// if there is a gap, enlarge and fill with zeros 
			unsigned int len = base - m_nBase;
			if (len > m_nDataLen)
			{
				m_pData = (uint8_t *)realloc(m_pData, len);
				memset(&m_pData[m_nDataLen], 0, len - m_nDataLen);
				m_nDataLen = len;
			}
#else
			m_nBase = base << 16;
#endif
			break;
		}
	}

	_close(fd);

	return PARSER_ERR_OK;
}

ImageFile::ParserError HexFile::Close()
{
	free(m_pData);

	m_pData = NULL;
	m_nDataLen = 0;
	m_nBase = 0;
	m_nOffset = 0;

	return PARSER_ERR_OK;
}

unsigned int HexFile::getSize()
{
	return m_nDataLen;
}

ImageFile::ParserError HexFile::Read(void * data, unsigned int * len)
{
	unsigned int left = m_nDataLen - m_nOffset;
	unsigned int get = left > *len ? *len : left;

	memcpy(data, &m_pData[m_nOffset], get);
	m_nOffset += get;

	*len = get;

	return PARSER_ERR_OK;
}

ImageFile::ParserError HexFile::Write(void * data, unsigned int len)
{
	return PARSER_ERR_RDONLY;
}
