#include "Uart5.h"


#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"
#include <stm32f4xx_exti.h>
#include <misc.h>
#include <string.h>
#include <version.h>

#define SizeBufferRx (SizeBuffer*4)
static uint8_t gBufferRx5[SizeBufferRx];
static uint8_t gBufferTx5[SizeBuffer];






static void tskWrite(void *p)
{
	Uart5::instance().taskWrite(p);
}

static void tskRead(void *p)
{
	Uart5::instance().taskRead(p);
}

extern "C" 
void DMA1_Stream7_IRQHandler(void)//tx uart
{
	/* Test on DMA Stream Transfer Complete interrupt */
	Uart5::instance().txEnd();

}



Uart5::Uart5()
{
	isRtosRun = false;
}

void Uart5::init()
{
	xSemWrite = xSemaphoreCreateBinary();
	xQueueWrite = xQueueCreate(SizeQueUart, sizeof(ElementUart));
	xReturned = xTaskCreate(
				tskWrite,       /* Function that implements the task. */
		"Uart5Write",          /* Text name for the task. */
		256,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
	
	xReturned = xTaskCreate(
		tskRead,       /* Function that implements the task. */
		"Uart5Read",          /* Text name for the task. */
		256,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY+2,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
	// Запускаем тактирование
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	initDma();
	initUart();


}

void Uart5::initDma()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 12;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//dma for TX uart5
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Stream7);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; // Receive
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)gBufferTx5;
	DMA_InitStructure.DMA_BufferSize = (uint16_t)10;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART5->DR;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream7, &DMA_InitStructure);
	USART_DMACmd(UART5, USART_DMAReq_Tx, ENABLE);
	
	DMA_ITConfig(DMA1_Stream7, DMA_IT_TC, ENABLE);

	DMA_ClearITPendingBit(DMA1_Stream7, DMA_IT_TCIF7);
	//DMA_Cmd(DMA1_Stream3, ENABLE);
	
	//dma rx for uart5
	DMA_DeInit(DMA1_Stream0);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory; // Receive
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)gBufferRx5;
	DMA_InitStructure.DMA_BufferSize = (uint16_t)SizeBufferRx;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART5->DR;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream0, &DMA_InitStructure);
	USART_DMACmd(UART5, USART_DMAReq_Rx, ENABLE);
	
	DMA_Cmd(DMA1_Stream0, ENABLE);
	
}

void Uart5::initUart()
{	
	
	USART_Cmd(UART5, DISABLE);

	GPIO_InitTypeDef pins;
	GPIO_StructInit(&pins);
	pins.GPIO_Mode = GPIO_Mode_AF;
	pins.GPIO_Pin = GPIO_Pin_12;
	pins.GPIO_Speed = GPIO_Speed_50MHz;
	pins.GPIO_OType = GPIO_OType_PP;
	pins.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &pins);
	pins.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &pins);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);
	
	
	USART_InitTypeDef usart;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_BaudRate = speed;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(UART5, &usart);
	USART_ClearITPendingBit(UART5, USART_IT_RXNE);
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	USART_Cmd(UART5, ENABLE);
}


void Uart5::taskWrite(void *p)
{

	while (true) {
		xQueueReceive(xQueueWrite, &pop, portMAX_DELAY); // Ожидаем элемента в очереди
		txDma(pop.buf, pop.len);
		xSemaphoreTake(xSemWrite, portMAX_DELAY);
	}
}

void Uart5::taskRead(void *p) {
	isRtosRun = true;
	int avalibleBytes = 0, pverBytes = 0, index = 0;
	while (true) {
		vTaskDelay(5 / portTICK_PERIOD_MS);
		avalibleBytes = SizeBufferRx - DMA_GetCurrDataCounter(DMA1_Stream0);
		if (avalibleBytes != pverBytes) {
			index = 0;
			while (pverBytes != avalibleBytes) {
				bufRx[index] = gBufferRx5[pverBytes];
				index++;
				pverBytes = (pverBytes + 1) % SizeBufferRx;
			}
			putByte(bufRx, index);
		}
	}
}

int Uart5::write(const uint8_t *buf, const uint8_t len)
{
	if (!isRtosRun)
		return 0;
	if (len > 0)
	{
		ElementUart el;
		memcpy(el.buf, buf, len); 
		el.len = len;
		txDma(el.buf, el.len);
		return 1;
	}
	return 0;
}

void Uart5::txEnd()
{
	if (DMA_GetITStatus(DMA1_Stream7, DMA_IT_TCIF7)) {
		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit(DMA1_Stream7, DMA_IT_TCIF7);
		DMA_Cmd(DMA1_Stream7, DISABLE);

		mBusyDma = false;
		if (isRtosRun)
		{
			xSemaphoreGiveFromISR(xSemWrite, NULL);
			taskYIELD();
		}
	}
}




int Uart5::txDma(const uint8_t *data, const uint8_t len)
{
	if (mBusyDma == true)
		return 1;
	
	mBusyDma = true;
	memcpy(gBufferTx5, data, len);
	DMA_MemoryTargetConfig(DMA1_Stream7, (uint32_t)gBufferTx5, 0); 
	DMA_SetCurrDataCounter(DMA1_Stream7, len);
	DMA_Cmd(DMA1_Stream7, ENABLE);
	return 0;
}



