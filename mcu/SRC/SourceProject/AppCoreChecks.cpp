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
        //LOG::instance().log("err temp sen2"); 
    }
    else
    {
        curState = NoFailSensorTemperature;		
    }
    if (adc->value1() > thresholdErrorTemperature) {
	    curState = static_cast<eFailSensorTemperature>(curState | NoFailSensorTemperature);
        //LOG::instance().log("err temp sen1"); 
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

bool AppCore::moveDamperToStartPositon()
{
	const int delayControlDamper = 100; // Период проверки положения шибера (мс)
	int cntDamperTime = 20000; // Таймаут инициализации шибера (20 секунд)
	// Инициализация шибера - приведение в нулевое положение
	gpio->disableIntDamperState();
	if (!gpio->isDamperStateStart()) {
		// Активируем привод шибера
		gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinOne);
        
		// Ожидаем пока шибер достигнет начального положения или сработает таймаут
		do {
			vTaskDelay(delayControlDamper / portTICK_PERIOD_MS);
			cntDamperTime -= delayControlDamper;
			if (cntDamperTime <= 0 || stateRun == StateRunStop)
				break;
		} while (!gpio->isDamperStateStart());
        
		// Отключаем привод шибера
		gpio->setPin(GpioDriver::PinShiberX, GpioDriver::StatePinZero);
	}
	gpio->enableIntDamperState();
	m_stateDamper = 0;
	if (cntDamperTime <= 0 && !gpio->isDamperStateStart())
		return false;
	return true;
}


/**
 * @brief Задача управления входными пинами и контроля состояния системы
 * 
 * Основные функции:
 * 1. Инициализация положения шибера (привод загрузки) в нулевое положение
 * 2. Обработка событий нажатия кнопок и изменения состояний
 * 3. Управление процессами загрузки и выгрузки хлеба
 * 4. Контроль состояния двери и связанных с ней функций
 * 5. Обработка тестового режима меню
 * 
 * @param p Указатель на параметры задачи (не используется)
 */
void AppCore::taskControlInPins(void *p)
{
	
	bool isRunLoad = false; // Флаг выполнения процесса загрузки/выгрузки
	
    
	
    
	// Обработка ошибки инициализации шибера
	if (moveDamperToStartPositon() == false) {
        
		// TODO: Вывести ошибку - шибер не достиг нулевого положения за время таймаута
		showIconError(DamperFailure);
	}
    
	m_stateDamper = 0; // Сброс состояния шибера
    
	// Основной цикл задачи
	while (true) {
		vTaskDelay(10 / portTICK_PERIOD_MS); // Задержка 10 мс
        
		// Режим тестового меню - приоритетная обработка
		if (isMenuTests) {
			controlTestPins(); // Управление пинами в тестовом режиме
			continue; // Пропускаем остальную логику в тестовом режиме
		}
        
		// Обработка события загрузки хлеба
		if (gpio->isEventLoadBread()) {
			if (gpio->isDoorOpen() && !isMenuTests && !isRunLoad) {
				gpio->setPin(GpioDriver::PinLoadBread, GpioDriver::StatePinOne);
				isRunLoad = true; // Устанавливаем флаг выполнения процесса
			}
		}
        
		// Обработка события выгрузки хлеба
		if (gpio->isEventDownloadBread()) {
			if (gpio->isDoorOpen() && !isMenuTests && !isRunLoad) {
				gpio->setPin(GpioDriver::PinDownloadBread, GpioDriver::StatePinOne);
				isRunLoad = true; // Устанавливаем флаг выполнения процесса
			}
		}
        
		// Обработка нажатия кнопки START
		if (gpio->isStartKey()) {
			if (stateRun == StateRunIdle) {//если находимся в состоянии простоя
				display->switchPage(PageRun); // Переключаем на страницу выполнения
				stateRun = StateRunStart; // Меняем состояние системы
			}
			else { // если печем - то это событие будет выполнятся как СТОП
				if (stateRun != StateRunStop) { 
					stateRun = StateRunStop;
					xSemaphoreGive(xSemPeriodic);
				}
			}
		}
        
		// Остановка процессов загрузки/выгрузки при нажатии STOP или закрытии двери
		if (gpio->isStopLoadKey() || !gpio->isDoorOpen()) {
			gpio->setPin(GpioDriver::PinLoadBread, GpioDriver::StatePinZero);
			gpio->setPin(GpioDriver::PinDownloadBread, GpioDriver::StatePinZero);
			isRunLoad = false; // Сбрасываем флаг выполнения процесса
		}
        
		// Обработка событий датчиков температуры приводов (заглушки)
		if (gpio->isEventSensorTempDrive1()) {
			// TODO: Реализовать обработку
		}
		if (gpio->isEventSensorTempDrive2()) {
			// TODO: Реализовать обработку
		}
		if (gpio->isEventSensorTempDrive3()) {
			// TODO: Реализовать обработку
		}
        
		// Управление подсветкой в зависимости от состояния двери
		if (!isMenuTests)
			gpio->setPin(GpioDriver::EnableLightDoorLight, (GpioDriver::StatesPin)gpio->isDoorOpen());
        
		// Сброс счетчика воспроизведения звука при открытой двери
		if (gpio->isDoorOpen())
			m_statesWork.cntPlaySignal = -1; // Признак, что не надо продолжать воспроизводить звук
	}
}

/**
 * @brief Выбирает и возвращает корректное значение температуры из двух датчиков.
 * 
 * Функция выполняет следующие действия:
 * 1. В режиме тестового менения (isMenuTests == true) отображает на дисплее 
 *    сырые значения с обоих датчиков температуры
 * 2. Проверяет значения обоих датчиков на превышение порога ошибки
 * 3. Возвращает наиболее подходящее значение температуры согласно логике:
 *    - Если оба датчика исправны - среднее арифметическое
 *    - Если один датчик неисправен - значение исправного датчика
 *    - Если оба датчика неисправны - возвращает -1
 * 
 * @note В текущей реализации значение m_statesWork.currentTemp1 принудительно 
 *       устанавливается выше порога ошибки (thresholdErrorTemperature + 1),
 *       что фактически эмулирует неисправность второго датчика.
 * 
 * @return float - Корректное значение температуры или -1 в случае ошибки обоих датчиков
 * 
 * @warning Режим тестового менения доступен только при isMenuTests == true
 * @warning Порог ошибки температуры задается thresholdErrorTemperature
 */
float AppCore::selectTemperature() {
	
    if (isMenuTests) {
        static char buff[32];
        memset(buff, 0, 32);
        float t1 =  adc->value1();
        float t2 =  adc->value2();
		
        uint16_t len = sprintf(buff, "t1=%d, t2=%d", (int)t1, (int)t2);
        display->sendToDisplay(addrStrTempTest, len+1, (uint8_t*)buff);
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
    if (m_statesWork.isWaterStage2 == false) {//первый этам добавления воды
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
                    //LOG::instance().log("err water sen"); 
                    display->showMessage(PageMessage, 4);
                }
            }
            else {
                if ((stageDuration == timeEndAddWater + currentWorkMode.stages[currentStage].watertimeout + 5) && (m_statesWork.cntH2O == currentWorkMode.stages[currentStage].waterVolume2)) {
                    gpio->setPin(GpioDriver::GpioDriver::PinH2O, GpioDriver::StatePinZero);
                    //LOG::instance().log("err water sen"); 
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
    else {//второй этап добавлления
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
                //LOG::instance().log("err water sen"); 
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
    checkPinState(gpio->isDoorOpen(), EventDoor, addrIconDoor);
    checkPinState(gpio->isDamperStateStart(), EventDamper0, addrIconDamperZero);
    checkPinState(gpio->levelDownloadBread(), EventDownload, addrIconKeyDownload);
    checkPinState(gpio->levelLoadBread(), EventLoad, addrIconKeyLoad);
    checkPinState(gpio->levelSensorTempDrive1(), EventRotor1, addrIconRotor1);
    checkPinState(gpio->levelSensorTempDrive2(), EventRotor2, addrIconRotor2);
    checkPinState(gpio->levelSensorTempDrive3(), EventRotor3, addrIconRotor3);
    checkPinState(gpio->levelStartKey(), EventStart, addrIconKeyStart);
    checkPinState(gpio->levelStopLoadKey(), EventStop, addrIconKeyStop);
        

}

void AppCore::showIconError(uint16_t codeError)
{
	listError.push(codeError);
	display->sendToDisplay(addrIconFailure, 1);
}

