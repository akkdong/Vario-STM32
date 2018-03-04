// boot.h
//

#ifndef __BOOT_H__
#define __BOOT_H__

#include "stm32f10x_conf.h"

/////////////////////////////////////////////////////////////////////////////////////////
//

#define FIRMWARE_VERSION			(0x0100)

#define RESET_FOR_BOOTLOADER		(0x4501)
#define RESET_FOR_NORMAL_BOOT		(0x8923)



/////////////////////////////////////////////////////////////////////////////////////////
//

uint16_t 	boot_readResetReason(void);
void 		boot_writeResetReason(uint16_t reason);

int32_t 	boot_checkUserApplication(void);
void		boot_jumpToUserApplication(void);


/////////////////////////////////////////////////////////////////////////////////////////
//

void 		bootcmd_sendIdentify(void);

void 		bootcmd_erasePage(uint32_t start, uint32_t end);
void 		bootcmd_eraseAll(void);

void 		bootcmd_writeMemory(uint32_t address, uint8_t * data, uint16_t dataLen);
void 		bootcmd_readMemory(uint32_t address, uint16_t size);

void 		bootcmd_jumpToUserApplication();
void 		bootcmd_resetDevice();


#endif // __BOOT_H__
