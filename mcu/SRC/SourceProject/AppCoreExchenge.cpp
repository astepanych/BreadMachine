#include <AppCore.h>
#include <stdio.h>
#include <misc.h>
#include <string.h>
#include <string>
#include <stm32f4xx_flash.h>

#include "typedef.h"
#include <time.h>
#include <math.h>
#include <version.h>

#define BOOT_ADDRESS    0x08040000//����� ������ ���������


static void tskExchange(void *p) {
	AppCore::instance().taskExchange();
}

void AppCore::initExchange()
{
	queExchange = xQueueCreate(32, sizeof(PackageNetworkFormat));
	xReturned = xTaskCreate(
	                        tskExchange,       /* Function that implements the task. */
	                        "exch",          /* Text name for the task. */
	                        512,      /* Stack size in words, not bytes. */
	                        (void *) 1,    /* Parameter passed into the task. */
	                        tskIDLE_PRIORITY,/* Priority at which the task is created. */
							&xHandleExchange); /* Used to pass out the created task's handle. */
	
	
	
	objDataExchenge.procPack =[=](PackageNetworkFormat & p) {
		xQueueSend(queExchange, &p, 0);
	};
	
	uart5.init();
	uart5.putByte = [=](const uint8_t *byte, const int len) {
		objDataExchenge.putByte(byte, len);
	};
	
	objDataExchenge.sendBytes = [=](const uint8_t *buf, const uint8_t len) {
		uart5.txDma(buf, len);
	};
}

void AppCore::taskExchange(void *p)
{
	PackageNetworkFormat package;
	while (true)
	{
		xQueueReceive(queExchange, &package, portMAX_DELAY);
		procUartData(package);
	}
}




void AppCore::procUartData(const PackageNetworkFormat&p) {
	
	switch (p.cmdId) {
		case IdSoftVersion:
			objDataExchenge.sendPackage(IdSoftVersion, p.msgType, sizeof(versionSoft), (uint8_t*)&versionSoft);
			break;
		case IdStartBootloader: {
			typedef void(*fnc_ptr)(void);
			/* Function pointer to the address of the user application. */
			fnc_ptr jump_to_app;
 
			jump_to_app = (fnc_ptr)(*(volatile uint32_t*)(BOOT_ADDRESS + 4u));
 
			//HAL_DeInit();
			RCC->APB1RSTR = 0xFFFFFFFFU;
			RCC->APB1RSTR = 0x00;
			RCC->APB2RSTR = 0xFFFFFFFFU;
			RCC->APB2RSTR = 0x00;
 
			//SysTick DeInit
			SysTick->CTRL = 0;
			SysTick->VAL = 0;
			SysTick->LOAD = 0;
 
			__disable_irq();
 
			//NVIC DeInit
			__set_BASEPRI(0);
			__set_CONTROL(0);
			NVIC->ICER[0] = 0xFFFFFFFF;
			NVIC->ICPR[0] = 0xFFFFFFFF;
			NVIC->ICER[1] = 0xFFFFFFFF;
			NVIC->ICPR[1] = 0xFFFFFFFF;
			NVIC->ICER[2] = 0xFFFFFFFF;
			NVIC->ICPR[2] = 0xFFFFFFFF;
 
			__enable_irq();
 
			/* Change the main and local  stack pointer. */
			__set_MSP(*(volatile uint32_t*)BOOT_ADDRESS);
			SCB->VTOR = *(volatile uint32_t*)BOOT_ADDRESS;
 
			jump_to_app();
				break;
			}
		default:
			break;
	}
	

}

void AppCore::parsePackDisplay(const uint16_t id, uint8_t len, uint8_t* data) {
	uint8_t cmd = data[0];
	switch (id) {
		case addrUpT:
			gpio->setPin(GpioDriver::PinTemperatureUp, (GpioDriver::StatesPin)data[2]);
			break;
		case addrDownT:
			gpio->setPin(GpioDriver::PinTemperatureDown, (GpioDriver::StatesPin)data[2]);
			break;
		case addrEnFan:
			gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatesPin)data[2]);
			break;
		case AddrNumDamper:
			currentWorkMode.stages[currentStage].damper ^= 1;
			break;
		case AddrNumFan:
			currentWorkMode.stages[currentStage].fan ^= 1;
			gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatesPin)currentWorkMode.stages[currentStage].fan);
			break;
		case addrPassword: {
				uint16_t pwd = data[2] | (data[1] << 8);
				if (pwd == password) {
					display->switchPage(23);
					display->sendToDisplayF(addrK1, gParams.k1);
					display->sendToDisplayF(addrK2, gParams.k2);
					display->sendToDisplay(addrPeriod, gParams.period);
				}
				break;
			}
		case addrK1: {
		
				uint32_t t = data[4] | (data[3] << 8) | (data[2] << 16) | (data[1] << 24);
				memcpy(&gParams.k1, &t, sizeof(uint32_t));
				break;
			}
		case addrK2: {
		
				uint32_t t = data[4] | (data[3] << 8) | (data[2] << 16) | (data[1] << 24);
				memcpy(&gParams.k2, &t, sizeof(uint32_t));
				break;
			}
		case addrPeriod: {
		
				gParams.period = data[2] | (data[1] << 8);
				break;
			}
		case CmdDateTime: {
				helperBuf[0] = 0x5a;
				helperBuf[1] = 0xa5;
				helperBuf[2] = data[1];
				helperBuf[3] = data[2];
				helperBuf[4] = data[3];
				helperBuf[5] = data[5];
				helperBuf[6] = data[6];
				helperBuf[7] = data[7];
				display->sendToDisplay(CmdSetDateTime, 8, helperBuf);
				break;
			}
		case CmdNumProgramm: {
				uint16_t key = data[2] | (data[1] << 8);
				keyEvent(key);
				break;
			}
		default:
			p_widget->changeParams(id, len, data);
			break;
	}
}

void AppCore::keyEvent(uint16_t key) {
	switch (key) {
		case ReturnCodeKeyStart:
		//	if (isErrorSensor1 || isErrorSensor2)
			//	break;
			stateRun = StateRunStart;
			xSemaphoreGive(xSemPeriodic);
			break;
		case ReturnCodeKeyStop:
			stateRun = StateRunStop;
			break;
		case ReturnCodeKeyInMenuSettingsProgramms:
			p_widget = lstProgramsEdit;
			p_widget->resetWidget();
			break;
		default: 
			p_widget = p_widget->keyEvent(key);
			break;
	}
	
}
