#include <AppCore.h>
#include <stdio.h>
#include <misc.h>
#include <string.h>
#include <string>

constexpr uint16_t versionSoft = 1002;
GpioDriver *AppCore::gpio;
Uart1 *AppCore::uart1;

DisplayDriver *AppCore::display;

uint8_t AppCore::helperBuf[256];

Lists AppCore::listMain;
std::vector<Programs> AppCore::listPrograms;

AppCore::AppCore()
{
	initHal();
	initOsal();
	initText();
	
	
	
	
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
	display->newCmd = this->parsePackDisplay;
	
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

void AppCore::initText()
{
	listMain.list[0].addrColor = 0x6003;
	listMain.list[0].addrText = 0x6200;
	for (int i = 1; i < NumItemList; i++)
	{
		listMain.list[i].addrColor = listMain.list[i-1].addrColor+0x20;
		listMain.list[i].addrText =  listMain.list[i-1].addrText+0x40;
	}
	Programs el;
	
	el.name = L"Программа 1     ";
	listPrograms.insert(listPrograms.end(), el);
	el.name = L"Программа 2     ";
	listPrograms.insert(listPrograms.end(), el);
	el.name = L"Программа 3     ";
	listPrograms.insert(listPrograms.end(), el);
	el.name = L"Программа 4     ";
	listPrograms.insert(listPrograms.end(), el);
	el.name = L"Программа 5     ";
	listPrograms.insert(listPrograms.end(), el);
	el.name = L"Программа 6     ";
	listPrograms.insert(listPrograms.end(), el);
	el.name = L"Программа 7     ";
	listPrograms.insert(listPrograms.end(), el);
	el.name = L"Программа 8     ";
	listPrograms.insert(listPrograms.end(), el);
	el.name = L"Программа 9     ";
	listPrograms.insert(listPrograms.end(), el);
	
	listMain.maxItemDisplay = 6;
	listMain.maxItem = listPrograms.size();
	
}

void AppCore::parsePackDisplay(const uint16_t id, uint8_t* data)
{
	uint8_t cmd = data[0];
	switch (id)
	{
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
	}
}

void AppCore::keyEvent(uint16_t key)
{
	switch (key)
	{
	case ReturnCodeKeyDownSelectProgramm:
		listMain.indexCommon++;
		if (listMain.maxItem == listMain.indexCommon)
		{
			for (int i = 0; i < NumItemList; i++)
			{
				display->sendToDisplay(listMain.list[i].addrText, listPrograms.at(i).name);
				vTaskDelay(2 / portTICK_PERIOD_MS);
			}
			display->sendToDisplay(listMain.list[0].addrColor, 0xf800);
			display->sendToDisplay(listMain.list[listMain.maxItemDisplay].addrColor, 0x0000);
			listMain.indexCommon = 0;
			listMain.indexDisplay = 0;
			
		}
		else {
			if (listMain.indexDisplay != listMain.maxItemDisplay)
			{
				display->sendToDisplay(listMain.list[listMain.indexDisplay].addrColor, 0x0000);
				listMain.indexDisplay++;
				display->sendToDisplay(listMain.list[listMain.indexDisplay].addrColor, 0xf800);
			}
			else
			{
				int index = listMain.indexCommon - listMain.maxItemDisplay;
				for (int i = 0; i < NumItemList; i++)
				{
					display->sendToDisplay(listMain.list[i].addrText, listPrograms.at(index).name);
					index++;
					vTaskDelay(2 / portTICK_PERIOD_MS);
				}
			}
		}
		break;
	case ReturnCodeKeySaveProgramm:
		listMain.currentIndex = listMain.indexDisplay;
		listMain.currentPos = listMain.indexCommon;
		display->sendToDisplay(addrMainItem, listPrograms.at(listMain.currentPos).name);

		break;
	case ReturnCodeKeyInMenuListProgramms:
		
		break;
	case ReturnCodeKeyUpSelectProgramm:
		listMain.indexCommon--;
		if (listMain.indexCommon == -1)
		{
			listMain.indexCommon = listMain.maxItem - 1;
			listMain.indexDisplay = listMain.maxItemDisplay;
			int index = listMain.indexCommon - listMain.maxItemDisplay;
			for (int i = 0; i < NumItemList; i++)
			{
				display->sendToDisplay(listMain.list[i].addrText, listPrograms.at(index).name);
				index++;
				vTaskDelay(2 / portTICK_PERIOD_MS);
			}
			display->sendToDisplay(listMain.list[0].addrColor, 0x0000);
			display->sendToDisplay(listMain.list[listMain.maxItemDisplay].addrColor, 0xf800);

			
		}
		else {
			if (listMain.indexDisplay != 0)
			{
				display->sendToDisplay(listMain.list[listMain.indexDisplay].addrColor, 0x0000);
				listMain.indexDisplay--;
				display->sendToDisplay(listMain.list[listMain.indexDisplay].addrColor, 0xf800);
			}
			else
			{
				int index = listMain.indexCommon;
				for (int i = 0; i < NumItemList; i++)
				{
					display->sendToDisplay(listMain.list[i].addrText, listPrograms.at(index).name);
					index++;
					vTaskDelay(2 / portTICK_PERIOD_MS);
				}
			}
		}
		break;

		
	default:
		break;
	}
}



void AppCore::taskPeriodic(void *p)
{
	int cnt = 0, len;
	char buf[256];
	//timer3->start();
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	display->reset();
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	
	display->sendToDisplay(CmdSoftVersion, versionSoft);
	std::wstring mainItem = L"Выбор программы";
	
	display->sendToDisplay(listMain.currentPos, mainItem);
	


	for (int i = 0; i < NumItemList; i++)
	{
		display->sendToDisplay(listMain.list[i].addrText, listPrograms.at(i).name);
		vTaskDelay(2 / portTICK_PERIOD_MS);
	}
	display->sendToDisplay(listMain.list[0].addrColor,0xf800);
	while (true) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		
		//display->sendToDisplay(0x2600, listPrograms.at(cnt%NumItemList).name);
		cnt++;
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

