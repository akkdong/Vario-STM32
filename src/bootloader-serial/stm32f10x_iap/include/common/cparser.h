// cparser.h
//

#ifndef __CPARSER_H__
#define __CPARSER_H__

#include "stm32f10x.h"
#include "common/command.h"


/////////////////////////////////////////////////////////////////////////////////////////
//

void 	cparser_init(void);

int32_t	cparser_update(void);
int32_t	cparser_available(void);
int32_t	cparser_getBootCommand(BOOT_CMD * cmd);


#endif // __CPARSER_H__
