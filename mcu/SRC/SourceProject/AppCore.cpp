#include <AppCore.h>
#include <stdio.h>
#include <misc.h>
#include <string.h>
#include <string>
#include <stm32f4xx_flash.h>

#include "typedef.h"
#include <time.h>
#include <math.h>

constexpr uint16_t versionSoft = 1009;


struct StateWork
{
	bool isStart;
	bool isDownTemp;
	float signedDef;
	float unsignedDef;
	float currentTemp;
	float prevTemp;
	uint16_t newPeriodCorrect;
	uint16_t cntH2O;
	bool isWaterStart;
	uint16_t periodWater;
	uint16_t currentIndex;
	
}gRun;

struct CalcParam
{
	CalcParam()
	{ k1 = 0.1;
		k2 = 5;
		period = 10;
	};
	float k1;
	float k2;
	uint16_t period;
}gParams;

struct FanState
{
	FanState()
	{
		cnt = 0;
		isEnable = false;
	}
	uint16_t cnt;
	bool isEnable;
}gFan;


static void tskPeriodic(void *p)
{
	AppCore::instance().taskPeriodic();
}

AppCore &AppCore::instance()
{
	static AppCore obj;
	return obj;
}

void vTimerCallback(TimerHandle_t xTimer)
{
	AppCore::instance().eventTimeoutLeds(xTimer);
}

void AppCore::eventTimeoutLeds(TimerHandle_t timer)
{
	uint16_t *id = (uint16_t*)pvTimerGetTimerID(timer);
	if (id == 0) {
		countYellowLeds++;
		if (countYellowLeds == 10)
		{
			GpioDriver::instace()->setPin(GpioDriver::GpioDriver::PinYellow, GpioDriver::StatePinOne);
			countYellowLeds = 0;
		}
		else
		{
			GpioDriver::instace()->setPin(GpioDriver::GpioDriver::PinYellow, GpioDriver::StatePinZero);
		}
	}
	else {
		countGreenLeds++;
		if (countGreenLeds == 10 )
		{
			GpioDriver::instace()->setPin(GpioDriver::GpioDriver::PinGreen, GpioDriver::StatePinOne);
			countGreenLeds = 0;
		}
		else
		{
			GpioDriver::instace()->setPin(GpioDriver::GpioDriver::PinGreen, GpioDriver::StatePinZero);
		}
		
	}
}

AppCore::AppCore()
{
	initHal();
	initOsal();
	initText();
	p_widget = lstPrograms;

	timerYellow = xTimerCreate("timerYellow", 1 / portTICK_PERIOD_MS, pdTRUE, (void*)0, vTimerCallback);
	timerGreen = xTimerCreate("timerGreen", 1 / portTICK_PERIOD_MS, pdTRUE, (void*)1, vTimerCallback);
	xTimerStart(timerGreen, 0);
	xTimerStart(timerYellow, 0);
}

void AppCore::initOsal()
{
	
	xSemPeriodic = xSemaphoreCreateBinary();
	xReturned = xTaskCreate(
				tskPeriodic,       /* Function that implements the task. */
		"NAME",          /* Text name for the task. */
		2048,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
		
}




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
			for (int j = 0; j < sizeof(WorkMode); j++) {
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



AppCore::~AppCore()
{
}
int cntH2O = 0;
void AppCore::initHal()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	gpio = new GpioDriver;
	gpio->initModule();
	
	
	gpio->pinEvent = [=](bool flag) {
		
		
		if (flag)
			gRun.cntH2O+=100;
		if (gRun.cntH2O >= currentWorkMode.stages[currentStage].waterVolume) {
			gRun.cntH2O = 0;
			gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinZero);
		}
		
	};

	display = new DisplayDriver();
	display->newCmd = [=](const uint16_t id, uint8_t len, uint8_t* data) {
		AppCore::instance().parsePackDisplay(id, len, data);
	};
	
	adc = new AdcDriver;
	adc->init();
	
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
		gRun.currentIndex = index;
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
		currentWorkMode = m_programs.at(gRun.currentIndex);
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
		
		uint32_t t= data[4] | (data[3] << 8) | (data[2] << 16) | (data[1] << 24);
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

void AppCore::keyEvent(uint16_t key)
{
	switch (key)
	{
	case ReturnCodeKeyStart:
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


void AppCore::fan()
{
	if (gRun.currentTemp / (currentWorkMode.stages[currentStage].temperature * 1.0) >= 1.07 && gFan.isEnable == false)
	{
		gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatePinOne));
		gFan.isEnable = true;
		gFan.cnt = 0;
		return;
	}
	if (gFan.isEnable)
	{
		gFan.cnt++;
		if (gFan.cnt >= 5)
		{
			gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatePinZero));
			gFan.isEnable = false;
		}
	}
		
}

#define PERIOD_CORRECT (gParams.period)
bool isCorect = true;
float_t k1 = 0.1;
float_t k2 = 10;
void AppCore::correctTemperature(float &currentTemp, uint16_t &targetTemp)
{
	static int delta = 1;
	static bool isFlag = false;
	if (isCorect == false)
		return;
	static uint16_t period = 0;
	static uint16_t periodFan = 0;

	static float target = 1.0*targetTemp;
	static uint16_t per = PERIOD_CORRECT;
	float defCur;
	period++;
	
	if (gRun.currentTemp / target >= 1.05) {
		gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatePinOne));
	}
	else if (gRun.currentTemp / target <= 1.01)
		gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatePinZero));
	
	if (period == per) {
		
		
		//delta = 1;
		gRun.newPeriodCorrect = PERIOD_CORRECT;
		
		float T = (target - gRun.currentTemp)*gParams.k1 - (gRun.currentTemp - gRun.prevTemp) * gParams.k2 / (PERIOD_CORRECT + delta);
		gRun.signedDef =  gRun.currentTemp - gRun.prevTemp;
		display->sendToDisplayF(0x4024, gRun.signedDef);
		gRun.prevTemp = gRun.currentTemp;
		if (T <= 0)
		{
			gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinOne);
		}
		else
		{
			gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinOne);
		}
		
		
		delta = ceil(fabs(T));
		return;
		if (currentTemp - target < 7*DeltaTemperature && currentTemp - target > 0)
		{
			//если мы в районе нужной температуры то ничего не регулируем
			//return;
			if (currentTemp - target > 4*DeltaTemperature)
				gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinOne);
		}
		else
		{
			gRun.signedDef =  gRun.currentTemp - gRun.prevTemp;
			gRun.unsignedDef = fabsf(gRun.signedDef);
			gRun.prevTemp = gRun.currentTemp;
			display->sendToDisplayF(0x4024, gRun.signedDef);
			
//			if (gRun.isStart) {//выравниваем температуру
//				if (gRun.signedDef < 0)
//					return;
//				gRun.isStart = false;
//				gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinOne);
//				gRun.newPeriodCorrect = PERIOD_CORRECT * 4;
//				delta = 10;
//				return;
//			}
			
			if (gRun.currentTemp < target) {
				if  (gRun.signedDef < 0) {
					gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinOne);
					delta = 3;
					gRun.newPeriodCorrect =  PERIOD_CORRECT;
				}
				else  if(gRun.signedDef < DeltaTemperature){
					gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinOne);
					if (gRun.signedDef < DeltaTemperature / 2 && gRun.currentTemp / target < 0.85) {
						delta = 3;
						gRun.newPeriodCorrect =  PERIOD_CORRECT * 3;
					}
					else {
						delta = gRun.currentTemp / target < 0.75 ? 5 : 1;
						gRun.newPeriodCorrect = gRun.currentTemp / target < 0.75 ? PERIOD_CORRECT * 5 : PERIOD_CORRECT;
					}
				} else if (gRun.signedDef > 4*DeltaTemperature && gRun.currentTemp / target > 0.85) {
					gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinOne);
					delta = 3;
					gRun.newPeriodCorrect = PERIOD_CORRECT * 3;
				}
			}
			else if (gRun.currentTemp > target) {
					gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinOne);
				if (gRun.currentTemp - target > 5) {
					delta = 3;
				}
				gRun.newPeriodCorrect = PERIOD_CORRECT * 3;
			}
			
		}
		
	}
	else if (period >= per + delta) {
		period = 0;
		gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinZero);
		gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinZero);
		per = gRun.newPeriodCorrect;
		gRun.isDownTemp = false;
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
	lstPrograms->setIndex(0);
	lstPrograms->resetWidget();	
	uint16_t per;
	float uTemp;
	while (true) {
		
		xSemaphoreTake(xSemPeriodic, 1000 / portTICK_PERIOD_MS);
		uTemp = adc->value2();
		U = uTemp * 3.3 / 4095;
		gRun.currentTemp = (2590.0*U - 330) / (1.2705 - 0.385*U);
		
		display->sendToDisplayF(AddrNumTemperatureMeasure, gRun.currentTemp);

		
		switch (stateRun)
		{
		case StateRunIdle:
			break;
		case StateRunStart:
			currentStage = 0;
			stageDuration = 0;
			gRun.prevTemp = gRun.currentTemp;
			modeDuration = 0;
			commonDuration = 0;
			gRun.isStart = true;
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
			//fan();
			
			correctTemperature(gRun.currentTemp, currentWorkMode.stages[currentStage].temperature);
			//gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatesPin)currentWorkMode.stages[currentStage].fan);
			gpio->setPin(GpioDriver::PinShiberX, (GpioDriver::StatesPin)currentWorkMode.stages[currentStage].damper);
			gpio->setPin(GpioDriver::PinShiberO, (GpioDriver::StatesPin)(!currentWorkMode.stages[currentStage].damper));
//			
//			if (modeDuration % 4 == 0)
//				gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatePinOne));
//			else
//				gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatePinZero));
			if (stageDuration == 30 && currentWorkMode.stages[currentStage].waterVolume != 0) {
				gRun.cntH2O = 0;
				gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinOne);
				gRun.isWaterStart = false;
			}
			
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
			per = (uint16_t)(stageDuration * 100.0 / currentWorkMode.stages[currentStage].duration);
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

