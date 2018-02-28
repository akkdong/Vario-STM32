// led.c
//

#include "common/led.h"

// Port numbers: 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G, ...
#define BLINK_PORT_NUMBER               (2)
#define BLINK_PIN_NUMBER                (13)
#define BLINK_ACTIVE_LOW                (1)

#define BLINK_GPIOx(_N)                 ((GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(_N)))
#define BLINK_PIN_MASK(_N)              (1 << (_N))
#define BLINK_RCC_MASKx(_N)             (RCC_APB2Periph_GPIOA << (_N))



/////////////////////////////////////////////////////////////////////////////////////////
//

static int32_t	_led_state = 0;



/////////////////////////////////////////////////////////////////////////////////////////
//

void led_init()
{
	  GPIO_InitTypeDef GPIO_InitStructure;

	  // Enable GPIO Peripheral clock
	  RCC_APB2PeriphClockCmd(BLINK_RCC_MASKx(BLINK_PORT_NUMBER), ENABLE);

	  // Configure pin in output push/pull mode
	  GPIO_InitStructure.GPIO_Pin = BLINK_PIN_MASK(BLINK_PIN_NUMBER);
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	  GPIO_Init(BLINK_GPIOx(BLINK_PORT_NUMBER), &GPIO_InitStructure);

	  // Start with led turned off
	  led_off();
}

void led_on()
{
#if (BLINK_ACTIVE_LOW)
	GPIO_ResetBits(BLINK_GPIOx(BLINK_PORT_NUMBER), BLINK_PIN_MASK(BLINK_PIN_NUMBER));
#else
	GPIO_SetBits(BLINK_GPIOx(BLINK_PORT_NUMBER), BLINK_PIN_MASK(BLINK_PIN_NUMBER));
#endif

	_led_state = 1;
}

void led_off()
{
#if (BLINK_ACTIVE_LOW)
	GPIO_SetBits(BLINK_GPIOx(BLINK_PORT_NUMBER), BLINK_PIN_MASK(BLINK_PIN_NUMBER));
#else
	GPIO_ResetBits(BLINK_GPIOx(BLINK_PORT_NUMBER), BLINK_PIN_MASK(BLINK_PIN_NUMBER));
#endif

	_led_state = 0;
}

void led_toggle()
{
	if (_led_state)
		led_off();
	else
		led_on();
}
