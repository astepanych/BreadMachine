#include "Uart3.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"
#include <stm32f4xx_exti.h>
#include <misc.h>
#include <string.h>
#define SizeBufferRx (SizeBuffer*4)
static uint8_t gBufferRx[SizeBufferRx];
static uint8_t gBufferTx[SizeBuffer];

static void tskWrite(void *p)
{
	Uart3::instance().taskWrite(p);
}

static void tskRead(void *p)
{
	Uart3::instance().taskRead(p);
}

extern "C" 
void DMA1_Stream3_IRQHandler(void)//tx uart
{
	/* Test on DMA Stream Transfer Complete interrupt */
	Uart3::instance().txEnd();

}
extern "C"
void USART3_IRQHandler() {
	
	if (USART_GetITStatus(USART3, USART_IT_RXNE)) {
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		uint16_t byte = USART_ReceiveData(USART3);
		Uart3::instance().pushByteRx((uint8_t)byte);
	}
}


Uart3::Uart3()
{
	isRtosRun = false;
}

void Uart3::init()
{
	xSemWrite = xSemaphoreCreateBinary();
	xQueueWrite = xQueueCreate(SizeQueUart, sizeof(ElementUart));
	xReturned = xTaskCreate(
				tskWrite,       /* Function that implements the task. */
		"Uart3Write",          /* Text name for the task. */
		256,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
	
	xReturned = xTaskCreate(
		tskRead,       /* Function that implements the task. */
		"Uart3Read",          /* Text name for the task. */
		256,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY+2,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
	// Запускаем тактирование
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	initDma();
	initUart();
}

void Uart3::initDma()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Stream3);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; // Receive
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)gBufferTx ;
	DMA_InitStructure.DMA_BufferSize = (uint16_t)10;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;
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
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);
	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);

	DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);
	//DMA_Cmd(DMA1_Stream3, ENABLE);
	
	
	DMA_DeInit(DMA1_Stream1);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory; // Receive
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)gBufferRx;
	DMA_InitStructure.DMA_BufferSize = (uint16_t)SizeBufferRx;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;
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
	DMA_Init(DMA1_Stream1, &DMA_InitStructure);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
	
	DMA_Cmd(DMA1_Stream1, ENABLE);
	
}

void Uart3::initUart()
{	
	
	USART_Cmd(USART3, DISABLE);

	GPIO_InitTypeDef pins;
	GPIO_StructInit(&pins);
	pins.GPIO_Mode = GPIO_Mode_AF;
	pins.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	pins.GPIO_Speed = GPIO_Speed_50MHz;
	pins.GPIO_OType = GPIO_OType_PP;
	pins.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &pins);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);
	
	
	USART_InitTypeDef usart;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_BaudRate = speedUart;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(USART3, &usart);
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART3, ENABLE);
}


void Uart3::taskWrite(void *p)
{
	isRtosRun = true;
	while (true) {
		xQueueReceive(xQueueWrite, &pop, portMAX_DELAY); // Ожидаем элемента в очереди
		txDma(pop.buf, pop.len);
		xSemaphoreTake(xSemWrite, portMAX_DELAY);
	}
}

void Uart3::taskRead(void *p) {
	int avalibleBytes = 0, pverBytes = 0;
	while (true) {
		vTaskDelay(5 / portTICK_PERIOD_MS);
		avalibleBytes = SizeBufferRx - DMA_GetCurrDataCounter(DMA1_Stream1);
		if (avalibleBytes != pverBytes)	{
			while (pverBytes != avalibleBytes) {
				putByte(gBufferRx[pverBytes]);
				pverBytes = (pverBytes + 1) % SizeBufferRx;
			}
		}
	}
}

void Uart3::write(const uint8_t *buf, const uint8_t len)
{
	if (!isRtosRun)
		return;
	if (len > 0)
	{
		ElementUart el;
		memcpy(el.buf, buf, len); 
		el.len = len;
		xQueueSendToBack(xQueueWrite, &el, 0);
		taskYIELD();
	}
	
}

void Uart3::txEnd()
{
	if (DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3)) {
		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);
		DMA_Cmd(DMA1_Stream3, DISABLE);

		mBusyDma = false;
		if (isRtosRun)
		{
			xSemaphoreGiveFromISR(xSemWrite, NULL);
			taskYIELD();
		}
	}
}




int Uart3::txDma(const uint8_t *data, const uint8_t len)
{
	if (mBusyDma == true)
		return 1;
	
	mBusyDma = true;
	memcpy(gBufferTx, data, len);
	DMA_MemoryTargetConfig(DMA1_Stream3, (uint32_t)gBufferTx, 0); 
	DMA_SetCurrDataCounter(DMA1_Stream3, len);
	DMA_Cmd(DMA1_Stream3, ENABLE);
	return 0;
}

void Uart3::pushByteRx(uint8_t byte)
{
	putByte(byte);
}


