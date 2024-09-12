#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>

#define SizeBuffer 0x100

struct ElementUart
{
	uint8_t len;
	uint8_t buf[SizeBuffer];
};
#define  SizeQueUart  (32)

constexpr int speedUart = 115200;




