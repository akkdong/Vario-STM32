// csender.h
//

#ifndef __CSENDER_H__
#define __CSENDER_H__

#include "stm32f10x.h"
#include "common/command.h"


/////////////////////////////////////////////////////////////////////////////////////////
//

void 	csender_start(uint8_t code, uint16_t len);
void 	csender_send_u8(uint8_t data);
void 	csender_send_u16(uint16_t data);
void 	csender_send_u32(uint32_t data);
void 	csender_send_mem(uint32_t address, uint16_t size);
void 	csender_finish(void);

void	csender_send_ack(void);
void	csender_send_nack(uint16_t error);



#endif // __CSENDER_H__
