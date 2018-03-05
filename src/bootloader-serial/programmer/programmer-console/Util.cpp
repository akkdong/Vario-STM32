// Util.cpp
//

#include "Util.h"

// detect CPU endian
char cpu_le()
{
	const uint32_t cpu_le_test = 0x12345678;
	return ((const unsigned char*)&cpu_le_test)[0] == 0x78;
}

uint32_t be_u32(const uint32_t v)
{
	if (cpu_le())
		return	((v & 0xFF000000) >> 24) |
				((v & 0x00FF0000) >> 8) |
				((v & 0x0000FF00) << 8) |
				((v & 0x000000FF) << 24);
	return v;
}

uint32_t le_u32(const uint32_t v)
{
	if (!cpu_le())
		return  ((v & 0xFF000000) >> 24) |
				((v & 0x00FF0000) >> 8) |
				((v & 0x0000FF00) << 8) |
				((v & 0x000000FF) << 24);
	return v;
}
