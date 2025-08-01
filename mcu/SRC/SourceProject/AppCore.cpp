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


struct StateWork {
    bool isStart;
    bool isDownTemp;
    float signedDef;
    float unsignedDef;
    float currentTemp;
    float currentTemp1;
    float prevTemp;
    uint16_t newPeriodCorrect;
    uint16_t cntH2O;
    bool isWaterStart;
    uint16_t periodWater;
    uint16_t currentIndex;
	
}gRun;



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
}

AppCore::AppCore() {
    initHal();
    initOsal();
    initText();
    initExchange();
    p_widget = lstPrograms;


}

void AppCore::initOsal() {

    timerDamper = xTimerCreate("timerDamp", (30000 / portTICK_PERIOD_MS), pdFALSE, (void*)2, vTimerCallback);
    xSemPeriodic = xSemaphoreCreateBinary();
    xReturned = xTaskCreate(
                            tskPeriodic,       /* Function that implements the task. */
                            "NAME",          /* Text name for the task. */
                            1024,      /* Stack size in words, not bytes. */
                            (void *) 1,    /* Parameter passed into the task. */
                            tskIDLE_PRIORITY,/* Priority at which the task is created. */
                            &xHandle); /* Used to pass out the created task's handle. */
		
}




void AppCore::initDefaultPrograms() {
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
        el.stages[i].duration = rand() % 100 * 10;
        el.stages[i].temperature = 180 + rand() % 20 * 10;
        el.stages[i].waterVolume = rand() % 30 * 100;
        el.stages[i].fan = rand() % 2;
        el.stages[i].damper = rand() % 2;
    }
    m_programs.insert(m_programs.end(), el);
}
void AppCore::readPrograms() {
	
    int magic;
	
#ifdef EEPROM_MEMORY
	


    int delay = 20000;
    while (delay--) ;
    I2C3Interface::instance().read(EepromAddrProgramsAttribute, (uint8_t*)&magic, sizeof(int));
    if (magic != MagicNumber) {
        initDefaultPrograms();
        writProgramsToEeprom();
    }
    else {
        I2C3Interface::instance().read(EepromAddrProgramsAttribute + OffsetAddrNumPrograms, (uint8_t*)&magic, sizeof(int));
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
    I2C3Interface::instance().write(EepromAddrProgramsAttribute, (uint8_t*)&MagicNumber, sizeof(int));
    int val = m_programs.size();
    I2C3Interface::instance().write(EepromAddrProgramsAttribute + OffsetAddrNumPrograms, (uint8_t*)&val, sizeof(int));
    uint8_t *p;
    uint16_t size;

    for (int i = 0; i < m_programs.size(); i++) {
        p = (uint8_t*)&m_programs[i];
        uint16_t addr = EepromAddrPrograms + i * EepromPageSize * 5;
	
        for (int j = 0; j < sizeof(WorkMode); j += EepromPageSize) {
            size = (sizeof(WorkMode) - j >= EepromPageSize) ? EepromPageSize : sizeof(WorkMode) - j; 
            I2C3Interface::instance().write(addr + j, p, size);
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
int cntH2O = 0;
void AppCore::initHal() {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    gpio = new GpioDriver;
    gpio->initModule();
	
	
    gpio->pinEvent = [=](bool flag) {
        static uint16_t cntInt = 0;
        if (gRun.cntH2O == 0)
            cntInt = 0;
        cntInt++;
        if (cntInt % 2 == 0) {
            currentWorkMode.stages[currentStage].waterVolume -= 100;
			
        }
		
		
        if (currentWorkMode.stages[currentStage].waterVolume <= 0) {
		
            gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinZero);
			
        }
		
    };

    display = new DisplayDriver();
    display->newCmd = [=](const uint16_t id, uint8_t len, uint8_t* data) {
        AppCore::instance().parsePackDisplay(id, len, data);
    };
	
    adc = new AdcDriver;
    adc->init();
	
    m_rtc = &Rtc::instance();
    m_rtc->initRtc();
#ifdef EEPROM_MEMORY
    I2C3Interface::instance().init();
    readPrograms();
#endif
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
        gRun.currentIndex = index;
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
        std::string s = lstPrograms->text(gRun.currentIndex);
        display->sendToDisplay(addrMainItem, s);
        currentWorkMode = m_programs.at(gRun.currentIndex);
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




void AppCore::getSizeWRectangle(const WorkMode &mode, uint16_t *wList) {
	
    int i;
    int xStart = xProgresStage, xEnd;

    for (i = 0; i < mode.numStage; i++) {
        wList[2*i] = xStart;
        xEnd = xStart + mode.stages[i].duration * (wProgresStage - mode.numStage * 2) / (commonDuration);
        wList[2*i + 1] = xEnd;
        xStart = xEnd + 3;
    }
    wList[2*i - 1] = xProgresStage + wProgresStage;
}

void AppCore::paintStageProgress() {

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
	
    rec.endY = yProgresStage + hProgresStage;
	
    p16 = (uint16_t *)(helperBuf + 4);
    for (int i = 0; i < currentWorkMode.numStage; i++) {
        rec.beginX = wList[2*i];
        rec.endX = wList[2*i + 1];
        rec.color = i < currentStage ? ColorGreen : ColorGrey;
		
        memcpy(p16, &rec, sizeof(Rectangle));
        p16 += (sizeof(Rectangle) / sizeof(uint16_t));
    }
	
    p = (u16be *)p16;
    *p = 0xff00;
    p16++;
    uint16_t len = p16 - (uint16_t*)helperBuf;
    display->sendToDisplay(AddrStages, len*sizeof(uint16_t), helperBuf);
	
}

void AppCore::updateProgressBar(uint16_t value) {
    display->sendToDisplay(AddrProgressBar, value);
}
void AppCore::updateParamStage() {
    display->sendToDisplay(AddrNumStage, currentStage + 1);
    display->sendToDisplay(AddrNumWater, currentWorkMode.stages[currentStage].waterVolume);
    display->sendToDisplay(AddrNumTemperature, currentWorkMode.stages[currentStage].temperature);
    display->sendToDisplay(AddrNumFan, currentWorkMode.stages[currentStage].fan);
    display->sendToDisplay(AddrNumDamper, currentWorkMode.stages[currentStage].damper);
	
}

void AppCore::updateTime(uint16_t sec) {
    uint16_t _min = sec / 60;	
    uint16_t _sec = sec % 60;
    char buf[10];
    uint8_t len = sprintf(buf, "%02d:%02d", _min, _sec);
    display->sendToDisplay(AddrNumTime, len, (uint8_t*)buf);
	
    _min = (commonDuration - modeDuration) / 60;
    _sec = (commonDuration - modeDuration) % 60;
    len = sprintf(buf, "%02d:%02d", _min, _sec);
    display->sendToDisplay(AddrNumTimeMode, len, (uint8_t*)buf);
}


#define PERIOD_CORRECT (gParams.period)

void AppCore::correctTemperature(float &currentTemp, uint16_t &targetTemp) {
    static int delta = 1;
    static uint16_t period = 0;


    float target = 1.0*targetTemp;
    static uint16_t per = PERIOD_CORRECT;

    period++;
	
    if (period == per) {
		
        gRun.newPeriodCorrect = PERIOD_CORRECT;
		
        float T = (target - gRun.currentTemp)*gParams.k1 - (gRun.currentTemp - gRun.prevTemp) * gParams.k2 / (PERIOD_CORRECT + delta);
        gRun.signedDef =  gRun.currentTemp - gRun.prevTemp;
        gRun.prevTemp = gRun.currentTemp;
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
        per = gRun.newPeriodCorrect;
    }
}

float AppCore::selectTemperature() {
	
    if (isMenuTests) {
        static char buff[32];
        memset(buff, 0, 32);
        float t1 =  adc->value1();
        float t2 =  adc->value2();
		
        uint16_t len = sprintf(buff, "t1 = %d, t2 = %d", (int)t1, (int)t2);
        display->sendToDisplay(addrStrTempTest, len, (uint8_t*)buff);
    }
	
    gRun.currentTemp = adc->value2(); 
    gRun.currentTemp1 = thresholdErrorTemperature + 1; //adc->value1();
    if (gRun.currentTemp < thresholdErrorTemperature && gRun.currentTemp1 < thresholdErrorTemperature) {
        return (gRun.currentTemp + gRun.currentTemp1) / 2;		
    }
    if (gRun.currentTemp < thresholdErrorTemperature && gRun.currentTemp1 > thresholdErrorTemperature) {
        return gRun.currentTemp;		
    }
    if (gRun.currentTemp > thresholdErrorTemperature && gRun.currentTemp1 < thresholdErrorTemperature) {
        return (gRun.currentTemp1);		
    }
    return -1;
} 

void AppCore::taskPeriodic(void *p) {
    int count = 0;
    int cnt = 0, len;

    vTaskDelay(100 / portTICK_PERIOD_MS);
	

    display->reset();
    vTaskDelay(1800 / portTICK_PERIOD_MS);
    display->getDataFromDisplay(AddrRtc, 0, 8);
    vTaskDelay(100 / portTICK_PERIOD_MS);
	
	
    checkTemperatureSensors();

    objDataExchenge.sendPackage(IdBootHost, 1, 0, nullptr);
    objDataExchenge.sendPackage(IdWifiSSID, 1, strlen(gParams.wifiSSID), (uint8_t*)gParams.wifiSSID);
    objDataExchenge.sendPackage(IdWifiPassword, 1, strlen(gParams.wifiPassword), (uint8_t*)gParams.wifiPassword);
    objDataExchenge.sendPackage(IdWifiState, 1, sizeof(gParams.stateWifi), (uint8_t*)&gParams.stateWifi);
	
    display->sendToDisplay(CmdSoftVersion, versionSoft);
    display->sendToDisplay(addrStateWifiIcon, iconIndexWifi[gParams.stateWifi]);
    uint8_t arr[4] = {gParams.numSound, 0, gParams.volume, 0};
    display->sendToDisplay(addrCurrentSound, 4, arr);
    lstPrograms->setIndex(0);
    lstPrograms->resetWidget();	
    uint16_t per;
    float uTemp, uTemp1;
    while (true) {
		
        xSemaphoreTake(xSemPeriodic, 1000 / portTICK_PERIOD_MS);
		
        float tem = selectTemperature();
        display->sendToDisplayF(AddrNumTemperatureMeasure, tem);

		
        switch (stateRun) {
            case StateRunIdle:
                if (timeBlinkYellow >= 0) {	
                    if (timeBlinkYellow % 2 == 0)
                        gpio->disableYellowLed();
                    else
                        gpio->enableYellowLed();
                    timeBlinkYellow--;
                }
                break;
            case StateRunStart:
                if (checkTemperatureSensors() != 0) {
                    stateRun = StateRunIdle;
                    display->showMessage(PageMessage, stateTemperatureSensor);
                    break;
                }
                currentWorkMode = m_programs.at(gRun.currentIndex);

                LOG::instance().log("start"); 
                currentStage = 0;
                stageDuration = 0;
                gRun.prevTemp = gRun.currentTemp;
                modeDuration = 0;
                commonDuration = 0;
                gRun.isStart = true;
                gRun.isWaterStart  = false;
                for (int i = 0; i < currentWorkMode.numStage; i++) {
                    commonDuration += currentWorkMode.stages[i].duration;
                }
                paintStageProgress();
                stateRun = StateRunWork;
                updateParamStage();
                gpio->enableYellowLed();
                gRun.isWaterStart = false;
                timeBlinkYellow = 0;
                gpio->setPin(GpioDriver::GlobalEnable, GpioDriver::StatePinOne);
                break;
            case StateRunWork:
                {
                    
                    if (checkTemperatureSensors() != 0) {
                        stateRun = StateRunError;
                        display->showMessage(PageMessage, stateTemperatureSensor);
                        break;
                    }
                    stageDuration += 1;
                    modeDuration += 1;
                    uint16_t temp = currentWorkMode.stages[currentStage].temperature;
                    correctTemperature(tem, temp);
                    if (stageDuration == 2) {
                        gpio->setPin(GpioDriver::PinFan, (GpioDriver::StatesPin)currentWorkMode.stages[currentStage].fan);
                        gpio->setPin(GpioDriver::PinShiberX, (GpioDriver::StatesPin)currentWorkMode.stages[currentStage].damper);
                        gpio->setPin(GpioDriver::PinShiberO, (GpioDriver::StatesPin)(!currentWorkMode.stages[currentStage].damper));
                        xTimerStart(timerDamper, 0);
                    }

                    //Добавляем воду если она должна быть добавлена
                    if ((stageDuration >= gParams.timeoutAddWater) && (currentWorkMode.stages[currentStage].waterVolume != 0) && (gRun.isWaterStart == false)) {
                        gRun.cntH2O = currentWorkMode.stages[currentStage].waterVolume;
                        gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinOne);
                        gRun.isWaterStart = true;
                    }		
                    if (gRun.isWaterStart == true) {
				
                        //проверяем что вода пошла
                        if ((stageDuration == gParams.timeoutAddWater + 5) && (gRun.cntH2O ==  currentWorkMode.stages[currentStage].waterVolume)) {
                            gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinZero);
                            LOG::instance().log("err water sen"); 
                            display->showMessage(PageMessage, 4);
                        }
                        //обновляем воду на дисплее
                        display->sendToDisplay(AddrNumWater, currentWorkMode.stages[currentStage].waterVolume);
                        if (currentWorkMode.stages[currentStage].waterVolume <= 0) {
                            gRun.isWaterStart = false;
                        }
                    }
					
			
                    paintStageProgress();
                    updateTime(currentWorkMode.stages[currentStage].duration - stageDuration);
                    if (stageDuration >= currentWorkMode.stages[currentStage].duration) {
                        gRun.isWaterStart = false;
                        currentStage++;
                        stageDuration = 0;
                        gRun.cntH2O = currentWorkMode.stages[currentStage].waterVolume;
                        if (currentStage == currentWorkMode.numStage) {
                            timeBlinkYellow = 60;
                            updateProgressBar(100);
                            stateRun = StateRunStop;
                            display->playSound(gParams.numSound, gParams.volume);
                            LOG::instance().log("finish");
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
                paintStageProgress();
                gpio->setPin(GpioDriver::PinFan, GpioDriver::StatePinZero);
                gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinZero);
                gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinZero);
                gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinZero);
                gpio->setPin(GpioDriver::PinShiberO, GpioDriver::StatePinZero);
                gpio->disableYellowLed();
                gpio->setPin(GpioDriver::GlobalEnable, GpioDriver::StatePinZero);
                //xTimerStart(timerYellow, 0);

                break;
            case StateRunError:
                stateRun = StateRunIdle;
                gpio->setPin(GpioDriver::PinFan, GpioDriver::StatePinZero);
                gpio->setPin(GpioDriver::PinTemperatureDown, GpioDriver::StatePinZero);
                gpio->setPin(GpioDriver::PinTemperatureUp, GpioDriver::StatePinZero);
                gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinZero);
                gpio->setPin(GpioDriver::PinShiberO, GpioDriver::StatePinZero);
                gpio->setPin(GpioDriver::PinH2O, GpioDriver::StatePinZero);
                gpio->disableYellowLed();
                break;
            default:
                break;
        }
    }
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




