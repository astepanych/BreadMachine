#include "Uart1.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include <string.h>


Uart1 *Uart1::m_instance;
xQueueHandle Uart1::xQueueWrite;
BaseType_t Uart1::xReturned;
xTaskHandle Uart1::xHandle;
Uart1::Uart1()
{
	m_instance = this;
}


void Uart1::init()
{
	
	xQueueWrite = xQueueCreate(SizeQueUart, sizeof(ElementUart));
	
	xReturned = xTaskCreate(
				instance()->taskWrite,       /* Function that implements the task. */
		"Uart1Write",          /* Text name for the task. */
		2048,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
	
	initUart();
	
}

void Uart1::initUart()
{
	// Запускаем тактирование
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	USART_Cmd(USART2, DISABLE);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
	
	GPIO_InitTypeDef pins;
	GPIO_StructInit(&pins);
	pins.GPIO_Mode = GPIO_Mode_AF;
	pins.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5;
	pins.GPIO_Speed = GPIO_Speed_50MHz;
	pins.GPIO_OType = GPIO_OType_PP;
	pins.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &pins);
	
	USART_InitTypeDef usart;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_BaudRate = speed;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &usart);

	USART_Cmd(USART2, ENABLE);
}


void Uart1::taskWrite(void *p)
{
	ElementUart el;
	while (true)
	{
		
		xQueueReceive(xQueueWrite, &el, portMAX_DELAY); // Ожидаем элемента в очереди
		unsigned int i = 0;
		while (i < el.len) {
			while (!USART_GetFlagStatus(USART2, USART_FLAG_TC)) ;
			USART_SendData(USART2, el.buf[i++]);
		}
	}
}

void Uart1::write(uint8_t *buf, uint8_t len)
{
	if (len > 0)
	{
		ElementUart el;
		memcpy(el.buf, buf, len); 
		el.len = len;
		xQueueSendToBack(xQueueWrite, &el, 0);
	}
	
}



