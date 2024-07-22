#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>
#include <Uart.h>


class Uart1
{
public:
	Uart1();
	~Uart1() {};
	static Uart1 *instance() {return m_instance;};
	static void init();

	static void write(uint8_t *buf, uint8_t len);
private:
	static void initUart();
	static Uart1 *m_instance;
	static const int speed = 115200;
	
	static void taskWrite(void *p);

	static BaseType_t xReturned;
	static xTaskHandle xHandle;
	static xQueueHandle xQueueWrite;
	static xSemaphoreHandle xSemWrite;
	
	
		
};

