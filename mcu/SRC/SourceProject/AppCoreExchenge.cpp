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

#define BOOT_ADDRESS    0x08040000//адрес начала программы загрузчика


static void tskExchange(void *p) {
    AppCore::instance().taskExchange();
}

void AppCore::initExchange() {
    queExchange = xQueueCreate(32, sizeof(PackageNetworkFormat));
    xReturned = xTaskCreate(
                            tskExchange,       /* Function that implements the task. */
                            "exch",          /* Text name for the task. */
                            256,      /* Stack size in words, not bytes. */
                            (void *) 1,    /* Parameter passed into the task. */
                            tskIDLE_PRIORITY,/* Priority at which the task is created. */
                            &xHandleExchange); /* Used to pass out the created task's handle. */
	
	
	
    objDataExchenge.procPack = [=](PackageNetworkFormat & p) {
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

void AppCore::taskExchange(void *p) {
    PackageNetworkFormat package;
    while (true) {
        xQueueReceive(queExchange, &package, portMAX_DELAY);
    }
}




void AppCore::parsePackDisplay(const uint16_t id, uint8_t len, uint8_t* data) {
    uint8_t cmd = data[0];
    switch (id) {
        case addrStateWifiSSID: {
                int l = 0;
                uint8_t *pName = data + 1;
                while (1) {
                    if (*pName == 0xff || *pName == 0) {
                        break;
                    }
                    pName++;
                    l++;
                }
                memset(gParams.wifiSSID, 0, LenWifiSSID);
                memcpy(gParams.wifiSSID, data + 1, l);
                break;
            }
        case addrStateWifiPassword: {
                int l = 0;
                uint8_t *pName = data + 1;
                while (1) {
                    if (*pName == 0xff || *pName == 0) {
                        break;
                    }
                    pName++;
                    l++;
                }
                memset(gParams.wifiPassword, 0, LenWifiPassword);
                memcpy(gParams.wifiPassword, data + 1, l);
                display->switchPage(currentPage);
                objDataExchenge.sendPackage(IdWifiSSID, 1, strlen(gParams.wifiSSID), (uint8_t*)gParams.wifiSSID);
                objDataExchenge.sendPackage(IdWifiPassword, 1, strlen(gParams.wifiPassword), (uint8_t*)gParams.wifiPassword);
#ifdef EEPROM_MEMORY
                writeParamsToEeprom();
#else
                writeGlobalParams();
#endif
                //
                break;
            }
        case AddrCurrentPage:
            currentPage = (data[1] << 8) | data[2];
            if (newPage == PageWifiMenu) {
                display->switchPage(PageWifiMenu);
                display->sendToDisplay(addrStateWifi, gParams.stateWifi);
                display->sendToDisplay(addrStateWifiSSID, strlen(gParams.wifiSSID), (uint8_t*)gParams.wifiSSID);
                display->sendToDisplay(addrStateWifiPassword, strlen(gParams.wifiPassword), (uint8_t*)gParams.wifiPassword);
            }
            break;
	    case addrIconFailure: {
		    uint16_t icon = data[2] | (data[1] << 8);
		    m_ErrorCode = 0x5555;
		    if (icon == 1) {
			    if (!listError.empty()) {
				    icon = listError.front();
				    listError.pop();
				    display->showMessage(PageMessage, icon);
			    }
		    }
			    
	    }
	    break;
        case addrUpT:
            gpio->setPin(GpioDriver::PinTemperatureUp, (GpioDriver::StatesPin)data[2]);
            break;
        case addrDownT:
            gpio->setPin(GpioDriver::PinTemperatureDown, (GpioDriver::StatesPin)data[2]);
            break;
        case addrEnFan:
            gpio->setPin(GpioDriver::PinFanLowSpeed, (GpioDriver::StatesPin)data[2]);
            break;
        case addrEnFanFast:
            gpio->setPin(GpioDriver::PinFanFastSpeed, (GpioDriver::StatesPin)data[2]);
        break;
        case addrHoodVisor:
            gpio->setPin(GpioDriver::HoodVisor, (GpioDriver::StatesPin)data[2]);
        break;
        case addrMainHood:
            gpio->setPin(GpioDriver::MainHood, (GpioDriver::StatesPin)data[2]);
        break;
        case addrEnableLightDoorLight:
            gpio->setPin(GpioDriver::EnableLightDoorLight, (GpioDriver::StatesPin)data[2]);
        break;
        case addrPinDownloadBread:
        if(gpio->isDoorOpen())
            gpio->setPin(GpioDriver::PinDownloadBread, (GpioDriver::StatesPin)data[2]);
        break;
        case addrPinLoadBread:
            if (gpio->isDoorOpen())
                gpio->setPin(GpioDriver::PinLoadBread, (GpioDriver::StatesPin)data[2]);
        break;
        case addrWater:
            gpio->setPin(GpioDriver::GpioDriver::PinH2O, (GpioDriver::StatesPin)data[2]);
            break;
        case addrDamperOpen:
            gpio->setPin(GpioDriver::PinShiberO, (GpioDriver::StatesPin)data[2]);
            /*if (data[2] == GpioDriver::StatePinOne)
                xTimerStart(timerDamper, 0);*/
            break;
        case addrDamperClose:
            gpio->setPin(GpioDriver::PinShiberX, (GpioDriver::StatesPin)data[2]);
          /*  if (data[2] == GpioDriver::StatePinOne)
                xTimerStart(timerDamper, 0);*/
            break;
        case addrGreenLed:
            if (data[2])
                gpio->enableGreenLed();
            else 
                gpio->disableGreenLed();
            break;
        case addrYellowLed:
            if (data[2])
                gpio->enableYellowLed();
            else 
                gpio->disableYellowLed();
            break;
        case addrMainRele:
            gpio->setPin(GpioDriver::GpioDriver::GlobalEnable, (GpioDriver::StatesPin)data[2]);
            break;
        case AddrNumWater:
            currentWorkMode.stages[currentStage].waterVolume = data[2] | (data[1] << 8);
            break;
        case AddrNumDurationNew: {
                short delta = (short)((data[2] | (data[1] << 8)) * 60);
                if (delta < 0 && abs(delta)> TO_SECONDS(currentWorkMode.stages[currentStage].duration)) {
	                delta = -(TO_SECONDS(currentWorkMode.stages[currentStage].duration) - 1) ;
                    modeDuration -= stageDuration;
                    stageDuration = 0;
				
                }
	        currentWorkMode.stages[currentStage].duration += delta ;
				
                commonDuration += delta;
                break;
            }
        case AddrNumTemperature:
            currentWorkMode.stages[currentStage].temperature = data[2] | (data[1] << 8);
            break;
        case AddrNumDamper:
#ifdef EXTENDED_SETTINGS
        currentWorkMode.stages[currentStage].damper[m_currentIndexDamper].state = data[2] | (data[1] << 8);
#else
        currentWorkMode.stages[currentStage].damper = data[2] | (data[1] << 8);
	    m_stateDamper = 0;
#endif
	    /*if (currentWorkMode.stages[currentStage].damper[m_currentIndexDamper].state > m_stateDamper) {
	        m_signedStateDamper = 1;
            gpio->setPin(GpioDriver::PinShiberX, (GpioDriver::StatePinOne));
            xTimerStart(timerDamper, 0);
            
        }
	    else if (currentWorkMode.stages[currentStage].damper[m_currentIndexDamper].state < m_stateDamper)
        {
	        m_signedStateDamper = -1;
            gpio->setPin(GpioDriver::PinShiberO, (GpioDriver::StatePinOne));
            xTimerStart(timerDamper, 0);
            
        }*/

            break;
        case AddrNumFan:
#ifdef EXTENDED_SETTINGS
	        currentWorkMode.stages[currentStage].fan[m_currentIndexFan].state = data[2] | (data[1] << 8);
#else
            currentWorkMode.stages[currentStage].fan = data[2] | (data[1] << 8);
            gpio->setPin(GpioDriver::PinFanLowSpeed, (GpioDriver::StatesPin)currentWorkMode.stages[currentStage].fan);
#endif
            break;
        case addrCurrentSound:
            gParams.numSound = data[1];
            break;
        case addrCurrentVolume:
            gParams.volume = data[1];
            break;
        case addrPassword: {
                uint16_t pwd = data[2] | (data[1] << 8);
                if (pwd == password) {
                    //m_pageSettings = PageExternSettings;
                    display->switchPage(PageExternSettings);
                    display->sendToDisplayF(addrK1, gParams.k1);
                    display->sendToDisplayF(addrK2, gParams.k2);
                    display->sendToDisplay(addrPeriod, gParams.period);
                    display->sendToDisplay(addrAddWater, gParams.timeoutAddWater);
                    display->sendToDisplay(addrWaterOneVolume, gParams.waterOneVolume);
                    display->sendToDisplayF(addrAmpSensTem, gParams.ampSensTemp);

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
        case addrWaterOneVolume:
        {
            gParams.waterOneVolume = data[2] | (data[1] << 8);

            break;
        }
        case addrAmpSensTem: {
            uint32_t t = data[4] | (data[3] << 8) | (data[2] << 16) | (data[1] << 24);
            memcpy(&gParams.ampSensTemp, &t, sizeof(uint32_t));
            adc->setCoeff(gParams.ampSensTemp);
            break;
        }

        case addrAddWater:
            gParams.timeoutAddWater = data[2] | (data[1] << 8);
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
                m_rtc->setDate(data[1], data[2], data[3]);
                m_rtc->setTime(data[5], data[6], data[7]);
                display->sendToDisplay(CmdSetDateTime, 8, helperBuf);
                break;
            }
        case CmdNumProgramm: {
                uint16_t key = data[2] | (data[1] << 8);
                keyEvent(key);
                break;
            }
        case addrStateWifi: {
	
                gParams.stateWifi = data[2] | (data[1] << 8);
                display->sendToDisplay(addrStateWifiIcon, iconIndexWifi[gParams.stateWifi]);
                objDataExchenge.sendPackage(IdWifiState, 1, sizeof(gParams.stateWifi), (uint8_t*)&gParams.stateWifi);
#ifdef EEPROM_MEMORY
                writeParamsToEeprom();
#else
                writeGlobalParams();
#endif
                break;
            }
        case AddrRtc:
            m_rtc->setDate(data[1], data[2], data[3]);
            m_rtc->setTime(data[5], data[6], data[7]);
            break;
    case addrIsIdleMode:
	    m_statesWork.isModeIdleControlTemperature = ((bool)data[2]);
	    if (m_statesWork.isModeIdleControlTemperature) {
		    m_statesWork.cntContolDownTemperature = 30;
		    gpio->setPin(GpioDriver::PinTemperatureUp, (GpioDriver::StatePinZero));
		    gpio->setPin(GpioDriver::PinTemperatureDown, (GpioDriver::StatePinOne));
		    gpio->setPin(GpioDriver::PinShiberO, (GpioDriver::StatePinOne));
		    gpio->setPin(GpioDriver::MainHood, GpioDriver::StatePinOne);
		    gpio->setPin(GpioDriver::PinFanFastSpeed, (GpioDriver::StatePinOne));
	    }
	    else {
		    if (m_statesWork.cntContolDownTemperature) {
			    gpio->setPin(GpioDriver::PinTemperatureDown, (GpioDriver::StatePinZero));
			    m_statesWork.cntContolDownTemperature = 0;
		    }
		    if (m_statesWork.timeoutPeriodicTask == portMAX_DELAY) {
			    m_statesWork.timeoutPeriodicTask = pdMS_TO_TICKS(1000);
			    xSemaphoreGive(xSemPeriodic);
		    }
		    	
	    }
	    break;
    
        default:
            p_widget->changeParams(id, len, data);
            break;
    }
}

void AppCore::keyEvent(uint16_t key) {
    switch (key) {
        case ReturnCodeKeyStart:
            stateRun = StateRunStart;
            xSemaphoreGive(xSemPeriodic);
            break;
        case ReturnCodeKeyStop:
            stateRun = StateRunStop;
            LOG::instance().log("stop");
            break;
        case ReturnCodeKeyExitMenuTest:
            gpio->enableGreenLed();
            if (stateRun == StateRunStart || stateRun == StateRunWork) {
                gpio->enableYellowLed();
            }
            else {
                gpio->disableYellowLed();
            }
            gpio->setPin(GpioDriver::GlobalEnable, GpioDriver::StatePinZero);
            isMenuTests = false;
            break;
        case ReturnCodeKeySoundTest:
        case ReturnCodeKeyPlaySoundTest:
            display->playSound(gParams.numSound, gParams.volume);
            break;
        case ReturnCodeKeyInMenuTest:
            isMenuTests = true;
            m_stateInpinTestMenu = NoEvent;
            break;
			
        case ReturnCodeKeyInMenuSettingsProgramms:
            p_widget = lstProgramsEdit;
            p_widget->resetWidget();
            break;
        case ReturnCodeKeyWifiMenu:
            newPage = PageWifiMenu;
            display->getDataFromDisplay(AddrCurrentPage, 0, 2); 
            break;
        case ReturnCodeKeyWifiMenuExit:
            display->switchPage(currentPage);
			
            break;
	    case ReturnCodeKeyHideMsg1 :
	    display->hideMessage();
	    break;
        case ReturnCodeKeyHideMsg :
            display->hideMessage();
	    if (m_statesWork.timeoutPlayAfterRun == SOUND_ON) {
		    xSemaphoreTake(xSemAccessCtrlSound, portMAX_DELAY);
		    m_statesWork.timeoutPlayAfterRun = SOUND_OFF;
		    gpio->disableGreenLed();
		    xSemaphoreGive(xSemAccessCtrlSound);
	    }

	    if (m_ErrorCode == 0x5555) {
		    if (!listError.empty()) {
			    int icon = listError.front();
			    listError.pop();
			    display->showMessage(PageMessage, icon);
		    }
		    else {
			    m_ErrorCode = 0;
			    display->sendToDisplay(addrIconFailure, 0);
		    }
	    }
            break;
        case ReturnCodeKeyMainSettings:
            m_pageExitSettings = PageMain;
            display->switchPage(m_pageSettings);
            break;
        case ReturnCodeKeyRunSettings:
            m_pageExitSettings = PageRun;
            display->switchPage(m_pageSettings);
            break;
        case ReturnCodeKeyApplySettings:
            display->switchPage(m_pageExitSettings);
            break;
        case ReturnCodeKeyExtendedSettings:
            {
#ifdef EEPROM_MEMORY
                writeParamsToEeprom();
#else
				
                if (memcmp((void*)FlashAddrGlobalParams, &gParams, sizeof(RomParams)) != 0)
                    writeGlobalParams();
#endif
				
				
                break;
            }
        default: 
            p_widget = p_widget->keyEvent(key);
            break;
    }
	
}

void AppCore::getSizeWRectangle(const WorkMode &mode, uint16_t *wList) {
	
    int i;
    int xStart = xProgresStage, xEnd;

    for (i = 0; i < mode.numStage; i++) {
        wList[2*i] = xStart;
	    xEnd = xStart + TO_SECONDS(mode.stages[i].duration) * (wProgresStage - mode.numStage * 2) / (commonDuration) ;
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
#ifdef EXTENDED_SETTINGS
    display->sendToDisplay(AddrNumFan, currentWorkMode.stages[currentStage].fan[m_currentIndexFan].state);
    display->sendToDisplay(AddrNumDamper, currentWorkMode.stages[currentStage].damper[m_currentIndexDamper].state);
#else 
    display->sendToDisplay(AddrNumFan, currentWorkMode.stages[currentStage].fan);
    display->sendToDisplay(AddrNumDamper, currentWorkMode.stages[currentStage].damper);
#endif
	
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
