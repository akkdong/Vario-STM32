// serial.c
//

#include "common/ring.h"
#include "driver/serial.h"


//////////////////////////////////////////////////////////////////////////
//

#define SERIAL_BAUDRATE			(57600) // (115200), (57600)

static RING_BUFFER rb;


//////////////////////////////////////////////////////////////////////////
//

void serial_init(void)
{
    /* USART configuration structure for USART1 */
    USART_InitTypeDef USART_InitStructure;
    /* Bit configuration structure for GPIOA PIN9 and PIN10 */
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable clock for USART1, AFIO and GPIOA */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

    /* GPIOA PIN9 alternative function Tx */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* GPIOA PIN9 alternative function Rx */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Enable USART1 */
    USART_Cmd(USART1, ENABLE);
    /* Baud rate 9600, 8-bit data, One stop bit
     * No parity, Do both Rx and Tx, No HW flow control
     */
    USART_InitStructure.USART_BaudRate = SERIAL_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    /* Configure USART1 */
    USART_Init(USART1, &USART_InitStructure);
    /* Enable RXNE interrupt */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    /* Enable USART1 global interrupt */
    NVIC_EnableIRQ(USART1_IRQn);

    //
    ring_init(&rb);
}

void serial_end(void)
{
	/* Disable USART1 global interrupt */
	NVIC_DisableIRQ(USART1_IRQn);
	/* Disable RXNE interrupt */
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	/* Disable USART1 */
	USART_Cmd(USART1, DISABLE);

	/* Disable clock for USART1, AFIO and GPIOA */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, DISABLE);
}

int32_t serial_available(void)
{
	return ! ring_isEmpty(&rb);
}

int32_t serial_read(void)
{
	return ring_pop(&rb);
}

void serial_write(int32_t ch)
{
	USART_SendData(USART1, ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

void serial_puts(char * str)
{
	while (*str)
		serial_write(*str++);
}

/*
 *
 *
 *
 */

void USART1_IRQHandler(void)
{
    while (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    	ring_push(&rb, USART_ReceiveData(USART1));
}
