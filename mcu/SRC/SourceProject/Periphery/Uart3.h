#pragma once

#include <Uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>
#include <functional>
class Uart3
{
public:
	Uart3();
	~Uart3() {}
	;
	static Uart3 *instance() {return m_instance;};
	static void init();

	static void write(const uint8_t *buf, const uint8_t len);
	static bool isBusyDma() 
	{
		return mBusyDma;
	};
	static void setBusyDma(const bool f) { mBusyDma = f;};
	static void txEnd();
	static uint16_t getAvalibleByte();
	static int getDataRx(uint8_t *buf);
	static void pushByteRx(uint8_t byte);
	
	static std::function<void(const uint8_t)> putByte;
	
private:
	static void initUart();
	static void initDma();
	static Uart3 *m_instance;
	static const int speed = 115200;

	static void taskWrite(void *p);
	
	
	static BaseType_t xReturned;
	static xTaskHandle xHandle;
	static xQueueHandle xQueueWrite;
	
	
	static int txDma(const uint8_t *data, const uint8_t len);
	
	static bool mBusyDma; 
	static ElementUart push;
	static ElementUart pop;
	static xSemaphoreHandle xSemWrite;
	
	static int indexWriteRx;
	static int indexReadRx;
	static bool isRtosRun;
};

