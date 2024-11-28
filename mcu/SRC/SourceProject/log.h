#pragma once
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>
#include <stm32f4xx_flash.h>

#define FlashAddLog 0x08020000

class LOG {
public:
	LOG();
	~LOG();
	void log(const char *msg);
	uint8_t getLogBuf(const uint32_t index, const uint16_t len, uint8_t *data);
	uint16_t getLogBufLength();
	static LOG &instance();
	void eraseLog();
	

private:
	xSemaphoreHandle xSem;
	uint16_t indexCurrent{0};
	uint32_t indexCurrentGlobal{FlashAddLog};
	
	
	
};

