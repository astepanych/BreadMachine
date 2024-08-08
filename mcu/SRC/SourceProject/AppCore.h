#pragma once
#include  <Periphery/GpioDriver.h>
#include <FreeRTOS.h>
#include <task.h>
#include <portmacro.h>
#include <Periphery/Uart1.h>
#include <Periphery/Uart3.h>
#include <DisplayDriver.h>
#include <vector>

#define NumItemList  (7)

struct ElementList
{
	uint16_t addrColor;
	uint16_t addrText;
};

struct Lists
{
	Lists()
	{
		indexDisplay = indexCommon = 0;
	}
	uint16_t indexDisplay;
	int16_t indexCommon;
	uint16_t maxItemDisplay;
	uint16_t maxItem;
	uint16_t currentIndex;
	uint16_t currentPos;
	ElementList list[NumItemList];
};

struct Programs
{
	std::wstring name;
};

class AppCore
{
public:
	AppCore();
	~AppCore();

private:
	
	static void parsePackDisplay(const uint16_t id, uint8_t* data);
	
	static void keyEvent(uint16_t key);
	void initHal();
	void initOsal();
	
	static void initText();
	static void taskPeriodic(void *p);
	

	
	static GpioDriver *gpio;
	
	static Uart1 *uart1;

	static DisplayDriver *display;
	
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

	static uint8_t helperBuf[256];
	
	static Lists listMain;
	static std::vector<Programs> listPrograms;
	

	

};

