#include <AppCore.h>
#include <stdio.h>
#include <misc.h>
#include <string.h>
#include <string>
#include <stm32f4xx_flash.h>

#include "typedef.h"
#include <time.h>
#include <math.h>

eFailSensorTemperature AppCore::checkTemperatureSensors()
{
    eFailSensorTemperature curState = NoFailSensorTemperature;

    if (adc->value2() > thresholdErrorTemperature)
    {
        curState = FailSensorTemperature2;
        LOG::instance().log("err temp sen2"); 
    }
    else
    {
        curState = NoFailSensorTemperature;		
    }
    if (adc->value1() > thresholdErrorTemperature) {
        curState = static_cast<eFailSensorTemperature>(curState | FailSensorTemperature1);
        LOG::instance().log("err temp sen1"); 
    }
    else
    {	
        curState = static_cast<eFailSensorTemperature>(curState&(~FailSensorTemperature1));
    }
	
    if (stateTemperatureSensor != curState)
    {
        if (curState != NoFailSensorTemperature)
            display->showMessage(PageMessage, curState);
        else
            display->hideMessage();
        stateTemperatureSensor = curState;
    }
    return stateTemperatureSensor;
	
}

void AppCore::taskControlInPins(void *p)
{
    const int delayControlDamper = 100;
    bool isRunLoad = false;
    int cntDamperTime = 20000;
    if (!gpio->isDamperStateStart()) {
        
        gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinOne);
        do {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            cntDamperTime -= 100;
            if (cntDamperTime <= 0)
                break;
        } while (!gpio->isDamperStateStart());
        
        gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinZero);
    }
    if (cntDamperTime == 0) {
    //тут надо вывести ошибку, что шибер не в нулевом положении за время таймаута
    }
    m_stateDamper = 0;
    while (true) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if (isMenuTests) {
            controlTestPins();
        }
        if (gpio->isEventLoadBread()) {
            if (gpio->isDoorClosed() && !isMenuTests && !isRunLoad) {
                gpio->setPin(GpioDriver::PinLoadBread, GpioDriver::StatePinOne);
                isRunLoad = true;
            }
        }
        if (gpio->isEventDownloadBread()) {
            if (gpio->isDoorClosed() && !isMenuTests && !isRunLoad) {
                gpio->setPin(GpioDriver::PinDownloadBread, GpioDriver::StatePinOne);
                isRunLoad = true;
            }
        }
        if (gpio->isStartKey()) {
            if (stateRun == StateRunIdle) {
                display->switchPage(PageRun);
                stateRun = StateRunStart;
            }
        }
        
        if (gpio->isStopLoadKey() || !gpio->isDoorClosed()) {
            gpio->setPin(GpioDriver::PinLoadBread, GpioDriver::StatePinZero);
            gpio->setPin(GpioDriver::PinDownloadBread, GpioDriver::StatePinZero);
          //  gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinOne);
            isRunLoad = false; 
        }

        if (gpio->isEventSensorTempDrive1()) {
        
        }
        if (gpio->isEventSensorTempDrive2()) {
        
        }
        if (gpio->isEventSensorTempDrive3()) {
        
        }
        //управление подсветкой взависимости от состояния двери
        if (!isMenuTests)
            gpio->setPin(GpioDriver::EnableLightDoorLight, (GpioDriver::StatesPin)gpio->isDoorClosed());
        if (gpio->isDoorClosed())
            m_statesWork.cntPlaySignal = -1;

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
	
    m_statesWork.currentTemp = adc->value2(); 
    m_statesWork.currentTemp1 = thresholdErrorTemperature + 1; //adc->value1();
    if (m_statesWork.currentTemp < thresholdErrorTemperature && m_statesWork.currentTemp1 < thresholdErrorTemperature) {
        return (m_statesWork.currentTemp + m_statesWork.currentTemp1) / 2;		
    }
    if (m_statesWork.currentTemp < thresholdErrorTemperature && m_statesWork.currentTemp1 > thresholdErrorTemperature) {
        return m_statesWork.currentTemp;		
    }
    if (m_statesWork.currentTemp > thresholdErrorTemperature && m_statesWork.currentTemp1 < thresholdErrorTemperature) {
        return (m_statesWork.currentTemp1);		
    }
    return -1;
} 

void AppCore::addWater()
{
    static int timeEndAddWater = 0;
    if (m_statesWork.isWaterStage2 == false) {
        //Добавляем воду если она должна быть добавлена
        if ((stageDuration >= gParams.timeoutAddWater) && (currentWorkMode.stages[currentStage].waterVolume != 0) && (m_statesWork.isWaterStart == false)) {
            m_statesWork.cntH2O = currentWorkMode.stages[currentStage].waterVolume;
            m_statesWork.cntIntWater = 0;
            gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinOne);
            m_statesWork.isWaterStart = true;
        }		
        if (m_statesWork.isWaterStart == true) {
            //проверяем что вода пошла
            if (!m_statesWork.isWaterStage2) {
                if ((stageDuration == gParams.timeoutAddWater + 5) && (m_statesWork.cntH2O == currentWorkMode.stages[currentStage].waterVolume)) {
                    gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinZero);
                    LOG::instance().log("err water sen"); 
                    display->showMessage(PageMessage, 4);
                }
            }
            else {
                if ((stageDuration == timeEndAddWater + currentWorkMode.stages[currentStage].watertimeout + 5) && (m_statesWork.cntH2O == currentWorkMode.stages[currentStage].waterVolume2)) {
                    gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinZero);
                    LOG::instance().log("err water sen"); 
                    display->showMessage(PageMessage, 4);
                }
            }
            
            //обновляем воду на дисплее
            display->sendToDisplay(AddrNumWater, m_statesWork.cntH2O);
            if (m_statesWork.cntH2O <= 0) {
                m_statesWork.isWaterStart = false;
                m_statesWork.isWaterStage2 = true;
                timeEndAddWater = stageDuration;
                currentWorkMode.stages[currentStage].waterVolume = 0;
            }
        }
    }
    else {
        //Добавляем воду если она должна быть добавлена
        if ((stageDuration >= timeEndAddWater + currentWorkMode.stages[currentStage].watertimeout) && (currentWorkMode.stages[currentStage].waterVolume2 != 0) && (m_statesWork.isWaterStart == false)) {
            m_statesWork.cntH2O = currentWorkMode.stages[currentStage].waterVolume2;
            m_statesWork.cntIntWater = 0;
            gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinOne);
            m_statesWork.isWaterStart = true;
        }		
        if (m_statesWork.isWaterStart == true) {
            //проверяем что вода пошла
            if ((stageDuration == timeEndAddWater + currentWorkMode.stages[currentStage].watertimeout + 5) && (m_statesWork.cntH2O == currentWorkMode.stages[currentStage].waterVolume2)) {
                gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinZero);
                LOG::instance().log("err water sen"); 
                display->showMessage(PageMessage, 4);
            }
            //обновляем воду на дисплее
            display->sendToDisplay(AddrNumWater1, m_statesWork.cntH2O);
            if (m_statesWork.cntH2O <= 0) {
                m_statesWork.isWaterStart = false;
                m_statesWork.isWaterStage2 = false;
                timeEndAddWater = stageDuration;
                currentWorkMode.stages[currentStage].waterVolume2 = 0;
            }
        }
    }
}

void AppCore::checkPinState(bool event, uint16_t mask, uint16_t addrIcon)
{
    if (event) {
        if (!(m_stateInpinTestMenu & mask)) {
            m_stateInpinTestMenu |= mask;
            display->sendToDisplay(addrIcon, 1);
        }
    }
    else {
        if ((m_stateInpinTestMenu & mask)) {
            m_stateInpinTestMenu &= (~mask);
            display->sendToDisplay(addrIcon, 0);
        }
    }
}

void AppCore::controlTestPins()
{

    checkPinState(gpio->isDoorClosed(), EventDoor, addrIconDoor);
    checkPinState(gpio->isDamperStateStart(), EventDamper0, addrIconDamperZero);
    checkPinState(gpio->isEventDownloadBread(), EventDownload, addrIconKeyDownload);
    checkPinState(gpio->isEventLoadBread(), EventLoad, addrIconKeyLoad);
    checkPinState(gpio->isEventSensorTempDrive1(), EventRotor1, addrIconRotor1);
    checkPinState(gpio->isEventSensorTempDrive2(), EventRotor2, addrIconRotor2);
    checkPinState(gpio->isEventSensorTempDrive2(), EventRotor3, addrIconRotor3);
    checkPinState(gpio->isStartKey(), EventStart, addrIconKeyStart);
    checkPinState(gpio->isStopLoadKey(), EventStop, addrIconKeyStop);
        

}

