// FixedLenDigit.cpp
//

#include <Arduino.h>

#include "FixedLenDigit.h"

/////////////////////////////////////////////////////////////////////////////
// class FixedLenDigit

uint32_t FixedLenDigit::powTable[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };

FixedLenDigit::FixedLenDigit() : value(0), pos(0), length(-1)
{
}

int32_t	FixedLenDigit::begin(uint32_t v, int32_t n)
{
	if (n < 1 || 8 < n)
		return 0;
	
	// truncate if value is too big
	uint32_t div = powTable[n]; // powi(10, n);
	if (div < v)
		v = v - (v / div * div);
	
	value = v;
	pos = 0;
	length = n;
}

int32_t	FixedLenDigit::available()
{
	return pos < length ? true : false;
}

int32_t	FixedLenDigit::read()
{
	if (pos >= length)
		return -1;
	
	int div = powTable[length - pos - 1]; // powi(10, length - pos - 1);
	int num = value / div;
	
	value = value - (value / div * div);
	pos += 1;
	
	return '0' + num;
}

uint32_t FixedLenDigit::powi(uint32_t base, int32_t exp)
{
	uint32_t value = 1;
	
	for (int32_t i = 0; i < exp; i++)
		value = value * base;
	
	return value;
}