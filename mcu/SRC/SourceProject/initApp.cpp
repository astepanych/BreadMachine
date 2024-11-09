
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

volatile static uint32_t CPU_IDLE = 0;

uint32_t GetCPU_IDLE(void) {
	return CPU_IDLE;
}


static uint32_t _GET_DIFF(portTickType cur, portTickType prev) {
	return (cur - prev);
}

extern "C" void vApplicationIdleHook(void)
{
	static portTickType LastTick; 
	static uint32_t count; //наш труд¤га счетчик
	static uint32_t max_count; //максимальное значение счетчика, вычисл¤етс¤ при калибровке и соответствует 100% CPU idle

	count++; //приращение счетчика

	//если прошло 1000 тиков (1 сек дл¤ моей платфрмы)
	if (_GET_DIFF(xTaskGetTickCount(), LastTick) > 5) {
		LastTick = xTaskGetTickCount();
		if (count > max_count) max_count = count; //это калибровка
		CPU_IDLE = 100 - 100 * count / max_count; //вычисл¤ем текущую загрузку
		count = 0; //обнул¤ем счетчик
	}
}

int main()
{
//	SystemClock_Config();
//	initRtc();
	AppCore::instance();
	vTaskStartScheduler();
	return 0;
}