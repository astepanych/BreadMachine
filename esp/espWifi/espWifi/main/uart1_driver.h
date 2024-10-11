#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
	
/*!
 * ��� ����������� ��������� �� ������� ��� ��������� �������� ������ �� �����
 */
	typedef void(*fnxProcessRxData)(const uint8_t* data, const uint8_t length);
	void initUart(fnxProcessRxData cbFunc);
int  sendDataToUart(const uint8_t* data, const uint8_t len);
int  sendByteToUart(const uint8_t data);
#ifdef __cplusplus
}
#endif