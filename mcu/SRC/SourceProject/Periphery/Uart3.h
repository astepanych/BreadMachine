#pragma once

#include <Uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>
#include <functional>

#define uart3 Uart3::instance()

class Uart3
{
public:

	~Uart3() {};
	
	static Uart3 &instance() {
		static Uart3 m_instance;
		return m_instance;
	};
	void taskWrite(void *p);
	void taskRead(void *p);
	
	void init();

	void write(const uint8_t *buf, const uint8_t len);
	std::function<void(const uint8_t)> putByte;
	bool isBusyDma() 
	{
		return mBusyDma;
	};
	void setBusyDma(const bool f) { mBusyDma = f; };
	void txEnd();

	void pushByteRx(uint8_t byte);
	

	
private:
	Uart3();
	void initUart();
	void initDma();
	int txDma(const uint8_t *data, const uint8_t len);
	
	BaseType_t xReturned;
	xTaskHandle xHandle;
	xTaskHandle xHandleRead;
	xQueueHandle xQueueWrite;
	xSemaphoreHandle xSemWrite;
	
	int indexWriteRx;
	int indexReadRx;
	bool isRtosRun;
	bool mBusyDma; 
	ElementUart push;
	ElementUart pop;
};

