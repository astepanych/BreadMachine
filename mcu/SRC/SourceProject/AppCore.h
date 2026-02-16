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
#include <queue>

#define NumItemList  (7)        //!< максимальное количество строчек в списке выбора программы
#define NumItemListEdit  (5)    //!< максимальное количество строчек в редакторе программ

#define SOUND_OFF (0)
#define SOUND_ON_3      (3)
#define SOUND_ON (100)

constexpr uint16_t xProgresStage = 17;       //!< позиция по оси x на дисплее, с которой начинает отрисовываться прогресс бар выполения программы
constexpr uint16_t yProgresStage = 175;      //!< позиция по оси y на дисплее, с которой начинает отрисовываться прогресс бар выполения программы
constexpr uint16_t hProgresStage = 60;       //!< высота прогресс бара выполения программы
constexpr uint16_t wProgresStage = 568;      //!< ширина прогресс бара выполения программы

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
    bool isWaterStage2; // флаг отвечающий за номер этапа добавления воды
    int cntIntWater;
    uint16_t periodWater;
    uint16_t currentIndexProgramm;
    int cntPlaySignal;
	int timeoutPlayAfterRun;
	uint16_t m_targetTemperature;
	bool isModeIdleControlTemperature;
	uint16_t cntContolDownTemperature;
	uint16_t timeoutOffMachine;
	bool isOffMachine;
	int timeoutPeriodicTask;
	
};

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
    PageMessage1       = 26, //!< страница показа сообщений
    PageWifiMenu       = 27 //!< страница настроек беспроводной сети
};

/**
    @enum  ePages
    @brief перечисление описывает номера страниц на дисплее
**/
enum eMessages {
    ProgrammEnd = 0,
    FailureTemperatureSensor1,
    FailureTemperatureSensor2,
    FailureTemperatureSensors,
    FailureWaterSensor,
    ReserveMessage,
    DoorNoClosed,
    DataWorkModeBad,//повреждены данные программы
    DamperFailure//неисправность шибера
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

enum eInputEventPins
{
    NoEvent = 0,
    EventWater = 1<<0,
    EventDoor =  1<<1,
    EventLoad = 1 << 2,
    EventDownload = 1<<3,
    EventStart = 1<<4,
    EventDamper0 = 1<<5,
    EventRotor1 = 1<<6,
    EventRotor2 = 1<<7,
    EventRotor3 = 1<<8,
    EventStop = 1<<9,
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

    void taskControlInPins(void *p = 0);
	void taskControlLeds(void *p = 0);
	void taskControlSound(void *p = 0);
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

	void initTasks();
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


	static uint16_t calculateCRC16(const uint8_t* data, size_t length);
    /**
        @brief  проверяет работоспособность датчиков температуры
        @retval  - возвращает маску несправности датчиков
    **/
    eFailSensorTemperature checkTemperatureSensors();

    void addWater();

    void controlTestPins();

	bool moveDamperToStartPositon();
	
private:
    /**
        @brief производит обработку данны
        @param p - 
    **/
    void initExchange();
    void checkPinState(bool state, uint16_t mask, uint16_t addrIcon);
    TimerHandle_t timerYellow;
    TimerHandle_t timerGreen;
    uint16_t countGreenLeds{0};
    uint16_t countYellowLeds{0};
    uint16_t stateRun;
    int32_t commonDuration;
    uint16_t currentStage;
	uint16_t m_currentIndexFan;
	uint16_t m_currentIntervalFanDuration;
	uint16_t m_currentIndexDamper;
	uint16_t m_prevsIntervalsDamperDuration;
    uint16_t stageDuration;
    uint16_t modeDuration;
    uint16_t allModeDuration;
    uint16_t currentPage;
    uint16_t newPage;
	
    float Utemp1;
    float Utemp2;
    uint16_t temperature;
    float prevTemperature;

    void correctTemperature(float &currentTemp, uint16_t targetTemp);
	
    xSemaphoreHandle xSemPeriodic;
    xQueueHandle queExchange;
    GpioDriver *gpio;
	
	
    AdcDriver *adc;

    DisplayDriver *display;
	
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;
    TaskHandle_t xHandlePull = NULL;
    TaskHandle_t xHandleExchange = NULL;
	TaskHandle_t xHandleTaskCtrlLeds = NULL;
	xSemaphoreHandle xSemTaskCtrlLeds;
	TaskHandle_t xHandleTaksCtrlSound = NULL;
	xSemaphoreHandle xSemTaskCtrlSound;
	xSemaphoreHandle xSemAccessCtrlSound;

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
    uint16_t m_stateDamper;
    int16_t m_signedStateDamper;
    StateWork m_statesWork;
    int m_stateInpinTestMenu;
	/**
	 * @brief Константа задержки после сброса дисплея (в миллисекундах)
	 * 
	 * Определяет время задержки после выполнения команды сброса дисплея
	 * перед продолжением инициализации системы.
	 */
	const int delayAfterResetDisplay = 1800;

	/**
	 * @brief Таймаут проверки потока воды (в секундах)
	 * 
	 * Время, по истечении которого система проверяет, поступила ли вода
	 * после открытия клапана. Если вода не поступила - генерируется ошибка.
	 */
	const int checkWaterFlowTimeout = 5;

	/**
	 * @brief Время предварительного уведомления о завершении (в секундах)
	 * 
	 * За сколько секунд до окончания работы начать воспроизводить
	 * звуковое оповещение о скором завершении процесса.
	 */
	const int preFinishSoundTime = 30;

	/**
	 * @brief Время включения вытяжки перед завершением (в секундах)
	 * 
	 * За сколько секунд до окончания работы включить вытяжку
	 * для удаления дыма и запахов.
	 */
	const int preFinishVentTime = 300;



	/**
	 * @brief Инициализация состояния работы
	 * 
	 * Сбрасывает и инициализирует все параметры рабочего состояния:
	 * - Сбрасывает текущий этап и его длительность
	 * - Обнуляет счетчики времени
	 * - Сбрасывает флаги управления водой
	 * - Вычисляет общую длительность работы
	 * - Обновляет индикацию прогресса
	 */
	void initializeWorkState();


	/**
	 * @brief Обработка состояния простоя (Idle)
	 * 
	 * Выполняет действия в состоянии простоя системы:
	 * - Управляет миганием желтого светодиода
	 * - Не выполняет активных операций с оборудованием
	 */
	void handleIdleState(float temperature);

	/**
	 * @brief Обработка состояния запуска (Start)
	 * 
	 * Выполняет проверки и инициализацию перед началом работы:
	 * - Проверяет исправность датчиков температуры
	 * - Проверяет закрытие дверцы
	 * - Инициализирует положение шибера
	 * - Подготавливает параметры текущего режима работы
	 * 
	 * @param temperature Текущая температура для проверок
	 * @return true если инициализация прошла успешно, false если требуется повторный вызов
	 */
	bool handleStartState(float temperature);

	/**
	 * @brief Обработка рабочего состояния (Work)
	 * 
	 * Основная функция управления процессом работы:
	 * - Обновление длительности этапа и общего времени
	 * - Контроль и коррекция температуры
	 * - Управление вентилятором и заслонкой
	 * - Управление подачей воды
	 * - Обновление индикации прогресса
	 * - Проверка предварительных действий перед завершением
	 * - Обработка завершения текущего этапа
	 * 
	 * @param temperature Текущая температура для контроля процесса
	 */
	void handleWorkState(float temperature);

	/**
	 * @brief Обработка состояний остановки и ошибки (Stop/Error)
	 * 
	 * Выполняет безопасное отключение оборудования:
	 * - Отключает вентиляторы
	 * - Отключает нагревательные элементы
	 * - Сбрасывает управление шибером
	 * - Отключает желтый светодиод
	 * - Деактивирует главное реле питания
	 */
	void handleStopAndErrorState();

	/**
	 * @brief Управление режимами работы вентилятора
	 * 
	 * Устанавливает соответствующий режим работы вентилятора
	 * в соответствии с настройками текущего этапа:
	 * - Выключенный режим
	 * - Низкая скорость (FanX1)
	 * - Высокая скорость (FanX2)
	 */
	void handleFanControl();

	/**
	 * @brief Управление положением заслонки (шибера)
	 * 
	 * Регулирует положение заслонки в соответствии с заданными
	 * параметрами текущего этапа. Запускает таймер управления
	 * шибером при необходимости изменения положения.
	 */
	void handleDamperControl();

	/**
	 * @brief Обработка предварительных действий перед завершением
	 * 
	 * Выполняет действия перед завершением процесса:
	 * - Воспроизведение звукового оповещения
	 * - Включение вытяжки для удаления дыма
	 * - Управление вытяжкой в зависимости от положения шибера
	 */
	void handlePreFinishActions();

	/**
	 * @brief Обработка завершения текущего этапа работы
	 * 
	 * Выполняет переход между этапами работы или завершение процесса:
	 * - Сброс флагов управления водой
	 * - Переход к следующему этапу
	 * - Обновление параметров этапа
	 * - Обработка завершения всего процесса
	 */

	void controlHoodVisor();
	void handleStageCompletion();
	void initGpio();
	void initDisplay();
	void initAdc();
	void initRtc();
	void handleGpioEvent(int pin, bool flag);
	void handleWaterSensorEvent(bool &flag);
	
	uint16_t m_ErrorCode{0};
	std::queue<uint16_t> listError;
	void showIconError(uint16_t codeError);

	bool isBreadmashineDone;
	bool isBreadmashineHot {false};
    int timeoutWokrHoodVisor{0};
};









