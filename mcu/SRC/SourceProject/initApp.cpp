
#include <initTasks.h>
#include <AppCore.h>
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rtc.h"

static void initRtc()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	RTC_DeInit();
	
	RTC_InitTypeDef s;
	RTC_StructInit(&s);
	RTC_Init(&s);
}

static void SystemClock_Config(void)
{
	RCC_HSEConfig(RCC_HSE_ON);
	
	ErrorStatus res =  RCC_WaitForHSEStartUp();
	
	
	RCC_PLLConfig(RCC_PLLSource_HSE, 10, 64, 8, 4);
	RCC_PLLCmd(ENABLE);
	
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	
	
	asm(" nop");
}

int main()
{
	SystemClock_Config();
//	initRtc();
	AppCore *pApp = new AppCore;
	return 0;
}