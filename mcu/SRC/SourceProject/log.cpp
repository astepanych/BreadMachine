#include <log.h>
#include <string.h>
#include <Rtc.h>
#include <stdio.h>

#define bufLogSize 32 
static uint8_t bufLog[bufLogSize];

LOG::LOG() {
	xSem = xSemaphoreCreateBinary();
	xSemaphoreGiveFromISR(xSem, NULL);
	uint8_t *p = (uint8_t*)FlashAddLog;
	while (true)
	{
		if (*p == 0xff)
		{
			break;
		}
		p++;
		indexCurrentGlobal++;
	}
}

LOG::~LOG() {
}
LOG &LOG::instance()
{
	static LOG obj;
	return obj;

}

void LOG::log(const char *msg)
{
	xSemaphoreTake(xSem, portMAX_DELAY);
	
	if (indexCurrentGlobal > FlashAddLog + 0x10000)
	{
		eraseLog();
	}
	indexCurrent = 	sprintf((char*)bufLog, "~%02d:%02d ", Rtc::instance().getHoure(), Rtc::instance().getMinute());

	for (int i = 0; i < strlen(msg); i++)
	{
		bufLog[indexCurrent++] = msg[i];
	}
	bufLog[indexCurrent++] = (uint8_t)'*';
	FLASH_Unlock();
	for (int i = 0; i < indexCurrent; i++)
	{
		FLASH_ProgramByte(indexCurrentGlobal, bufLog[i]);
		indexCurrentGlobal++;
	}
	FLASH_Lock();
	xSemaphoreGiveFromISR(xSem, NULL);
}
uint8_t LOG::getLogBuf(const uint32_t index, const uint16_t len, uint8_t *data)
{
	memcpy(data, (void*)(FlashAddLog + index), len);
	return len;
}
uint16_t LOG::getLogBufLength()
{
	return indexCurrentGlobal;
}

void LOG::eraseLog() {
	FLASH_Unlock();
	FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3);
	FLASH_Lock();
}