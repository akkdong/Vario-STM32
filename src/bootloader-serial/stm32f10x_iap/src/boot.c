// boot.c
//

#include "common/cparser.h"
#include "common/csender.h"
#include "driver/serial.h"
#include "driver/flash.h"
#include "diag/Trace.h"
#include "boot.h"


/////////////////////////////////////////////////////////////////////////////////////////
//

typedef void (* FUNC_USERAPP)(void);



/////////////////////////////////////////////////////////////////////////////////////////
//

uint16_t boot_readResetReason(void)
{
	uint16_t value;

	// Enable clocks for the backup domain registers
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	// Disable backup register write protection
	PWR_BackupAccessCmd(ENABLE);
	// store value in DR10
	value = BKP_ReadBackupRegister(BKP_DR10);
	// Re-enable backup register write protection
	PWR_BackupAccessCmd(DISABLE);

	return value;
}

void boot_writeResetReason(uint16_t reason)
{
	// Enable clocks for the backup domain registers
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	// Disable backup register write protection
	PWR_BackupAccessCmd(ENABLE);
	// store value in DR10
	BKP_WriteBackupRegister(BKP_DR10, reason);
	// Re-enable backup register write protection
	PWR_BackupAccessCmd(DISABLE);
}

int32_t boot_checkUserApplication(void)
{
	return (((*(volatile uint32_t *)ADDRESS_USER_APPLICATION) & 0x2FFE0000) == 0x20000000) ? 1 : 0;
}

void boot_jumpToUserApplication(void)
{
	// Jump to User application
	u32 JumpAddress = *(volatile uint32_t *)(ADDRESS_USER_APPLICATION + 4);
	FUNC_USERAPP Jump2Application = (FUNC_USERAPP)JumpAddress;

	// Initialize user application's Stack Pointer
	__set_MSP(*(volatile uint32_t *)ADDRESS_USER_APPLICATION);
	Jump2Application();
}


/////////////////////////////////////////////////////////////////////////////////////////
//

void bootcmd_sendIdentify(void)
{
	//trace_printf("Dev-ID: %04X\n", DBGMCU_GetDEVID());
	//trace_printf("Rev-ID: %04X\n", DBGMCU_GetREVID());

	csender_start(DCODE_IDENTIFY, 4); // payload: device-id(2 bytes), firmware version(2 bytes)
	csender_send_u16(DBGMCU_GetDEVID());
	csender_send_u16(FIRMWARE_VERSION);
	csender_finish();
}

void bootcmd_erasePage(uint32_t start, uint32_t end)
{
	uint16_t result = flash_erasePage(start, end);

	if (result == ERROR_OK)
		csender_send_ack();
	else
		csender_send_nack(result);
}

void bootcmd_eraseAll(void)
{
	uint16_t result = flash_eraseAll();

	if (result == ERROR_OK)
		csender_send_ack();
	else
		csender_send_nack(result);
}

void bootcmd_writeMemory(uint32_t address, uint8_t * data, uint16_t dataLen)
{
	uint16_t result = flash_write(address, data, dataLen);

	if (result == ERROR_OK)
		csender_send_ack();
	else
		csender_send_nack(result);
}

void bootcmd_readMemory(uint32_t address, uint16_t size)
{
	// check memory validation
	if (memory_is_valid(address, size))
	{
		csender_start(DCODE_DUMP_MEM, size + 4); // payload: address(4 bytes), data(size bytes)
		csender_send_u32(address);
		csender_send_mem(address, size);
		csender_finish();
	}
	else
	{
		csender_send_nack(ERROR_OUT_OF_MEMORY);
	}
}

void bootcmd_resetDevice()
{
	//
	csender_send_ack();

	//
	boot_writeResetReason(RESET_FOR_BOOTLOADER);
    NVIC_SystemReset();
}

void bootcmd_jumpToUserApplication(void)
{
	//
	csender_send_ack();

	//
	boot_writeResetReason(RESET_FOR_NORMAL_BOOT);
	NVIC_SystemReset();
}
