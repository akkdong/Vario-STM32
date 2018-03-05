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

	/*
	static inline const char* parser_errstr(ParserError err) {
		switch (err) {
		case PARSER_ERR_OK: return "OK";
		case PARSER_ERR_SYSTEM: return "System Error";
		case PARSER_ERR_INVALID_FILE: return "Invalid File";
		case PARSER_ERR_WRONLY: return "Parser can only write";
		case PARSER_ERR_RDONLY: return "Parser can only read";
		default:
			return "Unknown Error";
		}
	}
	*/
};
