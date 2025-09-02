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
    	float avg = (m_value1 * 3.3 / 4095) / m_coeff;
    	return (2590.0*avg  - 330) / (1.2705 - 0.385*(avg));
	};
	static float value2()
	{
    	float avg = (m_value2 * 3.3 / 4095) / m_coeff;
		return (2590.0*avg - 330) / (1.2705 - 0.385*(avg));
	};
	
	static void thread(void *p);

    static void setCoeff(float newCoeff)
    {	
        m_coeff = newCoeff;
    };
	
private:
	static AdcDriver *m_instance;
	static float m_value1;
	static float m_value2;
	static xSemaphoreHandle xSem;
	static uint16_t *pWork;
	static BaseType_t xReturned;
	static xTaskHandle xHandle;
    static float m_coeff;
	
	
};

