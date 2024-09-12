#pragma once
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>
class AdcDriver
{
public:
	AdcDriver();
	static AdcDriver *instance()
	{
		return m_instance;
	}
	
	static void init();
	static void initTimer(uint16_t smplperiod);
	static void rx();
	static uint16_t value1()
	{
		return m_value1;
	};
	static uint16_t value2()
	{
		return m_value2;
	};
	
	static void thread(void *p);
	
private:
	static AdcDriver *m_instance;
	static uint16_t m_value1;
	static uint16_t m_value2;
	static xSemaphoreHandle xSem;
	static uint16_t *pWork;
	static BaseType_t xReturned;
	static xTaskHandle xHandle;
	
	
};

