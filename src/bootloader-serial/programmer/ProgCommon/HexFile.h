#pragma once

#include "ImageFile.h"


////////////////////////////////////////////////////////////////////////////////////
//

class HexFile : public ImageFile
{
public:
	HexFile();

public:
	virtual ParserError		Open(const char * filename, const char write);
	virtual ParserError		Close();

	virtual unsigned int	getSize();

	virtual ParserError		Read(void * data, unsigned int * len);
	virtual ParserError		Write(void * data, unsigned int len);

private:
	size_t			m_nDataLen, m_nOffset;
	uint8_t *		m_pData;
	uint32_t		m_nBase;
};