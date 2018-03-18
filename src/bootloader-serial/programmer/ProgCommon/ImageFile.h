#pragma once

////////////////////////////////////////////////////////////////////////////////////
// class ImageFile

class ImageFile
{
public:
	enum ParserError
	{
		PARSER_ERR_OK,
		PARSER_ERR_SYSTEM,
		PARSER_ERR_INVALID_FILE,
		PARSER_ERR_WRONLY,
		PARSER_ERR_RDONLY
	};
	
public:
	// open the file for read | write
	virtual ParserError		Open(const char * filename, const char write) = 0;
	// close and free the parser
	virtual ParserError		Close() = 0;

	// get the total data size
	virtual unsigned int	getSize() = 0;

	// read a block of data
	virtual ParserError		Read(void * data, unsigned int * len) = 0;
	// write a block of data
	virtual ParserError		Write(void * data, unsigned int len) = 0;
	// rewind
	virtual void			Rewind() = 0;
};
