#include <AppCore.h>
#include <stdio.h>
#include <misc.h>
#include <string.h>

constexpr uint16_t versionSoft = 1001;
GpioDriver *AppCore::gpio;
Uart1 *AppCore::uart1;

Uart3 *AppCore::uart3;

//display
static const uint8_t cmdTouch[] = { 0x5a,0xa5,0x4,0x83,0x00, 0x16, 0x03 };
xQueueHandle AppCore::xQueueDisplay;
xSemaphoreHandle AppCore::xSemDisplay;

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
	
	uart3 = new Uart3();
	
	uart3->init();
	
	
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
	
	
	xSemDisplay = xSemaphoreCreateBinary();
	xQueueDisplay = xQueueCreate(SizeQueUart, sizeof(ElementUart));
	
	xTaskCreate(this->taskDisplay,
		"display",
		512,
		(void*)2,
		tskIDLE_PRIORITY+1,
		&xHandleDisplay); 
	
}

void AppCore::parsePackDisplay(const uint8_t *pack, const int len)
{
	
}

void AppCore::taskDisplay(void *p)
{
	ElementUart pop;
	BaseType_t res;
	uint8_t bufRcv[256];
	int len; 
	while (true)
	{
		res = xQueueReceive(xQueueDisplay, &pop, 100); // ќжидаем элемента в очереди
		//если дождались - значит это команда - шлем ее на дислей
		if (res == pdPASS) {
			uart3->write(pop.buf, pop.len);
		}
		else// иначе это событие опроса тачскрина диспле€
		{
			uart3->write(cmdTouch, sizeof(cmdTouch));
		}
		vTaskDelay(100); // ждем отправку и ответ
		len = uart3->getDataRx(bufRcv);
		if (len > 0)
		{
			parsePackDisplay(bufRcv, len);
		}
	}
}

void AppCore::taskPeriodic(void *p)
{
	int cnt = 0, len;
	char buf[256];
	//timer3->start();
	while (true) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		gpio->togglePin(GpioDriver::Led);
		cnt++;
		len  = sprintf(buf, "cnt = %d\n\r", cnt);
		uart1->write((uint8_t*)buf, len);
		//uart3->write((uint8_t*)buf, len);
		//len = uart3->getDataRx((uint8_t*)buf);
		//if(len > 0)
			//uart3->write((uint8_t*)buf, len);
	}
}

