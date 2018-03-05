// BinFile.cpp
//

#include "stdafx.h"
#include "BinFile.h"


////////////////////////////////////////////////////////////////////////////////////
//

BinFile::BinFile() : m_fd(0), m_bWrite(0)
{

}

ImageFile::ParserError BinFile::Open(const char * filename, const char write)
{
	errno_t err;

	if (write)
	{
		if (filename[0] == '-')
		{
			m_fd = 1;
			err = 0;
		}
		else
		{
			err = _sopen_s(&m_fd, filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
		}

		m_stat.st_size = 0;
	}
	else
	{
		if (filename[0] == '-')
		{
			m_fd = 0;
			err = 0;
		}
		else
		{
			if (stat(filename, &m_stat) != 0)
				return PARSER_ERR_INVALID_FILE;

			err = _sopen_s(&m_fd, filename, O_RDONLY | O_BINARY, _SH_DENYWR, _S_IREAD);
		}
	}

	m_bWrite = write;

	return err != 0 ? PARSER_ERR_SYSTEM : PARSER_ERR_OK;
}

ImageFile::ParserError BinFile::Close()
{
	_close(m_fd);

	return PARSER_ERR_OK;
}

unsigned int BinFile::getSize()
{
	return m_stat.st_size;
}

ImageFile::ParserError BinFile::Read(void * data, unsigned int * len)
{
	if (m_bWrite) 
		return PARSER_ERR_WRONLY;

	unsigned int left = *len;

	int r;

	while (left > 0)
	{
		r = _read(m_fd, data, left);
		// If there is no data to read at all, return OK, but with zero read
		//if (r == 0 && left == *len)
		//{
		//	*len = 0;
		//	return PARSER_ERR_OK;
		//}
		if (r == 0)
			break;

		if (r < 0) 
			return PARSER_ERR_SYSTEM;

		left = left - r;
		data = (uint8_t *)data + r;
	}

	*len = *len - left;

	return PARSER_ERR_OK;
}

ImageFile::ParserError BinFile::Write(void * data, unsigned int len)
{
	if (! m_bWrite) 
		return PARSER_ERR_RDONLY;

	size_t r;

	while (len > 0)
	{
		r = _write(m_fd, data, len);

		if (r < 1) 
			return PARSER_ERR_SYSTEM;

		m_stat.st_size += r;

		len = len - r;
		data = (uint8_t *)data + r;
	}

	return PARSER_ERR_OK;
}
