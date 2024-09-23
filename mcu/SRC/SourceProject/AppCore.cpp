#include <AppCore.h>
#include <stdio.h>
#include <misc.h>
#include <string.h>
#include <string>
#include <stm32f4xx_flash.h>

#include "typedef.h"
#include <time.h>
#include <math.h>
float AppCore::U;
uint16_t AppCore::temperature;

constexpr uint16_t versionSoft = 1006;
GpioDriver *AppCore::gpio;

AdcDriver *AppCore::adc;

DisplayDriver *AppCore::display;

uint8_t AppCore::helperBuf[256];

WorkModeEdit *AppCore::lstProgramsEdit;
MyList *AppCore::lstPrograms;
Widget *AppCore::p_widget;
WorkMode AppCore::currentWorkMode;
uint16_t AppCore::stateRun = StateRunIdle;
uint32_t AppCore::commonDuration;
uint16_t AppCore::currentStage;
 uint16_t AppCore::stageDuration;
 uint16_t AppCore::modeDuration;




std::vector<WorkMode> AppCore::m_programs;
void AppCore::initDefaultPrograms()
{
	m_programs.clear();
	fillProgram("ХЛЕБ БОРОДИНСКИЙ", 3);
	fillProgram("ХЛЕБ КИРПИЧИК", 4);
	fillProgram("ХЛЕБ СЕРЫЙ", 5);
	fillProgram("БАТОН НАРЕЗНОЙ", 2);
	fillProgram("БАТОН С ПОВИДЛОМ", 7);
	fillProgram("ВАТРУШКА", 6);
	fillProgram("БАТОН С МАКОМ", 7);
	fillProgram("ПЕЧЕНЬЕ ОВСЯНОЕ", 4);
	fillProgram("ПРЯНИК МЕДОВЫЙ", 3);
	

}

void AppCore::fillProgram(const std::string &name, const uint16_t numStages, const uint16_t *stage)
{
	
	WorkMode el;
	memset(el.nameMode, 0x0, MaxLengthNameMode);
	char tmpBuf[MaxLengthNameMode];
	memset(tmpBuf, 0x0, MaxLengthNameMode);
	memcpy(tmpBuf, name.data(), name.length());
	el.lenNameMode = convertUtf8ToCp1251(tmpBuf, el.nameMode);

	el.numStage = numStages;
	srand(time(0));
	for (int i = 0; i < numStages; i++)	{
		el.stages[i].duration = rand() % 100 * 10;
		el.stages[i].temperature = 180 + rand() % 20 * 10;
		el.stages[i].waterVolume = rand() % 30 * 100;
		el.stages[i].fan = rand() % 2;
		el.stages[i].damper = rand() % 2;
	}
	m_programs.insert(m_programs.end(), el);
}
void AppCore::readPrograms()
{
	int *p = (int*)FlashAddrPrograms;
	if (*p != MagicNumber)
	{
		initDefaultPrograms();
		writePrograms();
	}
	else
	{
		p++;
		int numProgram = *p;
		WorkMode mode;
		m_programs.clear();
		m_programs.resize(numProgram);
		uint8_t *pByte = (uint8_t *)(FlashAddrPrograms + OffsetAddrPrograms);
		uint8_t *pMode;// = (uint8_t*)&mode;
		for (int i = 0; i < m_programs.size(); i++)
		{
			pMode = (uint8_t*)&m_programs[i];
			for (int j = 0; j < sizeof(WorkMode); j++)
			{
				*pMode++ = *pByte++;
	
			}
		
		}
	}
}
void AppCore::writePrograms()
{
	FLASH_Unlock();
	FLASH_EraseSector(FLASH_Sector_11, VoltageRange_4);
	FLASH_ProgramWord(FlashAddrPrograms, MagicNumber);
	FLASH_ProgramWord(FlashAddrPrograms+OffsetAddrNumPrograms, m_programs.size());
	uint32_t adr = FlashAddrPrograms + OffsetAddrPrograms;
	uint8_t *p;
	for (int i = 0; i < m_programs.size(); i++)
	{
		p = (uint8_t*)&m_programs[i];
		for (int j = 0; j < sizeof(WorkMode); j++)
		{
			FLASH_ProgramByte(adr, *p);
			adr++;
			p++;
		}
		
	}
	FLASH_Lock();
}

AppCore::AppCore()
{
	initHal();
	initOsal();
	initText();
	p_widget = lstPrograms;


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
	

	
	display = new DisplayDriver();
	display->newCmd = this->parsePackDisplay;
	
	adc = new AdcDriver;
	adc->init();
	
}
void AppCore::initOsal()
{
	xReturned = xTaskCreate(
				this->taskPeriodic,       /* Function that implements the task. */
		"NAME",          /* Text name for the task. */
		2048,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
		
}

void AppCore::initText()
{
	lstPrograms = new MyList(display, NumItemList);
	lstPrograms->setWModes(&m_programs);
	lstPrograms->changeValue = [&](int index) {
		std::string s = lstPrograms->text(index);
		display->sendToDisplay(addrMainItem, s);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		display->sendToDisplay(addrNameProg, s);
		currentWorkMode = m_programs.at(index);
	};
	lstPrograms->setAddrScrollValue(AddrScrollMainList);
	
	ElementList el;
	el.addrColor = 0x6003;
	el.addrText = 0x6200;
	lstPrograms->addItemHard(el);
	for (int i = 1; i < NumItemList; i++) {
		el.addrColor = lstPrograms->at(i-1).addrColor + 0x20;
		el.addrText =  lstPrograms->at(i-1).addrText + 0x40;
		lstPrograms->addItemHard(el);
	}
	
	lstProgramsEdit = new WorkModeEdit(display, NumItemListEdit);
	
	lstProgramsEdit->setWModes(&m_programs);
	lstProgramsEdit->setPrevWidgwt(lstPrograms);
	lstProgramsEdit->changeValue = [&](int index) {
	
	};
	lstProgramsEdit->saveWorkModes = [=]() {
		writePrograms();
	};
	lstProgramsEdit->setAddrScrollValue(AddrScrolBar);
	
	el.addrColor = StartAddrListEditItemsSP+OffsetColorsText;
	el.addrText = StartAddrListEditItemsVP;
	lstProgramsEdit->addItemHard(el);
	for (int i = 1; i < NumItemList; i++) {
		el.addrColor = lstProgramsEdit->at(i - 1).addrColor + StepListEditSP;
		el.addrText =  lstProgramsEdit->at(i - 1).addrText + StepListEditVP;
		lstProgramsEdit->addItemHard(el);
	}
	

}

void AppCore::parsePackDisplay(const uint16_t id, uint8_t len, uint8_t* data)
{
	uint8_t cmd = data[0];
	switch (id)
	{
	case AddrNumDamper:
		currentWorkMode.stages[currentStage].damper ^= 1;
		break;
	case AddrNumFan:
		currentWorkMode.stages[currentStage].fan ^= 1;
		break;
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

void AppCore::keyEvent(uint16_t key)
{
	switch (key)
	{
	case ReturnCodeKeyStart:
		stateRun = StateRunStart;
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


void AppCore::getSizeWRectangle(const WorkMode &mode, uint16_t *wList)
{
	
	int i;
	int xStart = xProgresStage, xEnd;

	for ( i = 0; i < mode.numStage; i++)
	{
		wList[2*i] = xStart;
		xEnd = xStart + mode.stages[i].duration * (wProgresStage - mode.numStage * 2) / (commonDuration);
		wList[2*i + 1] = xEnd;
		xStart = xEnd + 3;
	}
	wList[2*i - 1] = xProgresStage + wProgresStage;
}

void AppCore::paintStageProgress()
{

	uint16_t wList[2*MaxStageMode];
	getSizeWRectangle(currentWorkMode, wList);
	
	
	uint16_t numSpliter = currentWorkMode.numStage - 1;
	uint16_t *p16;
	u16be *p = (u16be *)helperBuf;
	*p = CmdPaintFillRectangle;
	p++;
	*p = currentWorkMode.numStage;
	
	Rectangle rec;
	
	rec.beginY = yProgresStage;
	
	rec.endY = yProgresStage+hProgresStage;
	
	p16 = (uint16_t *)(helperBuf + 4);
	for (int i = 0; i < currentWorkMode.numStage; i++)
	{
		rec.beginX = wList[2*i];
		rec.endX = wList[2*i+1];
		rec.color = i < currentStage ? ColorGreen:ColorGrey;
		
		memcpy(p16, &rec, sizeof(Rectangle));
		p16 += (sizeof(Rectangle) / sizeof(uint16_t));
	}
	
	p = (u16be *)p16;
	*p = 0xff00;
	p16++;
	uint16_t len = p16 - (uint16_t*)helperBuf;
	display->sendToDisplay(AddrStages, len*sizeof(uint16_t), helperBuf);
	
}

void AppCore::updateProgressBar(uint16_t value)
{
	display->sendToDisplay(AddrProgressBar, value);
}
void AppCore::updateParamStage()
{
	display->sendToDisplay(AddrNumStage, currentStage + 1);
	display->sendToDisplay(AddrNumWater, currentWorkMode.stages[currentStage].waterVolume);
	display->sendToDisplay(AddrNumTemperature, currentWorkMode.stages[currentStage].temperature);
	display->sendToDisplay(AddrNumFan, currentWorkMode.stages[currentStage].fan);
	display->sendToDisplay(AddrNumDamper, currentWorkMode.stages[currentStage].damper);
	
}

void AppCore::updateTime(uint16_t sec)
{
	uint16_t _min = sec / 60;	
	uint16_t _sec = sec % 60;
	char buf[6];
	uint8_t len = sprintf(buf, "%02d:%02d", _min, _sec);
	display->sendToDisplay(AddrNumTime, len, (uint8_t*)buf);
	
}

void AppCore::correctTemperature(float &currentTemp, uint16_t &targetTemp)
{
	static uint16_t period = 0;
	static float target = 1.0*targetTemp;
	static float current = currentTemp;
	period++;
	if (period == 10)
	{
		if (target > current)
		{
			gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::GpioDriver::StatePinOne);
			
		}
		if (target < currentTemp)
		{
			gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::GpioDriver::StatePinOne);
			
		}
	} 
	if (period == 11) {
		period = 0;
		gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinZero);
		gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinZero);
	}
}

void AppCore::taskPeriodic(void *p)
{
	float testF = 1.1;
	int count = 0;
	int cnt = 0, len;
	char buf[256];
	readPrograms();

	display->reset();
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	
	display->sendToDisplay(CmdSoftVersion, versionSoft);
	display->sendToDisplay(addrMainItem, "ВЫБОР ПРОГРАММЫ");
	lstPrograms->resetWidget();	
	uint16_t per;
	while (true) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);

		U = adc->value2() * 3.3 / 4095;
		float f = (2590.0*U - 330) / (1.2705 - 0.385*U);
		
		display->sendToDisplayF(AddrNumTemperatureMeasure, f);

		
		switch (stateRun)
		{
		case StateRunIdle:
			break;
		case StateRunStart:
			currentStage = 0;
			stageDuration = 0;
			modeDuration = 0;
			for (int i = 0; i < currentWorkMode.numStage; i++)
			{
				commonDuration += currentWorkMode.stages[i].duration;
			}
			paintStageProgress();
			stateRun = StateRunWork;
			updateParamStage();
			break;
		case StateRunWork:
		{

			stageDuration += 1;
			modeDuration += 1;
			correctTemperature(f, currentWorkMode.stages[currentStage].temperature);
			gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatesPin)currentWorkMode.stages[currentStage].fan);
			gpio->setPin(GpioDriver::PinShiberX, (GpioDriver::StatesPin)currentWorkMode.stages[currentStage].damper);
			gpio->setPin(GpioDriver::PinShiberO, (GpioDriver::StatesPin)(!currentWorkMode.stages[currentStage].damper));
			
			
			paintStageProgress();
			updateTime(currentWorkMode.stages[currentStage].duration - stageDuration);
			if (stageDuration >= currentWorkMode.stages[currentStage].duration)
			{
				currentStage++;
				stageDuration = 0;
				
				if (currentStage == currentWorkMode.numStage) {
					updateProgressBar(100);
					stateRun = StateRunStop;
					break;
				}
				updateParamStage();
			}
			per = (uint16_t)(modeDuration * 100.0 / commonDuration);
			updateProgressBar(per);
			
		}	
			break;
		case StateRunStop:
			stateRun = StateRunIdle;
			break;
		default:
			break;
		}
		
		
	}
}

