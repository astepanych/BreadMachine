#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>
#include <Uart.h>
#include <functional>

#define uart2 Uart1::instance()

class Uart1
{
public:
	
	~Uart1() {};
	static Uart1 &instance() {
		static Uart1 m_instance;
		return m_instance;
	};
	void taskWrite(void *p);
	void taskRead(void *p);
	void init();
	void write(uint8_t *buf, uint8_t len);
	std::function<void(const uint8_t)> putByte;
	bool isBusyDma() 
	{
		return mBusyDma;
	};
	void setBusyDma(const bool f) { mBusyDma = f; };
	void txEnd();

	void pushByteRx(uint8_t byte);
private:
	Uart1();
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

