#include "AppCore.h"

static void tskPeriodic(void *p) {
	AppCore::instance().taskPeriodic();
}
static void tskControlInPins(void *p) {
	AppCore::instance().taskControlInPins();
}

static void tskControlLeds(void *p) {
	AppCore::instance().taskControlLeds();
}
static void tskControlSound(void *p) {
	AppCore::instance().taskControlSound();
}

void AppCore::initTasks()
{
	xSemPeriodic = xSemaphoreCreateBinary();

	xReturned = xTaskCreate(
	                        tskPeriodic,       /* Function that implements the task. */
		"Period",          /* Text name for the task. */
		512,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */

#ifndef CONTROL_PIN
	xReturned = xTaskCreate(
	tskControlInPins,       /* Function that implements the task. */
		"CtrlInPins",          /* Text name for the task. */
		256,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandlePull); /* Used to pass out the created task's handle. */
#endif

	xSemTaskCtrlLeds = xSemaphoreCreateBinary();
	xSemTaskCtrlSound = xSemaphoreCreateBinary();
	xSemAccessCtrlSound = xSemaphoreCreateBinary();
	xReturned = xTaskCreate(
					tskControlLeds,       /* Function that implements the task. */
		"CrtlLeds",          /* Text name for the task. */
		256,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandleTaskCtrlLeds); /* Used to pass out the created task's handle. */
    
	xReturned = xTaskCreate(
	tskControlSound,       /* Function that implements the task. */
		"CtrlSound",          /* Text name for the task. */
		256,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandleTaksCtrlSound); /* Used to pass out the created task's handle. */

}

void AppCore::taskControlLeds(void *p)
{
	int timeout = 0;
	while (true) {
		xSemaphoreTake(xSemTaskCtrlLeds, portMAX_DELAY);

	}
}

void AppCore::taskControlSound(void *p)
{
	int timeout = 0; 
	int cntBeep = 0;
	xSemaphoreGive(xSemAccessCtrlSound);
	while (true) {
		xSemaphoreTake(xSemTaskCtrlSound, pdMS_TO_TICKS(500));

		xSemaphoreTake(xSemAccessCtrlSound, portMAX_DELAY);
		timeout++;
		if (m_statesWork.timeoutPlayAfterRun == SOUND_ON) {
			if (timeout % 2)
				gpio->disableGreenLed();
			else 
				gpio->enableGreenLed();
		}
		else {
			if (m_statesWork.timeoutPlayAfterRun == SOUND_ON_3) {
				m_statesWork.cntPlaySignal--;
				if (m_statesWork.cntPlaySignal%2) {
					gpio->enableGreenLed();
				}
				else {
					gpio->disableGreenLed();
				}
				if (m_statesWork.cntPlaySignal == 0)
					m_statesWork.timeoutPlayAfterRun = SOUND_OFF;
			}
		}
		xSemaphoreGive(xSemAccessCtrlSound);
	}

}



