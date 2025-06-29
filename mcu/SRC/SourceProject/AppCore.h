#pragma once
#include  <Periphery/GpioDriver.h>
#include <FreeRTOS.h>
#include <task.h>
#include <portmacro.h>
#include <AdcDriver.h>
#include <DisplayDriver.h>
#include <vector>
#include <WorkModeEdit.h>
#include "typedef.h"
#include "timers.h"
#include "Uart5.h"
#include <../../../common/dataexchenge.h>
#include <globals.h>
#include <Rtc.h>
#include <log.h>

#define NumItemList  (7)        //!< максимальное количество строчек в списке выбора программы
#define NumItemListEdit  (5)    //!< максимальное количество строчек в редакторе программ


constexpr uint16_t xProgresStage = 17;       //!< позиция по оси x на дисплее, с которой начинает отрисовываться прогресс бар выполения программы
constexpr uint16_t yProgresStage = 175;      //!< позиция по оси y на дисплее, с которой начинает отрисовываться прогресс бар выполения программы
constexpr uint16_t hProgresStage = 60;       //!< высота прогресс бара выполения программы
constexpr uint16_t wProgresStage = 568;      //!< ширина прогресс бара выполения программы

/**
    @enum  ePages
    @brief перечисление описывает номера страниц на дисплее
**/
enum ePages {
    PageMain           = 0, //!< стартовая страница
    PageRun            = 6, //!< страница выполнения программы
    PageSettings       = 2, //!< страница настроек
    PageMessage        = 8, //!< страница показа сообщений
    PageExternSettings = 23,//!< страница расширенных настроек
    PageWifiMenu       = 27 //!< страница настроек беспроводной сети
};

/**
    @enum  eStateWifi
    @brief состояние работы беспроводной сети
**/
enum eStateWifi {
    WifiOff = 0,     //!< отключено
    WifiOn           //!< включено
};

/**
    @enum  eFailSensorTemperature
    @brief определяет маски ошибок на датчиках температуры
**/
enum eFailSensorTemperature
{
    NoFailSensorTemperature = 0,  //!< нет неисправности
    FailSensorTemperature1 =  1<<0 ,       //!< неисправен датчик 1
    FailSensorTemperature2 = 1<<1 //!< неисправен датчик 2

};

constexpr uint16_t password = 2024;   //!< пароль для входа в расширенное меню Настроек

/**
    @enum  Перечисление StateRun
    @brief Определяет состояния выполнения программы выпечки
**/
enum StateRun { 
    StateRunIdle,       //!< состояние простоя
    StateRunStart,      //!< состояние начала выполения 
    StateRunWork,       //!< состояние выполнения
    StateRunStop,       //!< состояние завершения программы
    StateRunError,      //!< состояние ошибки
};
/**

    @class   AppCore
    @brief   Главный класс приложения
    @details ~

**/
class AppCore {
public:
    AppCore();
    ~AppCore();
    static AppCore &instance();
    /**
        @brief периодический поток, в котором происходит выполнение программы выпечки
        @param p - не используется
    **/
    void taskPeriodic(void *p = 0);
    /**
        @brief поток обработки данных от ESP
        @param p - не используется
    **/
    void taskExchange(void *p = 0);

    /**
        @brief Обработчик таймаута таймера контроля шибера
        @param timer - 
    **/
    void eventTimeoutDamper(TimerHandle_t timer);

    /**
        @brief обработчик данных от дисплея
        @param id   - идетификатор данных
        @param len  - длина данных
        @param data - данные
    **/
    void parsePackDisplay(const uint16_t id, uint8_t len, uint8_t* data);
    /**
        @brief обработчик касаний на дисплее
        @param key - идентификатор касания
    **/
    void keyEvent(uint16_t key);
    /**
        @brief производит инициализацию переферии
    **/
    void initHal();
    /**
        @brief производит инициализацию переменных для работы операционной системы
    **/
    void initOsal();
    /**
        @brief производит инициализацию списков рабочих программ выпечки
    **/
    void initText();
    /**
        @brief производит инициализацию программа значениями по умолчанию
    **/
    void initDefaultPrograms();
    /**
        @brief производит чтение данных программ из ПЗУ
    **/
    void readPrograms();
    /**
        @brief производит запись параметров во флеш МКУ
    **/
    void writeGlobalParams();
    /**
        @brief производит запись данных программ в микросхему ПЗУ
    **/
    void writProgramsToEeprom();
    /**
        @brief производит запись параметров в микросхему ПЗУ
    **/
    void writeParamsToEeprom();
    /**
        @brief производит инициализацию программы случайными значениями
        @param name      - названиие программы
        @param numStages - количество этапов
    **/
    void fillProgram(const std::string &name, const uint16_t numStages);
    /**
        @brief производит отправку на дисплей данных для обновления прогресса выполнения программы выпечки
        @param value - значение в процетах
    **/
    void updateProgressBar(uint16_t value);
    /**
        @brief  определяет с какого датчика нужно брать значение температуры
        @retval  - температура
    **/
    float selectTemperature();
    /**
        @brief отрисоваывает общий прогресс выпечки
    **/
    void paintStageProgress();
    WorkMode currentWorkMode; //!< текущая программы выпечки
    /**
        @brief вычисляет ширину прогресса для всех этапов программы выпечки
        @param mode  - текущия программа выпечки
        @param wList - список значений ширины прогресса
    **/
    void getSizeWRectangle(const WorkMode &mode, uint16_t *wList);
    /**
        @brief обновляет значения на экране при выполнении программы выпечки
    **/
    void updateParamStage();
    /**
        @brief обновляет оставшееся время выпечки
        @param sec - оставшееся время в секундах 
    **/
    void updateTime(uint16_t sec);
    /**
        @brief  производит вычисление контрольной сумму crc32
        @param  buf - последовательность, для которой считается контрольная сумма
        @param  len - длина последовательности
        @retval     - значение контрольной суммы
    **/
    static unsigned int CRC32_function(unsigned char *buf, unsigned long len);
    /**
        @brief  проверяет работоспособность датчиков температуры
        @retval  - возвращает маску несправности датчиков
    **/
    eFailSensorTemperature checkTemperatureSensors();
	
private:
    /**
        @brief производит обработку данны
        @param p - 
    **/
    void procUartData(const PackageNetworkFormat&p);
    void initExchange();
    TimerHandle_t timerYellow;
    TimerHandle_t timerGreen;
    uint16_t countGreenLeds{0};
    uint16_t countYellowLeds{0};
    uint16_t stateRun;
    int32_t commonDuration;
    uint16_t currentStage;
    uint16_t stageDuration;
    uint16_t modeDuration;
    uint16_t allModeDuration;
    uint16_t currentPage;
    uint16_t newPage;
	
    float Utemp1;
    float Utemp2;
    uint16_t temperature;
    float prevTemperature;

    void correctTemperature(float &currentTemp, uint16_t &targetTemp);
	
    xSemaphoreHandle xSemPeriodic;
    xQueueHandle queExchange;
    GpioDriver *gpio;
	
	
    AdcDriver *adc;

    DisplayDriver *display;
	
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;
    TaskHandle_t xHandleExchange = NULL;

    uint8_t helperBuf[256];
	
    MyList *lstPrograms;
    WorkModeEdit *lstProgramsEdit;
    Widget *p_widget;
	
    std::vector<WorkMode> m_programs;
	
    RomParams gParams;
	
    eFailSensorTemperature stateTemperatureSensor{NoFailSensorTemperature};
    const uint16_t iconIndexWifi[2] = {3, 2};
	
    Rtc *m_rtc;
	
    uint16_t m_pageSettings{PageSettings};
    uint16_t m_pageExitSettings;
	
    const float thresholdErrorTemperature = 330;
	
    xTimerHandle timerDamper; ///< таймер отключения шибера
    int16_t timeBlinkYellow{0};
    bool isMenuTests{false};
    uint16_t indexProgramms;
    uint16_t indexProgrammsData;
};

