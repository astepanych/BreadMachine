#include "Timer3.h"

#include "misc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
Timer3 * Timer3::m_instance;
std::function<void()> Timer3::handlerTimeout;
Timer3::Timer3()
{
	m_instance = this;
	
}

Timer3::~Timer3()
{
}


void Timer3::init()
{
	TIM_TimeBaseInitTypeDef tim;
	NVIC_InitTypeDef nvic;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_Cmd(TIM2, DISABLE);
	tim.TIM_Prescaler = 839;
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Period = TimeoutTim3;
	tim.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &tim);
	// Настраиваем прерывание в NVIC
	nvic.NVIC_IRQChannel = TIM2_IRQn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 8;
	nvic.NVIC_IRQChannelSubPriority = 7;
	NVIC_Init(&nvic);
	TIM_Cmd(TIM2, DISABLE);
	TIM_SetAutoreload(TIM2, TimeoutTim3);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	
	
}
	
void Timer3::start()
{
	TIM2->CNT = 0;
	TIM_Cmd(TIM2, ENABLE);
}
	
void Timer3::stop()
{
	TIM2->CNT = 0;
	TIM_Cmd(TIM2, DISABLE);
}
extern "C"
void TIM2_IRQHandler()
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);   
		
		Timer3::handlerTimeout();
           
	}
}


