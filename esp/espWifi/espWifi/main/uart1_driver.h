#pragma once
#include "common_project.h"

#ifdef __cplusplus
extern "C" {
#endif
	

void initUart(fnxProcessRxData cbFunc);
int  sendDataToUart(const uint8_t* data, const uint8_t len);
int  sendByteToUart(const uint8_t data);
#ifdef __cplusplus
}
#endif