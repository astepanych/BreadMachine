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

Uart3 *Uart3::m_instance;
xQueueHandle Uart3::xQueueWrite;
BaseType_t Uart3::xReturned;
xTaskHandle Uart3::xHandle;

bool Uart3::mBusyDma; 
ElementUart Uart3::push;
ElementUart Uart3::pop;
xSemaphoreHandle Uart3::xSemWrite;

int Uart3::indexWriteRx = 0;
int Uart3::indexReadRx = 0;
bool Uart3::isRtosRun;

Uart3::Uart3()
{
	m_instance = this;
	isRtosRun = false;
}

void Uart3::init()
{
	xSemWrite = xSemaphoreCreateBinary();
	xQueueWrite = xQueueCreate(SizeQueUart, sizeof(ElementUart));
	xReturned = xTaskCreate(
				instance()->taskWrite,       /* Function that implements the task. */
		"Uart3Write",          /* Text name for the task. */
		256,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
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

	
	
}

void Uart3::initUart()
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 9;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	
	
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
	usart.USART_BaudRate = speed;
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

extern "C" 
void DMA1_Stream3_IRQHandler(void)//tx uart
{
	/* Test on DMA Stream Transfer Complete interrupt */
	Uart3::instance()->txEnd();

}
extern "C"
void USART3_IRQHandler() {
	
	if (USART_GetITStatus(USART3, USART_IT_RXNE)) {
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		uint16_t byte = USART_ReceiveData(USART3);
		Uart3::instance()->pushByteRx((uint8_t)byte);
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
	gBufferRx[indexWriteRx] = byte;
	indexWriteRx = (indexWriteRx + 1) % SizeBufferRx;
}

int Uart3::getDataRx(uint8_t *buf) {
	int len = 0;
	if(buf == nullptr)
		return -1;
	if (indexReadRx != indexWriteRx) {
		while (indexReadRx != indexWriteRx) {
			buf[len] = gBufferRx[indexReadRx];
			len++;
			indexReadRx = (indexReadRx + 1) % SizeBufferRx;
		}
	}
	return len;
}
