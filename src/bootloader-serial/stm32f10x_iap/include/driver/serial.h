// serial.h
//

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "stm32f10x_conf.h"


//////////////////////////////////////////////////////////////////////////
//

void 		serial_init(void);

int32_t 	serial_available(void);
int32_t 	serial_read(void);
void 		serial_write(int32_t ch);

void		serial_puts(char * str);


#endif // __SERIAL_H__
