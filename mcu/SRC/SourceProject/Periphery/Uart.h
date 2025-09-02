#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>

#define SizeBuffer 0x80//размер буфера для отправки на дисплей

struct ElementUart
{
	uint8_t len;
	uint8_t buf[SizeBuffer];
};
#define  SizeQueUart  (32) // размер очереди для отправки на дисплей

constexpr int speedUart = 115200;




