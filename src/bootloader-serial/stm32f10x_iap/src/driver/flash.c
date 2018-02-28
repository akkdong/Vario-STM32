// flash.c
//

#include "common/command.h"
#include "driver/flash.h"


//////////////////////////////////////////////////////////////////////////
//

static uint32_t BlockNbr = 0, UserMemoryMask = 0;

volatile uint32_t FlashProtection = 0;



//////////////////////////////////////////////////////////////////////////
//

uint32_t mapStatusToError(FLASH_Status status)
{
	if (status == FLASH_COMPLETE)
		return ERROR_OK;

	if (status == FLASH_BUSY)
		return ERROR_FLASH_BUSY;

	if (status == FLASH_ERROR_PG)
		return ERROR_FLASH_ERROR_PG;

	if (status == FLASH_ERROR_WRP)
		return ERROR_FLASH_ERROR_WRP;

	if (status == FLASH_TIMEOUT)
		return ERROR_FLASH_TIMEOUT;

	return ERROR_GENERIC;
}


//////////////////////////////////////////////////////////////////////////
//

void flash_init(void)
{
	//
	FLASH_Unlock();

	// Get the number of block (4 or 2 pages) from where the user program will be loaded
	BlockNbr = (ADDRESS_USER_APPLICATION - ADDRESS_FLASH) >> 12;

	// Compute the mask to test if the Flash memory, where the user program will be loaded, is write protected
	UserMemoryMask = ((uint32_t)~((1 << BlockNbr) - 1));

	// Test if any page of Flash memory where program user will be loaded is write protected
	FlashProtection = ((FLASH_GetWriteProtectionOptionByte() & UserMemoryMask) != UserMemoryMask) ? 1 : 0;
}

void flash_end(void)
{
	//
	FLASH_Lock();
}


uint16_t flash_erasePage(uint32_t address)
{
	uint32_t result = ERROR_OUT_OF_MEMORY;

	if (ADDRESS_USER_APPLICATION <= address && address < ADDRESS_USER_APPLICATION + FLASH_IMAGE_SIZE)
	{
		// Clear All pending flags
		FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

		// erase the FLASH pages
		FLASH_Status status = FLASH_ErasePage(address);
		result = mapStatusToError(status);
	}

	return result;
}

uint16_t flash_eraseAll(void)
{
	FLASH_Status status = FLASH_COMPLETE;

	// Clear All pending flags
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	// erase each pages
	for (uint32_t addr = ADDRESS_USER_APPLICATION; addr < ADDRESS_USER_APPLICATION + FLASH_IMAGE_SIZE && status == FLASH_COMPLETE; addr += PAGE_SIZE)
		status = FLASH_ErasePage(addr);

	return mapStatusToError(status);
}

uint16_t flash_write(uint32_t address, void * data, uint32_t size)
{
	uint32_t result = ERROR_OUT_OF_MEMORY;

	if (memory_is_valid(address, size))
	{
		FLASH_Status status = FLASH_COMPLETE;
		uint32_t * srcPtr = (uint32_t *)data;

		// Clear All pending flags
		FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

		// program
		for (uint32_t dstAddr = address; dstAddr < address + size && status == FLASH_COMPLETE; dstAddr += 4)
			status = FLASH_ProgramWord(dstAddr, *srcPtr++);

		result = mapStatusToError(status);
	}

	return result;
}



/////////////////////////////////////////////////////////////////////////////////////////
//

int32_t memory_is_valid(uint32_t address, uint16_t size)
{
	if (ADDRESS_USER_APPLICATION <= address && address + size <= ADDRESS_USER_APPLICATION + FLASH_IMAGE_SIZE)
		return 1;

	//if (0x20000000 <= address && address + size <= 0x20000000 + RAM_SIZE)
	//	return 1;

	return 0;
}
