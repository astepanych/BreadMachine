#include "AppCore.h"
#include "queue.h"

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

static void tskControlDamper(void *p) {
	AppCore::instance().taskControlDamper();
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


	xSemTaskCtrlDamper = xSemaphoreCreateBinary();
	queDamper = xQueueCreate(SIZE_QUEUE_DAMPER, sizeof(DamperControl));
	xReturned = xTaskCreate(
	tskControlDamper,       /* Function that implements the task. */
		"CtrlDamper",          /* Text name for the task. */
		512,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY+3,/* Priority at which the task is created. */
		&xHandleTaskCtrlDamper); /* Used to pass out the created task's handle. */


}

void AppCore::taskControlLeds(void *p)
{
	int timeout = 0;
	while (true) {
		xSemaphoreTake(xSemTaskCtrlLeds, pdMS_TO_TICKS(1000));
		switch (yellowLed) {
			case LedOff:
			if (gpio->isEnableYellowLed())
				gpio->disableYellowLed();
			break;
			case LedOn:
			if (!gpio->isEnableYellowLed())
				gpio->enableYellowLed();
			break;
			case LedBlink:
			if (gpio->isEnableYellowLed()) {
				gpio->disableYellowLed();
			}
			else {
				gpio->enableYellowLed();
			}
			break;
		}

		switch (redLed) {
			case LedOff:
			if (gpio->isEnableGreenLed())
				gpio->disableGreenLed();
			break;
			case LedOn:
			if (!gpio->isEnableGreenLed())
				gpio->enableGreenLed();
			break;
			case LedBlink:
			if (gpio->isEnableGreenLed()) {
				gpio->disableGreenLed();
			}
			else {
				gpio->enableGreenLed();
			}
			break;
		}
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
		
		xSemaphoreGive(xSemAccessCtrlSound);
	}

}


void AppCore::pushEventDamper(uint16_t id, uint16_t param) {
	DamperControl cmd(id, param);
	xQueueSendFromISR(queDamper,&cmd,0);
	xSemaphoreGiveFromISR(xSemTaskCtrlDamper, NULL);
}

void AppCore::taskControlDamper(void *p)
{
	int timeout = 0; 
	int cntBeep = 0;
	DamperControl cmd;

	while (true) {
		xSemaphoreTake(xSemTaskCtrlDamper, pdMS_TO_TICKS(100));
		if (xQueueReceive(queDamper, &cmd, 0) == pdTRUE) {
			switch (cmd.idEvent) {
				 case CloseDamper:
					xTimerStop(timerOpenDamper,0);
					if (moveDamperToStartPositon() == false) {
						// TODO: Вывести ошибку - шибер не достиг нулевого положения за время таймаута
						showIconError(DamperFailure);
					}
					gpio->setPin(GpioDriver::MainHood, GpioDriver::StatePinZero);
				 break;
				 case MoveDamperOnePosition:
					 // Обработка срабатывания датчика (активный низкий уровень)
					 if ((cmd.targetPosition && m_signedStateDamper == 1) || (!cmd.targetPosition && m_signedStateDamper == -1)) {
						 // Обновляем текущее положение заслонки
						 m_stateDamper += m_signedStateDamper;

						 // Проверяем, достигли ли целевого положения или границ
						 bool isTargetPosition = (targetDamper == m_stateDamper);
						 bool isMinPosition = (m_stateDamper == 0);
						 bool isMaxPosition = (m_stateDamper >= 120);

						 if (isTargetPosition || isMinPosition || isMaxPosition) {
							 // Останавливаем движение заслонки
							 gpio->setPin(GpioDriver::PinShiberO, GpioDriver::StatePinZero);
							 gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinZero);
							 // Достигли целевой позиции или границы - останавливаемся
							 m_signedStateDamper = 0;
							 targetDamper = -1;
						 }
						 if (isMinPosition) {
							gpio->setPin(GpioDriver::MainHood, GpioDriver::StatePinZero);
						 }
					 }
					 break;
				 case OpenDamper:
					 targetDamper = cmd.targetPosition;
					 gpio->setPin(GpioDriver::MainHood, GpioDriver::StatePinOne);
					 startMoveDamper();
					 break;
				 case TimeoutEventDamper: 
					gpio->setPin(GpioDriver::PinShiberX, (GpioDriver::StatePinZero));
					gpio->setPin(GpioDriver::PinShiberO, (GpioDriver::StatePinZero));
					m_signedStateDamper = 0;
				 break;
				default:
					break;
			}
		}

	}

}



