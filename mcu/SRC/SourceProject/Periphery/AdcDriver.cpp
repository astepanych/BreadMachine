#include "AdcDriver.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dma.h"
#include <stm32f4xx_exti.h>
#include <misc.h>
#include "stm32f4xx_tim.h"


#define ELFR_CFFT_LENGTH 256
#define AIN_CHANNELS 2

uint16_t AdcBuffer[ELFR_CFFT_LENGTH * AIN_CHANNELS];
uint16_t AdcBuffer1[ELFR_CFFT_LENGTH * AIN_CHANNELS];
BaseType_t AdcDriver::xReturned;
xTaskHandle AdcDriver::xHandle;
xSemaphoreHandle AdcDriver::xSem;
float AdcDriver::m_value1;
float AdcDriver::m_value2;

AdcDriver *AdcDriver::m_instance;
uint16_t *AdcDriver::pWork;

AdcDriver::AdcDriver()
{
	m_instance = this;
}


void AdcDriver::init()
{
	xSem = xSemaphoreCreateBinary();
	xReturned = xTaskCreate(
		instance()->thread,       /* Function that implements the task. */
		"adc",          /* Text name for the task. */
		512,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY+2,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
	pWork = AdcBuffer;
	
	initTimer(500);
	ADC_InitTypeDef        ADC_InitStructure;
	ADC_CommonInitTypeDef  ADC_CommonInitStructure;
	DMA_InitTypeDef        DMA_InitStructure;
	GPIO_InitTypeDef       GPIO_InitStructure;
	NVIC_InitTypeDef       NVIC_InitStructure;
 
	/* Enable ADC3, DMA2 and GPIO clocks ****************************************/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
 
	/* NVIC DMA interrupt setup - priority is 1, below sampling clock int. priority */
	NVIC_InitStructure.NVIC_IRQChannel                   = DMA2_Stream2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 11;
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
 
#ifdef DEBUG_ADC_INTERRUPT
	/* NVIC ADC interrupt setup - priority is 1, below sampling clock int. priority */
	NVIC_InitStructure.NVIC_IRQChannel                   = ADC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
 
	/* DMA2 Stream0 channel2 configuration **************************************/
	DMA_InitStructure.DMA_Channel            = DMA_Channel_1;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC2->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr    = (uint32_t)AdcBuffer;
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize         = ELFR_CFFT_LENGTH * AIN_CHANNELS;
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode           = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);
	DMA_DoubleBufferModeConfig(DMA2_Stream2, (uint32_t)AdcBuffer1, DMA_Memory_1);
	DMA_DoubleBufferModeCmd(DMA2_Stream2, ENABLE);
	
	DMA_Cmd(DMA2_Stream2, ENABLE);
 
	/* Configure ADC3 Channels 10 + 12 pin as analog input **********************/
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
 
	/* ADC Common Init **********************************************************/
	ADC_CommonInitStructure.ADC_Mode             = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler        = ADC_Prescaler_Div2; // CLK = (APB2_CLK / 2)
	ADC_CommonInitStructure.ADC_DMAAccessMode    = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_10Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
 
	/* ADC3 Init ****************************************************************/
	/* the conversion start is triggered by SW from the timer interrupt TIM3 */
	ADC_InitStructure.ADC_Resolution           = ADC_Resolution_12b;
	ADC_InitStructure.ADC_DataAlign            = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanConvMode         = ENABLE; // DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode   = DISABLE;
	ADC_InitStructure.ADC_NbrOfConversion      = AIN_CHANNELS;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv     = ADC_ExternalTrigConv_T3_TRGO;
	ADC_Init(ADC2, &ADC_InitStructure);
 
	/* ADC3 regular channel7 configuration *************************************/
	ADC_RegularChannelConfig(ADC2, ADC_Channel_8, 1, ADC_SampleTime_112Cycles);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_9, 2, ADC_SampleTime_112Cycles);
 
	ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);
	ADC_DMACmd(ADC2, ENABLE); /* Enable ADC3 DMA  */
	ADC_Cmd(ADC2, ENABLE); /* Enable ADC3      */
	TIM_Cmd(TIM3, ENABLE); /* TIM3 enable counter, to trigger ADC */
	DMA_ITConfig(DMA2_Stream2, DMA_IT_HT, ENABLE); // half transfer interrupt
	DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE); /* interrupt notification when the buffer full */
 
#ifdef DEBUG_ADC_INTERRUPT
	ADC_ITConfig(ADC3, ADC_IT_EOC, ENABLE); /* interrupt notification after sampling sequence */
#endif

}

void  AdcDriver::initTimer(uint16_t smplperiod)
{
	//  uint16_t                 PrescalerValue = 0;
	//  NVIC_InitTypeDef         NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef        TIM_OCInitStructure;
 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); /* TIM3 clock enable */
 
	/* TIM3 Configuration as trigger for the ADC */
	TIM_DeInit(TIM3);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_OCStructInit(&TIM_OCInitStructure);
 
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler     = (SystemCoreClock / 1000000) - 1; // 1.0 MHz
	TIM_TimeBaseStructure.TIM_Period        = smplperiod - 1; // 1 MHz/period = smpl.freq
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
	TIM_ARRPreloadConfig(TIM3, ENABLE);
}
#include "GpioDriver.h"
void AdcDriver::rx()
{
	
	if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF2)) {
		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF2);
	//	GpioDriver::instace()->togglePin(GpioDriver::PinGreen);
		if (DMA_GetCurrentMemoryTarget(DMA2_Stream2) == DMA_Memory_0)
			pWork = AdcBuffer1;
		else 
			pWork = AdcBuffer;
		xSemaphoreGiveFromISR(xSem, NULL);
	 }
	
	if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_HTIF2)) {
		DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_HTIF2);
		if (DMA_GetCurrentMemoryTarget(DMA2_Stream2) == DMA_Memory_0)
		{
			DMA_MemoryTargetConfig(DMA2_Stream2, (uint32_t)AdcBuffer1, DMA_Memory_1);
		}
		else
		{
			DMA_MemoryTargetConfig(DMA2_Stream2, (uint32_t)AdcBuffer, DMA_Memory_0);
		}
	}
		

	
}
#include <vector>
std::vector<uint16_t> workBuf;

void AdcDriver::thread(void *p)
{
	uint32_t v1, v2;
	const uint32_t lenWork = ELFR_CFFT_LENGTH * 5;
	workBuf.reserve(2*lenWork);
	while (true)
	{
		xSemaphoreTake(xSem, portMAX_DELAY);

		workBuf.insert(workBuf.end(), pWork, pWork + ELFR_CFFT_LENGTH*2);
		uint32_t curLen = workBuf.size();
		if (curLen < 2*lenWork)
			continue;
		v1 = v2 = 0;
		for (int i = 0; i < lenWork; i++)
		{
			v2 += workBuf[2*i + 1];
			v1 += workBuf[2*i];
		}
		workBuf.erase(workBuf.begin(), workBuf.begin() + ELFR_CFFT_LENGTH * 2);
		m_value2 = (v2 / 11.0) / lenWork;
		m_value1 = (v1 / 11.0) / lenWork;
		//m_value1 = v1 / ELFR_CFFT_LENGTH;
		
	}
		
}

extern "C" 
void DMA2_Stream2_IRQHandler(void)//rx dma
{
	/* Test on DMA Stream Transfer Complete interrupt */
	AdcDriver::instance()->rx();

}
