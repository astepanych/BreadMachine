#pragma once
#include <Uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>
#include <functional>

#define uart5 Uart5::instance()

class Uart5
{
public:

	~Uart5() {}
	;
	
	static Uart5 &instance() {
		static Uart5 m_instance;
		return m_instance;
	}
	;
	void taskWrite(void *p);
	void taskRead(void *p);
	
	void init();

	int write(const uint8_t *buf, const uint8_t len);
	std::function<void(const uint8_t*, const int)> putByte;
	bool isBusyDma() 
	{
		return mBusyDma;
	}
	;
	void setBusyDma(const bool f) { mBusyDma = f; }
	;
	void txEnd();

	int txDma(const uint8_t *data, const uint8_t len);

	
private:
	Uart5();
	void initUart();
	void initDma();
	
	
	BaseType_t xReturned;
	xTaskHandle xHandle;
	xTaskHandle xHandleRead;
	xQueueHandle xQueueWrite;
	xSemaphoreHandle xSemWrite;
	
	uint16_t indexWriteRx;
	uint16_t indexReadRx;
	bool isRtosRun;
	bool mBusyDma; 
	ElementUart push;
	ElementUart pop;
	const uint32_t speed = 115200;
	uint8_t bufRx[256];
};



