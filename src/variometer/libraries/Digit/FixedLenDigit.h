// FixedLenDigit.h
//

#ifndef __FIXEDLENDIGIT_H__
#define __FIXEDLENDIGIT_H__

/////////////////////////////////////////////////////////////////////////////
// class FixedLenDigit

class FixedLenDigit
{
public:
	FixedLenDigit();
	
public:
	int32_t			begin(uint32_t v, int32_t n);
	
	int32_t			available();
	int32_t			read();

private:
	uint32_t		powi(uint32_t base, int32_t exp);
	
private:
	uint32_t		value;
	int32_t			pos;
	int32_t			length;
	
	static uint32_t	powTable[];
};


#endif // __FIXEDLENDIGIT_H__
