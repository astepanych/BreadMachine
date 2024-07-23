#include <AppCore.h>
#include <stdio.h>
#include <misc.h>
#include <string.h>

constexpr uint16_t versionSoft = 1001;
GpioDriver *AppCore::gpio;
Uart1 *AppCore::uart1;

DisplayDriver *AppCore::display;





AppCore::AppCore()
{
	initHal();
	initOsal();
	vTaskStartScheduler();
}

AppCore::~AppCore()
{
}

void AppCore::initHal()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	gpio = new GpioDriver;
	gpio->initModule();
	
	uart1 = new Uart1();
	
	uart1->init();
	
	display = new DisplayDriver();
	
}
void AppCore::initOsal()
{
	xReturned = xTaskCreate(
				this->taskPeriodic,       /* Function that implements the task. */
		"NAME",          /* Text name for the task. */
		512,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
	
	
	
	
}

void AppCore::parsePackDisplay(const uint8_t *pack, const int len)
{
	
}



void AppCore::taskPeriodic(void *p)
{
	int cnt = 0, len;
	char buf[256];
	//timer3->start();
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	display->sendToDisplay(CmdSoftVersion, versionSoft);
	while (true) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		
		//gpio->togglePin(GpioDriver::Led);
		//cnt++;
		//len  = sprintf(buf, "cnt = %d\n\r", cnt);
		//uart1->write((uint8_t*)buf, len);
		//uart3->write((uint8_t*)buf, len);
		//len = uart3->getDataRx((uint8_t*)buf);
		//if(len > 0)
			//uart3->write((uint8_t*)buf, len);
	}
}

