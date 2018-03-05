#pragma once

//////////////////////////////////////////////////////////////////////////
//

#define STM32F10X_HD

#ifdef DEBUG
#define ADDRESS_USER_APPLICATION			(0x08003000)
#else
#define ADDRESS_USER_APPLICATION			(0x08002000)
#endif
#define ADDRESS_FLASH						(0x08000000)
#define ADDRESS_RAM							(0x20000000)

#if defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
#define PAGE_SIZE							(0x400)    // 1 KByte
#define FLASH_SIZE							(0x20000)  // 128 KBytes
#elif defined STM32F10X_CL
#define PAGE_SIZE							(0x800)    // 2 KBytes
#define FLASH_SIZE							(0x40000)  // 256 KBytes
#elif defined STM32F10X_HD || defined (STM32F10X_HD_VL)
#define PAGE_SIZE							(0x800)    // 2 KBytes
#define FLASH_SIZE							(0x80000)  // 512 KBytes
#elif defined STM32F10X_XL
#define PAGE_SIZE							(0x800)    // 2 KBytes
#define FLASH_SIZE							(0x100000) // 1 MByte
#else
#error "Please select first the STM32 device to be used (in stm32f10x.h)"
#endif

// Compute the FLASH upload image size
#define FLASH_IMAGE_SIZE					(uint32_t) (FLASH_SIZE - (ADDRESS_USER_APPLICATION - ADDRESS_FLASH))

#define PROGRAM_SIZE						(0x400)	// 1024 bytes
#define PROGRAM_COUNT						(PAGE_SIZE / PROGRAM_SIZE)
