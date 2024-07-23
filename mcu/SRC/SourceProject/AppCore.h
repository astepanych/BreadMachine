#pragma once
#include  <Periphery/GpioDriver.h>
#include <FreeRTOS.h>
#include <task.h>
#include <portmacro.h>
#include <Periphery/Uart1.h>
#include <Periphery/Uart3.h>
#include <DisplayDriver.h>

class AppCore
{
public:
	AppCore();
	~AppCore();

private:
	
	static void parsePackDisplay(const uint8_t *pack, const int len);
	void initHal();
	void initOsal();
	static void taskPeriodic(void *p);
	

	
	static GpioDriver *gpio;
	
	static Uart1 *uart1;

	static DisplayDriver *display;
	
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

	

	

};

