// flash.h
//

#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x_conf.h"


//////////////////////////////////////////////////////////////////////////
//

#ifdef DEBUG
#define ADDRESS_USER_APPLICATION			(0x08003000)
#else
#define ADDRESS_USER_APPLICATION			(0x08002000)
#endif // DEBUG

#define ADDRESS_FLASH						(0x08000000)
#define ADDRESS_RAM							(0x20000000)

#if defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
 #define PAGE_SIZE                         (0x400)    // 1 KByte
 #define FLASH_SIZE                        (0x20000)  // 128 KBytes
#elif defined STM32F10X_CL
 #define PAGE_SIZE                         (0x800)    // 2 KBytes
 #define FLASH_SIZE                        (0x40000)  // 256 KBytes
#elif defined STM32F10X_HD || defined (STM32F10X_HD_VL)
 #define PAGE_SIZE                         (0x800)    // 2 KBytes
 #define FLASH_SIZE                        (0x80000)  // 512 KBytes
#elif defined STM32F10X_XL
 #define PAGE_SIZE                         (0x800)    // 2 KBytes
 #define FLASH_SIZE                        (0x100000) // 1 MByte
#else
 #error "Please select first the STM32 device to be used (in stm32f10x.h)"
#endif

// Compute the FLASH upload image size
#define FLASH_IMAGE_SIZE                   (uint32_t) (FLASH_SIZE - (ADDRESS_USER_APPLICATION - ADDRESS_FLASH))



//////////////////////////////////////////////////////////////////////////
//

void		flash_init(void);
void		flash_end(void);

uint16_t	flash_erasePage(uint32_t start, uint32_t end);
uint16_t	flash_eraseAll(void);

uint16_t	flash_write(uint32_t address, void * data, uint32_t size);

//////////////////////////////////////////////////////////////////////////
//

int32_t memory_is_valid(uint32_t address, uint16_t size);



#endif // __FLASH_H__
