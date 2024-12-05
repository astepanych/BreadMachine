#include <FreeRTOS.h>
#include <task.h>
#include <portmacro.h>
#include <stdint.h>
extern "C"
void task1(void *p)
{
	int cnt = 0;
	while (1)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		cnt++;
	}
}

void initTask()
{
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;
	xReturned = xTaskCreate(
                    task1,       /* Function that implements the task. */
		"NAME",          /* Text name for the task. */
		2048,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
	
	
	
	vTaskStartScheduler();
}