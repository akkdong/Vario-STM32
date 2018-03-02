#pragma once

#include "ImageFile.h"


////////////////////////////////////////////////////////////////////////////////////
//

class BinFile : public ImageFile
{
public:
	BinFile();

public:
	virtual ParserError		Open(const char * filename, const char write);
	virtual ParserError		Close();

	virtual unsigned int	getSize();

	virtual ParserError		Read(void * data, unsigned int * len);
	virtual ParserError		Write(void * data, unsigned int len);

private:
	int				m_fd;
	char			m_bWrite;
	struct stat		m_stat;
};

