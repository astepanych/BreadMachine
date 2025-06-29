#pragma once
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>
#include <functional>
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
	static float value1()
	{
		
		return (2590.0*m_value1 * 3.3 / 4095 - 330) / (1.2705 - 0.385*m_value1 * 3.3 / 4095);
	};
	static float value2()
	{
		return (2590.0*(m_value2 * 3.3 / 4095) - 330) / (1.2705 - 0.385*(m_value2 * 3.3 / 4095));
	};
	
	static void thread(void *p);
	
private:
	static AdcDriver *m_instance;
	static float m_value1;
	static float m_value2;
	static xSemaphoreHandle xSem;
	static uint16_t *pWork;
	static BaseType_t xReturned;
	static xTaskHandle xHandle;
	
	
};

