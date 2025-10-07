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
#include <I2C3.h>

const char defaultSSIDName[] = "BreadMachine";
const char defaultSSIDPassword[] = "12345678";

struct FanState {
	FanState() {
		cnt = 0;
		isEnable = false;
	}
	uint16_t cnt;
	bool isEnable;
}gFan;


static void tskPeriodic(void *p) {
	AppCore::instance().taskPeriodic();
}
static void tskControlInPins(void *p) {
	AppCore::instance().taskControlInPins();
}

AppCore &AppCore::instance() {
	static AppCore obj;
	return obj;
}

void vTimerCallback(TimerHandle_t xTimer) {
	AppCore::instance().eventTimeoutDamper(xTimer);
}

void AppCore::eventTimeoutDamper(TimerHandle_t timer) {
	gpio->setPin(GpioDriver::PinShiberX, (GpioDriver::StatePinZero));
	gpio->setPin(GpioDriver::PinShiberO, (GpioDriver::StatePinZero));
	m_signedStateDamper = 0;
}

AppCore::AppCore() {
	m_stateDamper = 0xffff;
	initHal();
	initOsal();
	initText();
	initExchange();

	p_widget = lstPrograms;
	m_signedStateDamper = 0;
}

void AppCore::initOsal() {

	timerDamper = xTimerCreate("timerDamp", (30000 / portTICK_PERIOD_MS), pdFALSE, (void*)2, vTimerCallback);
	xSemPeriodic = xSemaphoreCreateBinary();
	xReturned = xTaskCreate(
	                        tskPeriodic,       /* Function that implements the task. */
		"NAME",          /* Text name for the task. */
		512,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandle); /* Used to pass out the created task's handle. */
    
	xReturned = xTaskCreate(
	tskControlInPins,       /* Function that implements the task. */
		"NAME1",          /* Text name for the task. */
		256,      /* Stack size in words, not bytes. */
		(void *) 1,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		&xHandlePull); /* Used to pass out the created task's handle. */
		
}

void AppCore::initDefaultPrograms() {
	m_programs.clear();
	fillProgram("ХЛЕБ БОРОДИНСКИЙ", 3);

}

void AppCore::fillProgram(const std::string &name, const uint16_t numStages) {
	
	WorkMode el;
	memset(el.nameMode, 0x0, MaxLengthNameMode);
	char tmpBuf[MaxLengthNameMode * 2];
	memset(tmpBuf, 0x0, MaxLengthNameMode * 2);
	memcpy(tmpBuf, name.data(), name.length());
	el.lenNameMode = convertUtf8ToCp1251(tmpBuf, el.nameMode);

	el.numStage = numStages;
	srand(time(0));
	for (int i = 0; i < numStages; i++) {
		el.stages[i].duration = 8;
		el.stages[i].temperature = 180 + rand() % 20 * 10;
		el.stages[i].waterVolume = rand() % 30 * 100;
		el.stages[i].waterVolume2 = rand() % 30 * 100;
		el.stages[i].watertimeout = 40;
		memset(el.stages[i].fan, 0, MAX_SETTINGS_FUN_AND_DAMP*sizeof(SettingsFanAndDamper));
		memset(el.stages[i].damper, 0, MAX_SETTINGS_FUN_AND_DAMP*sizeof(SettingsFanAndDamper));
		for (int j = 0; j < 3; j++) {
			el.stages[i].fan[j].start = 8*j+1;
			el.stages[i].fan[j].finish = 8*j + 7;
			el.stages[i].fan[j].state = 0;
			el.stages[i].damper[j].start = 8*j + 1;
			el.stages[i].damper[j].finish = 8*j + 7;
			el.stages[i].damper[j].state = 0;
		}

	}
	m_programs.insert(m_programs.end(), el);
}
void AppCore::readPrograms() {
	int magic = 0;
#ifdef EEPROM_MEMORY
	int delay = 20000;
	while (delay--) ;
	bool res = I2C3Interface::instance().read(EepromAddrProgramsAttribute, (uint8_t*)&magic, sizeof(int));
	if (magic != MagicNumber || !res) {
		initDefaultPrograms();
		writProgramsToEeprom();
	}
	else {
		res = I2C3Interface::instance().read(EepromAddrProgramsAttribute + OffsetAddrNumPrograms, (uint8_t*)&magic, sizeof(int));
		if (!res)
			return;
		WorkMode mode;
		m_programs.clear();
		m_programs.resize(magic);
		uint16_t stepAddr = 8;
		for (int i = 0; i < magic; i++) {
			
			uint16_t addr = EepromAddrPrograms + i * EepromPageSize * 5;
			uint8_t *p = (uint8_t*)&m_programs[i];
				
			for (int j = 0; j < sizeof(WorkMode); j += stepAddr) {
				uint16_t size = (sizeof(WorkMode) - j >= stepAddr) ? stepAddr : sizeof(WorkMode) - j; 
				I2C3Interface::instance().read(addr + j, p, size);
				p += stepAddr;
			}
		}
	}
	
	uint8_t *p = (uint8_t*)&gParams;
	uint16_t size;
	for (int i = 0; i < sizeof(gParams); i += EepromPageSize) {
		size = (sizeof(gParams) - i >= EepromPageSize) ? EepromPageSize : sizeof(gParams) - i; 
		I2C3Interface::instance().read(EepromAddrGlobalParams + i, p, size);
		p += EepromPageSize;
	}
	if (gParams.crc32 != CRC32_function((uint8_t*)&gParams.k1, sizeof(gParams) - sizeof(gParams.crc32))) {
		gParams.k1 = 0.1;
		gParams.k2 = 5;
		gParams.period = 10;
		gParams.timeoutAddWater = 60;
		memset(gParams.wifiPassword, 0, LenWifiPassword);
		memcpy(gParams.wifiPassword, defaultSSIDPassword, strlen(defaultSSIDPassword));
		memset(gParams.wifiSSID, 0, LenWifiSSID);
		memcpy(gParams.wifiSSID, defaultSSIDName, strlen(defaultSSIDName));
		gParams.stateWifi = WifiOn;
		gParams.numSound = 0;
		gParams.volume = 4;
		gParams.waterOneVolume = 100;
		gParams.ampSensTemp = 11.0;

		writeParamsToEeprom();
	}

#else
	bool isFlag = true;
	int *p = (int*)FlashAddrPrograms;
	if (*p != MagicNumber) {
		isFlag = false;
		initDefaultPrograms();
		//writePrograms();
	}
	else {
		p++;
		int numProgram = *p;
		WorkMode mode;
		m_programs.clear();
		m_programs.resize(numProgram);
		uint8_t *pByte = (uint8_t *)(FlashAddrPrograms + OffsetAddrPrograms);
		uint8_t *pMode; // = (uint8_t*)&mode;
		for (int i = 0; i < m_programs.size(); i++) {
			pMode = (uint8_t*)&m_programs[i];
			for (int j = 0; j < sizeof(WorkMode); j++) {
				*pMode++ = *pByte++;
			}
		}
	}
	
	memcpy(&gParams, (void*)FlashAddrGlobalParams, sizeof(gParams));
	if (gParams.crc32 != CRC32_function((uint8_t*)&gParams.k1, sizeof(gParams) - sizeof(gParams.crc32))) {
		gParams.k1 = 0.1;
		gParams.k2 = 5;
		gParams.period = 10;
		gParams.timeoutAddWater = 60;
		memset(gParams.wifiPassword, 0, LenWifiPassword);
		memcpy(gParams.wifiPassword, defaultSSIDPassword, strlen(defaultSSIDPassword));
		memset(gParams.wifiSSID, 0, LenWifiSSID);
		memcpy(gParams.wifiSSID, defaultSSIDName, strlen(defaultSSIDName));
		gParams.stateWifi = WifiOff;
		isFlag = false;
		//writeGlobalParams();
	}
	if (isFlag == false)
		writeGlobalParams();
#endif
	
}
void AppCore::writeParamsToEeprom() {
	gParams.crc32 = CRC32_function((uint8_t*)&gParams.k1, sizeof(gParams) - sizeof(gParams.crc32));
	uint8_t *p = (uint8_t*)&gParams;
	uint16_t size;
	for (int i = 0; i < sizeof(gParams); i += EepromPageSize) {
		size = (sizeof(gParams) - i >= EepromPageSize) ? EepromPageSize : sizeof(gParams) - i; 
		I2C3Interface::instance().write(EepromAddrGlobalParams + i, p, size);
		p += EepromPageSize;
	}
}

void AppCore::writProgramsToEeprom() {
	bool res = I2C3Interface::instance().write(EepromAddrProgramsAttribute, (uint8_t*)&MagicNumber, sizeof(int));
	if (!res)
		return;
	int val = m_programs.size();
	res = I2C3Interface::instance().write(EepromAddrProgramsAttribute + OffsetAddrNumPrograms, (uint8_t*)&val, sizeof(int));
	if (!res)
		return;
	uint8_t *p;
	uint16_t size;

	for (int i = 0; i < m_programs.size(); i++) {
		p = (uint8_t*)&m_programs[i];
		uint16_t addr = EepromAddrPrograms + i * EepromPageSize * 5;
	
		for (int j = 0; j < sizeof(WorkMode); j += EepromPageSize) {
			size = (sizeof(WorkMode) - j >= EepromPageSize) ? EepromPageSize : sizeof(WorkMode) - j; 
			if (I2C3Interface::instance().write(addr + j, p, size) == false)
				return;
			p += EepromPageSize;
		}
	}
}


void AppCore::writeGlobalParams() {
	gParams.crc32 = CRC32_function((uint8_t*)&gParams.k1, sizeof(gParams) - sizeof(gParams.crc32));
	FLASH_Unlock();
	FLASH_Status status = FLASH_EraseSector(FLASH_Sector_7, VoltageRange_3);
	FLASH_GetStatus();
	uint32_t adr = FlashAddrGlobalParams;
	uint8_t *p;
	p = (uint8_t*)&gParams;
	for (int j = 0; j < sizeof(gParams); j++) {
		status = FLASH_ProgramByte(adr, *p);
		adr++;
		p++;
	}
	
	status = FLASH_ProgramWord(FlashAddrPrograms, MagicNumber);
	status = FLASH_ProgramWord(FlashAddrPrograms + OffsetAddrNumPrograms, m_programs.size());
	adr = FlashAddrPrograms + OffsetAddrPrograms;
	
	for (int i = 0; i < m_programs.size(); i++) {
		p = (uint8_t*)&m_programs[i];
		for (int j = 0; j < sizeof(WorkMode); j++) {
			status = FLASH_ProgramByte(adr, *p);
			adr++;
			p++;
		}
	}
	FLASH_Lock();
}

AppCore::~AppCore() {
}

/**
 * @brief Инициализация аппаратного уровня (HAL) приложения
 * 
 * Выполняет настройку приоритетов прерываний, инициализацию драйверов периферии
 * и регистрацию обработчиков событий. Включает:
 * - Настройку NVIC
 * - Инициализацию GPIO с обработчиками событий
 * - Инициализацию драйвера дисплея
 * - Инициализацию ADC
 * - Инициализацию RTC
 * - Чтение конфигурации из EEPROM (если определена)
 * - Установку коэффициентов усиления для датчиков температуры
 */
void AppCore::initHal() {
	// Настройка группировки приоритетов прерываний
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
	// Инициализация драйвера GPIO
	initGpio();
    
	// Инициализация драйвера дисплея
	initDisplay();
    
	// Инициализация ADC
	initAdc();
    
	// Инициализация RTC
	initRtc();
    
	// Чтение программ из EEPROM (если поддержка включена)
#ifdef EEPROM_MEMORY
	I2C3Interface::instance().init();
	readPrograms();
#endif
    
	// Установка коэффициентов для датчиков температуры
	adc->setCoeff(gParams.ampSensTemp);
}

/**
 * @brief Инициализация драйвера GPIO и регистрация обработчиков событий
 */
void AppCore::initGpio() {
	gpio = new GpioDriver;
	gpio->initModule();
    
	// Регистрация обработчика событий пинов GPIO
	gpio->pinEvent = [this](int pin, bool flag) {
		handleGpioEvent(pin, flag);
	};
}

/**
 * @brief Обработчик событий GPIO
 * 
 * @param pin Идентификатор пина, на котором произошло событие
 * @param flag Состояние пина (true - активное, false - неактивное)
 */
void AppCore::handleGpioEvent(int pin, bool flag) {
	switch (pin) {
	case GpioDriver::InputPinWater:
		handleWaterSensorEvent(flag);
		break;
            
	case GpioDriver::InputPinDamperState:
		handleDamperSensorEvent(flag);
		break;
            
	default:
		// Игнорируем неизвестные пины
		break;
	}
}

/**
 * @brief Обработчик событий датчика воды
 * 
 * @param flag Состояние датчика воды
 */
void AppCore::handleWaterSensorEvent(bool &flag) {
	if (isMenuTests) {
		display->sendToDisplay(addrIconWaterPin, flag);
		return;
	}
    
	// Если счетчик воды обнулен, сбрасываем счетчик импульсов
	if (m_statesWork.cntH2O == 0) {
		m_statesWork.cntIntWater = 0;
	}
    
	// Увеличиваем счетчик импульсов воды
	m_statesWork.cntIntWater++;
    
	// Каждое второе срабатывание датчика уменьшает счетчик воды
	if (m_statesWork.cntIntWater % 2 == 0) {
		m_statesWork.cntH2O -= gParams.waterOneVolume;
	}
    
	// Если вода закончилась, закрываем клапан и сбрасываем счетчик
	if (m_statesWork.cntH2O <= 0) {
		gpio->setPin(GpioDriver::PinH2O, GpioDriver::StatePinZero);
		m_statesWork.cntIntWater = 0;
	}
}

/**
 * @brief Обработчик событий датчика положения заслонки
 * 
 * @param flag Состояние датчика заслонки
 */
void AppCore::handleDamperSensorEvent(bool flag) {
	if (isMenuTests) {
		display->sendToDisplay(addrIconDamperPos, !flag);
		return;
	}
    
	// Проверка на неинициализированное состояние заслонки
	if (m_stateDamper == 0xFFFF) {
		return;
	}
    
	// Обработка срабатывания датчика (активный низкий уровень)
	if (!flag) {
		// Останавливаем движение заслонки
		gpio->setPin(GpioDriver::PinShiberO, GpioDriver::StatePinZero);
		gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinZero);
        
		// Обновляем текущее положение заслонки
		m_stateDamper += m_signedStateDamper;
        
		// Проверяем, достигли ли целевого положения или границ
		bool isTargetPosition = (currentWorkMode.stages[currentStage].damper[m_currentIndexDamper].state == m_stateDamper);
		bool isMinPosition = (m_stateDamper == 0);
		bool isMaxPosition = (m_stateDamper == 8);
        
		if (isTargetPosition || isMinPosition || isMaxPosition) {
			// Достигли целевой позиции или границы - останавливаемся
			m_signedStateDamper = 0;
		}
		else {
			// Продолжаем движение в заданном направлении
			if (m_signedStateDamper == 1) {
				gpio->setPin(GpioDriver::PinShiberO, GpioDriver::StatePinOne);
			}
			else {
				gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinOne);
			}
		}
	}
}

/**
 * @brief Инициализация драйвера дисплея
 */
void AppCore::initDisplay() {
	display = new DisplayDriver();
    
	// Регистрация обработчика команд от дисплея
	display->newCmd = [this](const uint16_t id, uint8_t len, uint8_t* data) {
		parsePackDisplay(id, len, data);
	};
}

/**
 * @brief Инициализация драйвера ADC
 */
void AppCore::initAdc() {
	adc = new AdcDriver;
	adc->init();
}

/**
 * @brief Инициализация модуля RTC
 */
void AppCore::initRtc() {
	m_rtc = &Rtc::instance();
	m_rtc->initRtc();
}


void AppCore::initText() {
	lstPrograms = new MyList(display, NumItemList);
	lstPrograms->setWModes(&m_programs);
	lstPrograms->changeValue = [&](int index) {
		std::string s = lstPrograms->text(index);
		display->sendToDisplay(addrMainItem, s);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		display->sendToDisplay(addrNameProg, s);
		currentWorkMode = m_programs.at(index);
		m_statesWork.currentIndexProgramm = index;
		m_statesWork.m_targetTemperature = m_programs.at(index).stages[0].temperature;
	};
	lstPrograms->setAddrScrollValue(AddrScrollMainList);
	
	ElementList el;
	el.addrColor = 0x6003;
	el.addrText = 0x6200;
	lstPrograms->addItemHard(el);
	for (int i = 1; i < NumItemList; i++) {
		el.addrColor = lstPrograms->at(i - 1).addrColor + 0x20;
		el.addrText =  lstPrograms->at(i - 1).addrText + 0x40;
		lstPrograms->addItemHard(el);
	}
	
	lstProgramsEdit = new WorkModeEdit(display, NumItemListEdit);
	
	lstProgramsEdit->setWModes(&m_programs);
	lstProgramsEdit->setPrevWidget(lstPrograms);
	lstProgramsEdit->changeValue = [&](int index) {
		
	};
	lstProgramsEdit->saveWorkModes = [=]() {
#ifdef EEPROM_MEMORY
		writProgramsToEeprom();
#else
		writeGlobalParams();
#endif
		std::string s = lstPrograms->text(m_statesWork.currentIndexProgramm);
		display->sendToDisplay(addrMainItem, s);
		currentWorkMode = m_programs.at(m_statesWork.currentIndexProgramm);
	};
	lstProgramsEdit->setAddrScrollValue(AddrScrolBar);
	
	el.addrColor = StartAddrListEditItemsSP + OffsetColorsText;
	el.addrText = StartAddrListEditItemsVP;
	lstProgramsEdit->addItemHard(el);
	for (int i = 1; i < NumItemList; i++) {
		el.addrColor = lstProgramsEdit->at(i - 1).addrColor + StepListEditSP;
		el.addrText =  lstProgramsEdit->at(i - 1).addrText + StepListEditVP;
		lstProgramsEdit->addItemHard(el);
	}
}

void AppCore::taskPeriodic(void *p) {

    
	m_signedStateDamper = 0;

	vTaskDelay(pdMS_TO_TICKS(100));
	display->reset();
    
	if (delayAfterResetDisplay > 0) {
		vTaskDelay(pdMS_TO_TICKS(delayAfterResetDisplay));
	}
    
	display->getDataFromDisplay(AddrRtc, 0, 8);
	vTaskDelay(pdMS_TO_TICKS(100));
    
	checkTemperatureSensors();

	// Инициализация данных
	sendInitialData();
	display->sendToDisplay(CmdSoftVersion, versionSoft);
	display->sendToDisplay(addrStateWifiIcon, iconIndexWifi[gParams.stateWifi]);
    
	uint8_t soundParams[4] = { gParams.numSound, 0, gParams.volume, 0 };
	display->sendToDisplay(addrCurrentSound, 4, soundParams);
    
	lstPrograms->setIndex(0);
	lstPrograms->resetWidget();
	m_statesWork.m_targetTemperature = m_programs.at(0).stages[0].temperature;
	m_statesWork.cntPlaySignal = -1;
	m_statesWork.isMaintainTemperature = false;
	m_statesWork.cntContolDownTemperature = 0;
	while (true) {
		xSemaphoreTake(xSemPeriodic, pdMS_TO_TICKS(1000));
        
		handleSoundPlayback();
        
		float temperature = selectTemperature();
		display->sendToDisplayF(AddrNumTemperatureMeasure, temperature);
        
		switch (stateRun) {
		case StateRunIdle:
			handleIdleState(temperature);
			break;
                
		case StateRunStart:
			if (!handleStartState(temperature)) {
				continue;
			}
			break;
                
		case StateRunWork:
			handleWorkState(temperature);
			break;
                
		case StateRunStop:
			paintStageProgress();
			// Продолжаем в StateRunError
                
		case StateRunError:
			handleStopAndErrorState();
			stateRun = StateRunIdle;
			break;
                
		default:
			break;
		}
	}
}

// Вспомогательные функции
void AppCore::sendInitialData() {
	objDataExchenge.sendPackage(IdBootHost, 1, 0, nullptr);
	objDataExchenge.sendPackage(IdWifiSSID, 1, strlen(gParams.wifiSSID), (uint8_t*)gParams.wifiSSID);
	objDataExchenge.sendPackage(IdWifiPassword, 1, strlen(gParams.wifiPassword), (uint8_t*)gParams.wifiPassword);
	objDataExchenge.sendPackage(IdWifiState, 1, sizeof(gParams.stateWifi), (uint8_t*)&gParams.stateWifi);
}

void AppCore::handleSoundPlayback() {
	if (m_statesWork.cntPlaySignal != -1 && m_statesWork.cntPlaySignal-- == 0) {
		m_statesWork.cntPlaySignal = 5;
		display->playSound(gParams.numSound, gParams.volume);
	}
}

void AppCore::handleIdleState(float temperature) {
	if (m_statesWork.isMaintainTemperature)
		correctTemperature(temperature, m_statesWork.m_targetTemperature);
	else {
		if (m_statesWork.cntContolDownTemperature!=0) {
			m_statesWork.cntContolDownTemperature--;
			if (m_statesWork.cntContolDownTemperature == 0) {
				gpio->setPin(GpioDriver::PinTemperatureDown, (GpioDriver::StatePinZero));
			}
		}
	}
	if (timeBlinkYellow >= 0) {    
		if (timeBlinkYellow % 2 == 0) {
			gpio->disableYellowLed();
		}
		else {
			gpio->enableYellowLed();
		}
		timeBlinkYellow--;
	}
}

bool AppCore::handleStartState(float temperature) {
	if (checkTemperatureSensors() != 0) {
		stateRun = StateRunIdle;
		display->showMessage(PageMessage, stateTemperatureSensor);

		return true;
	}
    
	if (gpio->isDoorOpen()) {
		stateRun = StateRunIdle;
		display->showMessage(PageMessage, DoorNoClosed);
		return true;
	}
    
	gpio->disableIntDamperState();
    
	// Инициализация шибера
	if (!gpio->isDamperStateStart()) {
		gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinOne);
		vTaskDelay(pdMS_TO_TICKS(100));
		xSemaphoreGive(xSemPeriodic);
		return false;
	}
	gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinZero);
	gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinZero);
	m_stateDamper = 0;
	gpio->enableIntDamperState();
	currentWorkMode = m_programs.at(m_statesWork.currentIndexProgramm);

	LOG::instance().log("start"); 
	initializeWorkState();
    
	stateRun = StateRunWork;
	updateParamStage();
	gpio->enableYellowLed();
	timeBlinkYellow = 0;
	gpio->setPin(GpioDriver::GlobalEnable, GpioDriver::StatePinOne);
    
	display->sendToDisplay(AddrNumWaterTime, currentWorkMode.stages[currentStage].watertimeout);
	display->sendToDisplay(AddrNumWater1, currentWorkMode.stages[currentStage].waterVolume2);
	m_signedStateDamper = 0;
	m_currentIndexDamper = 0;
	m_currentIndexFan = 0;
    
	return true;
}

void AppCore::initializeWorkState() {
	currentStage = 0;
	stageDuration = 0;
	m_statesWork.prevTemp = m_statesWork.currentTemp;
	modeDuration = 0;
	commonDuration = 0;
	m_statesWork.isStart = true;
	m_statesWork.isWaterStart = false;
	m_statesWork.isWaterStage2 = false;
    
	for (int i = 0; i < currentWorkMode.numStage; i++) {
		commonDuration += TO_SECONDS(currentWorkMode.stages[i].duration);
	}
    
	paintStageProgress();
}

void AppCore::handleWorkState(float temperature) {
	if (checkTemperatureSensors() != 0) {
		stateRun = StateRunError;
		display->showMessage(PageMessage, stateTemperatureSensor);
		return;
	}
    
	stageDuration++;
	modeDuration++;
    
	correctTemperature(temperature, m_statesWork.m_targetTemperature);
    
	if (stageDuration == 1) {
		display->sendToDisplay(AddrNumWaterTime, currentWorkMode.stages[currentStage].watertimeout);
		display->sendToDisplay(AddrNumWater1, currentWorkMode.stages[currentStage].waterVolume2);
	}
    
	if (stageDuration >= 2) {
		handleFanControl();
		handleDamperControl();
	}
    
	addWater();
	paintStageProgress();
	updateTime(TO_SECONDS(currentWorkMode.stages[currentStage].duration) - stageDuration) ;
    
	handlePreFinishActions();

	if (currentWorkMode.stages[currentStage].duration - stageDuration) {
	
	}
    
	if (stageDuration >= TO_SECONDS(currentWorkMode.stages[currentStage].duration)) {
		handleStageCompletion();
	}
    
	uint16_t progress = static_cast<uint16_t>(stageDuration * 100.0 / TO_SECONDS(currentWorkMode.stages[currentStage].duration)) ;
	updateProgressBar(progress);
}

void AppCore::handleFanControl() {
	//есть в настройках работы вентилятора есть хотя бы один 0, то выключаем винтеляторы
	if (currentWorkMode.stages[currentStage].fan[m_currentIndexFan].start == 0 || currentWorkMode.stages[currentStage].fan[m_currentIndexFan].finish == 0 || currentWorkMode.stages[currentStage].fan[m_currentIndexFan].state == FanOff) {
		gpio->setPin(GpioDriver::PinFanLowSpeed, GpioDriver::StatePinZero);
		gpio->setPin(GpioDriver::PinFanFastSpeed, GpioDriver::StatePinZero);
		return;
	}
	//если текущее время меньше времени начала работы ветилятора, то вентилятор выключен
	if (stageDuration < TO_SECONDS(currentWorkMode.stages[currentStage].fan[m_currentIndexFan].start - 1)) {
		gpio->setPin(GpioDriver::PinFanLowSpeed, GpioDriver::StatePinZero);
		gpio->setPin(GpioDriver::PinFanFastSpeed, GpioDriver::StatePinZero);
		return;
	}
	//если текущее время больше времени завершения работы ветилятора, то наращиваем индекс 
	if (stageDuration >= TO_SECONDS(currentWorkMode.stages[currentStage].fan[m_currentIndexFan].finish)) {
		m_currentIndexFan++;
		return;
	}
	// в зависимости от настройки включаем вентилятор на нужную скорость
	switch (currentWorkMode.stages[currentStage].fan[m_currentIndexFan].state) {
	case FanX1:
		gpio->setPin(GpioDriver::PinFanLowSpeed, GpioDriver::StatePinOne);
		gpio->setPin(GpioDriver::PinFanFastSpeed, GpioDriver::StatePinZero);
		break;
	case FanX2:
		gpio->setPin(GpioDriver::PinFanLowSpeed, GpioDriver::StatePinZero);
		gpio->setPin(GpioDriver::PinFanFastSpeed, GpioDriver::StatePinOne);
		break;
	default:
		gpio->setPin(GpioDriver::PinFanLowSpeed, GpioDriver::StatePinZero);
		gpio->setPin(GpioDriver::PinFanFastSpeed, GpioDriver::StatePinZero);
		break;
	}
}

void AppCore::handleDamperControl() {
	if (m_signedStateDamper == 0) {
		int8_t targetDamper = currentWorkMode.stages[currentStage].damper[m_currentIndexDamper].state;
        
		if (targetDamper > m_stateDamper) {
			gpio->setPin(GpioDriver::PinShiberO, GpioDriver::StatePinOne);
			xTimerStart(timerDamper, 0);
			m_signedStateDamper = 1;
		}
		else if (targetDamper < m_stateDamper) {
			gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinOne);
			xTimerStart(timerDamper, 0);
			m_signedStateDamper = -1;
		}
	}
}

void AppCore::handlePreFinishActions() {
	if (commonDuration - modeDuration <= preFinishSoundTime) {
		display->playSound(gParams.numSound, gParams.volume);
		m_statesWork.cntPlaySignal = 1;
	}
    
	if (commonDuration - modeDuration <= preFinishVentTime) {
		gpio->setPin(GpioDriver::MainHood, GpioDriver::StatePinOne);
	}
	else {
		bool shouldEnableHood = currentWorkMode.stages[currentStage].damper != 0;
		gpio->setPin(GpioDriver::MainHood,
			shouldEnableHood ? 
		             GpioDriver::StatePinOne : GpioDriver::StatePinZero);
	}
}

void AppCore::handleStageCompletion() {
	m_statesWork.isWaterStart = false;
	m_statesWork.isWaterStage2 = false;
	currentStage++;
	stageDuration = 0;
	m_currentIndexDamper = 0;
	m_currentIndexFan = 0;
	m_statesWork.cntH2O = currentWorkMode.stages[currentStage].waterVolume;
    
	if (currentStage == currentWorkMode.numStage) {
		timeBlinkYellow = 60;
		updateProgressBar(100);
		stateRun = StateRunStop;
		display->playSound(gParams.numSound, gParams.volume);
		LOG::instance().log("finish");
		return;
	}
    
	updateParamStage();
}

void AppCore::handleStopAndErrorState() {
	gpio->setPin(GpioDriver::PinFanLowSpeed, GpioDriver::StatePinZero);
	gpio->setPin(GpioDriver::PinFanFastSpeed, GpioDriver::StatePinZero);
	gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinZero);
	gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinZero);
	gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinZero);
	gpio->setPin(GpioDriver::PinShiberO, GpioDriver::StatePinZero);
	gpio->disableYellowLed();
	gpio->setPin(GpioDriver::GlobalEnable, GpioDriver::StatePinZero);
}

unsigned int AppCore::CRC32_function(unsigned char *buf, unsigned long len) {
	unsigned long crc_table[256];
	unsigned long crc;
	for (int i = 0; i < 256; i++) {
		crc = i;
		for (int j = 0; j < 8; j++)
			crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
		crc_table[i] = crc;
	}
	;
	crc = 0xFFFFFFFFUL;
	while (len--)
		crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
	return crc ^ 0xFFFFFFFFUL;
}

#define PERIOD_CORRECT (gParams.period)
void AppCore::correctTemperature(float &currentTemp, uint16_t &targetTemp) {
	static int delta = 1;
	static uint16_t period = 0;


	float target = 1.0*targetTemp;
	static uint16_t per = PERIOD_CORRECT;
	period++;
	if (period == per) {
		
		m_statesWork.newPeriodCorrect = PERIOD_CORRECT;
		
		float T = (target - m_statesWork.currentTemp)*gParams.k1 - (m_statesWork.currentTemp - m_statesWork.prevTemp) * gParams.k2 / (PERIOD_CORRECT + delta);
		m_statesWork.signedDef =  m_statesWork.currentTemp - m_statesWork.prevTemp;
		m_statesWork.prevTemp = m_statesWork.currentTemp;
		if (T <= 0) {
			gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinOne);
		}
		else {
			gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinOne);
		}
		delta = ceil(fabs(T));
	}
	else if (period >= per + delta) {
		period = 0;
		gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinZero);
		gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinZero);
		per = m_statesWork.newPeriodCorrect;
	}
}




