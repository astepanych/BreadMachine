
#include "Uart5.h"
#include "../../../../common/dataexchenge.h"
#include "GpioDriver.h"
#include <string.h>
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include <stm32f4xx_flash.h>

#define MAIN_ADDRESS 0x08000000

void mainApp(void) {
	//��������� ������ ������
	__set_MSP(*(__IO uint32_t*) MAIN_ADDRESS);    
	__set_PRIMASK(1); //��������� ����������
	SCB->VTOR = MAIN_ADDRESS; //��������� ������ ������� ���������� �� ���������� ������
	__set_PRIMASK(0); //��������� ����������
	//������ 
	NVIC_SystemReset();
}


GpioDriver gpio;

xQueueHandle queExchange;
TaskHandle_t xHandleExchange = NULL;

#define BOOT_ADDRESS    0x08040000//����� ������ ���������

uint8_t bufFirmware[1100];
uint32_t indexBuf;
uint32_t commonIndex;
uint16_t crcCalc;

void procUartData(PackageNetworkFormat &p)
{
	switch (p.cmdId) {
		case IdSoftVersion:
			objDataExchenge.sendPackage(IdSoftVersion, 1, 0, nullptr);
			break;
		case IdStartBootloader:
			objDataExchenge.sendPackage(IdDoneBootloader, 1, 0, nullptr);
			break;
		case IdFirmwareStart:
			commonIndex = 0;
			indexBuf = 0;
		/*	FLASH_Unlock();
			FLASH_EraseSector(FLASH_Sector_0, VoltageRange_3);
			FLASH_EraseSector(FLASH_Sector_1, VoltageRange_3);
			FLASH_EraseSector(FLASH_Sector_2, VoltageRange_3);
			FLASH_EraseSector(FLASH_Sector_3, VoltageRange_3);
			FLASH_EraseSector(FLASH_Sector_4, VoltageRange_3);
			FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3);
			FLASH_Lock();*/
			
			break;
		case IdFirmwareData:
			if (p.dataSize > 0x20)
				asm(" nop");
			memcpy(bufFirmware + indexBuf, p.data, p.dataSize);
			
			indexBuf += p.dataSize;
			if(indexBuf>1024)
				asm(" nop");
			break;
		case IdFirmwareCrc:
			crcCalc = DataExchenge::CRC_Calc_s16_CCITT((uint16_t*)bufFirmware, indexBuf / sizeof(uint16_t));
			if (crcCalc == p.data[0]) {
				crcCalc = StateCrcOk;
				/*FLASH_Unlock();
				for (int i = 0; i < indexBuf; i++)
				{
					FLASH_ProgramByte(commonIndex, bufFirmware[i]);
					commonIndex++;
				}
				FLASH_Lock();*/
				
			}
			else
			{
				crcCalc = StateCrcBad;
			}
			indexBuf = 0;
			objDataExchenge.sendPackage(IdFirmwareResult, 1, sizeof(uint16_t), (uint8_t*)&crcCalc);
			break;
		case IdResetMcu:
			mainApp();
			break;
		default:
			break;
	}
}

static void tskExchange(void *p) {
	PackageNetworkFormat package;
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	objDataExchenge.sendPackage(IdDoneBootloader, 1, 0, nullptr);
	while (true) {
		
		//vTaskDelay(1000/portTICK_PERIOD_MS);
		//gpio.togglePin(GpioDriver::Led);
		xQueueReceive(queExchange, &package, portMAX_DELAY);
		procUartData(package);
	}
}

int main() {

	__set_CONTROL(0); 
	NVIC_SetVectorTable(BOOT_ADDRESS, 0); //������������ �������
	__enable_irq(); //��������� ����������
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	uart5;
	uart5.init();
	objDataExchenge;
	gpio.initModule();
	queExchange = xQueueCreate(32, sizeof(PackageNetworkFormat));
	xTaskCreate(
	                        tskExchange,       /* Function that implements the task. */
	                        "exch",          /* Text name for the task. */
	                        1024,      /* Stack size in words, not bytes. */
	                        (void *) 1,    /* Parameter passed into the task. */
	                        tskIDLE_PRIORITY,/* Priority at which the task is created. */
	                        &xHandleExchange); /* Used to pass out the created task's handle. */
	
	objDataExchenge.procPack = [=](PackageNetworkFormat & p) {
		xQueueSend(queExchange, &p, 0);
	};
	

	uart5.putByte = [=](const uint8_t *byte, const int len) {
		objDataExchenge.putByte(byte, len);
	};
	
	objDataExchenge.sendBytes = [=](const uint8_t *buf, const uint8_t len) {
		uart5.txDma(buf, len);
	};

	vTaskStartScheduler();
	return 0;
}