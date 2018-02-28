/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "driver/serial.h"
#include "driver/timer.h"
#include "driver/flash.h"
#include "common/cparser.h"
#include "common/led.h"
#include "diag/Trace.h"
#include "boot.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F1 led blink sample (trace via DEBUG).
//
// In debug configurations, demonstrate how to print a greeting message
// on the trace device. In release configurations the message is
// simply discarded.
//
// Then demonstrates how to blink a led with 1 Hz, using a
// continuous loop and SysTick delays.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//
// The external clock frequency is specified as a preprocessor definition
// passed to the compiler via a command line option (see the 'C/C++ General' ->
// 'Paths and Symbols' -> the 'Symbols' tab, if you want to change it).
// The value selected during project creation was HSE_VALUE=8000000.
//
// Note: The default clock settings take the user defined HSE_VALUE and try
// to reach the maximum possible system clock. For the default 8 MHz input
// the result is guaranteed, but for other values it might not be possible,
// so please adjust the PLL settings in system/src/cmsis/system_stm32f10x.c
//

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wunused-parameter"
//#pragma GCC diagnostic ignored "-Wmissing-declarations"
//#pragma GCC diagnostic ignored "-Wreturn-type"


int main(void)
{
	// Enable BT (PB0) : Output High
	{
	  GPIO_InitTypeDef GPIO_InitStructure;

	  // Enable GPIO Peripheral clock
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	  // Configure pin in output push/pull mode
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	  GPIO_Init(GPIOB, &GPIO_InitStructure);

	  // Set PB0
	  GPIO_SetBits(GPIOB, GPIO_Pin_0);
	}

	// read reset-reason & check validation for user application
	uint16_t reason = boot_readResetReason();
	uint16_t userApp = boot_checkUserApplication();
	//trace_printf("Reset reason: %04X\n", reason);
	//trace_printf("User application: %s\n", userApp ? "Valid" : "Invalid");

	if (boot_readResetReason() == RESET_FOR_NORMAL_BOOT)
	{
		// normal boot -> just jump to user application
		//trace_printf("jump to user program!!");
		boot_writeResetReason(0);
		boot_jumpToUserApplication();

		while(1); // never come here
	}

	//
	timer_ticks_t tickBoot;
	timer_ticks_t tickBlink;
	timer_ticks_t tickWaitCommand;

	//
	serial_init();
	timer_init();
	flash_init();
	cparser_init();
	led_init();

	// Send a greeting to the trace device (skipped on Release).
	//trace_puts("STM32 Bootloader started!!");
	// At this stage the system clock should have already been configured at high speed.
	//trace_printf("System clock: %u Hz\n", SystemCoreClock);

	// check reset-reason
	//
	tickWaitCommand = (timer_ticks_t)-1;

	if (reason != RESET_FOR_BOOTLOADER && userApp)
	{
		// 2000ms -> 2s : start user program after 2 seconds
		tickWaitCommand = (timer_ticks_t)2000;
	}
	// else : invalid user app or case of reset for bootloader -> goto boot mode

	// clear reset reason
	boot_writeResetReason(0);

	//
	tickBoot = tickBlink = timer_getTick();

	//
	while (1)
	{
		if (tickWaitCommand != (timer_ticks_t)-1 && (timer_getTick() - tickBoot) > tickWaitCommand)
		{
			//
			//trace_printf("reset to start user program!!");
			boot_writeResetReason(RESET_FOR_NORMAL_BOOT);
			NVIC_SystemReset();

			while(1);

			//
			break;
		}

		// parse incomming serial data
		cparser_update();

		// process if any command is available
		if (cparser_available())
		{
			BOOT_CMD cmd;

			cparser_getBootCommand(&cmd);
			tickWaitCommand = (timer_ticks_t)-1;

			switch(cmd.code)
			{
			case HCODE_IDENTIFY:
				//trace_printf("BOOT_CMD: Identify\n");
				bootcmd_sendIdentify();
				break;
			case HCODE_ERASE :
				//trace_printf("BOOT_CMD: Erase 0x%08X\n", cmd.a.addr);
				bootcmd_erasePage(cmd.a.addr);
				break;
			case HCODE_ERASE_ALL :
				//trace_printf("BOOT_CMD: Erase All\n");
				bootcmd_eraseAll();
				break;
			case HCODE_WRITE :
				//trace_printf("BOOT_CMD: Write 0x%08X\n", cmd.w.addr);
				bootcmd_writeMemory(cmd.w.addr, &cmd.w.data[0], cmd.payloadLen - sizeof(cmd.w.addr));
				break;
			case HCODE_READ :
				//trace_printf("BOOT_CMD: Read from 0x%08X to 0x%08X (0x%04X)\n", cmd.r.addr, cmd.r.addr + cmd.r.size, cmd.r.size);
				bootcmd_readMemory(cmd.r.addr, cmd.r.size);
				break;
			case HCODE_RESET :
				//trace_printf("BOOT_CMD: Reset\n");
				bootcmd_resetDevice();
				break;
			case HCODE_START : // Jump to User Program
				//trace_printf("BOOT_CMD: Start\n");
				bootcmd_jumpToUserApplication();
				break;
			}
		}

		//
		if (timer_getTick()- tickBlink > BLINK_INTERVAL_FAST)
		{
			tickBlink = timer_getTick();
			led_toggle();
		}
	}

	// restore all peripheral to reset state
	// ...

	// start user application
	boot_jumpToUserApplication();
	while(1); // never come here
}


//#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
