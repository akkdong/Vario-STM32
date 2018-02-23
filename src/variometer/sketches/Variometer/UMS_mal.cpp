// UMS_mal.cpp
//

#include "UMS_mal.h"
#include "Sd2CardEx.h"


////////////////////////////////////////////////////////////////////////////////////
//

uint32_t MAL_massBlockCount[2];
uint32_t MAL_massBlockSize[2];


void usb_mal_init(uint32_t numberOfBlocks)
{
	MAL_massBlockCount[0] = numberOfBlocks;
	MAL_massBlockCount[1] = 0;
	MAL_massBlockSize[0] = 512;
	MAL_massBlockSize[1] = 0;	
}



////////////////////////////////////////////////////////////////////////////////////
//

extern "C" uint16_t usb_mass_mal_init(uint8_t lun)
{
	return 0;
}

extern "C" uint16_t usb_mass_mal_get_status(uint8_t lun)
{
	return SdCard.errorCode();
}

extern "C" uint16_t usb_mass_mal_write_memory(uint8_t lun, uint32_t memoryOffset, uint8_t *writebuff, uint16_t transferLength)
{
	uint32_t block = memoryOffset / 512;

	if (lun != 0)
		return USB_MASS_MAL_FAIL;

	if (SdCard.writeBlock(block, writebuff))
		return USB_MASS_MAL_SUCCESS;

	return USB_MASS_MAL_FAIL;
}

extern "C" uint16_t usb_mass_mal_read_memory(uint8_t lun, uint32_t memoryOffset, uint8_t *readbuff, uint16_t transferLength)
{
	if (lun != 0)
		return USB_MASS_MAL_FAIL;

	if (SdCard.readBlock(memoryOffset / 512, readbuff))
		return USB_MASS_MAL_SUCCESS;

	return USB_MASS_MAL_FAIL;
}

extern "C" void usb_mass_mal_format()
{
}
	
