#pragma once
#include <stdint.h>
#include <Uart3.h>
#include <Uart1.h>
#include <string>
#include "typedef.h"
#include "1251/utf8_to_cp1251.h"

/**
 * @file display_driver.h
 * @brief Заголовочный файл для работы с дисплеем
 * 
 * Содержит константы адресов, команды и класс для управления дисплеем.
 */

/** @defgroup display_consts Константы дисплея
 *  @{
 */

/// Адрес текущей страницы дисплея
constexpr uint16_t AddrCurrentPage = 0x0014;

/// Адрес оверлея дисплея
constexpr uint16_t AddrOverlay = 0x00e8;

/**
 * @brief Адрес версии ПО МКУ
 * @details Адрес в памяти дисплея, в которой хранится номер версии ПО МКУ для отображения
 */
constexpr uint16_t CmdSoftVersion = 0x2000;


constexpr uint16_t AddrRtc = 0x0010;
constexpr uint16_t AddrDamper = 0x1200;
constexpr uint16_t AddrFan = 0x1202;

constexpr uint16_t CmdDateTime = 0x2014;
constexpr uint16_t CmdSetDateTime = 0x009c;
constexpr uint16_t CmdNumProgramm = 0x4000;
constexpr uint16_t AddrScrollMainList = 0x4002;
constexpr uint16_t AddrProgressBar = 0x4004;
constexpr uint16_t AddrNumStage = 0x4006;
constexpr uint16_t AddrNumWater = 0x4008;
constexpr uint16_t AddrNumTemperature = 0x400a;
constexpr uint16_t AddrNumDamper = 0x400E;
constexpr uint16_t AddrNumFan = 0x4010;
constexpr uint16_t AddrNumTime = 0x4012;
constexpr uint16_t AddrNumTemperatureMeasure = 0x4020;
constexpr uint16_t AddrNumDurationNew = 0x4028;
constexpr uint16_t AddrMessage = 0x4040;
constexpr uint16_t AddrNumTimeMode = 0x4080;
constexpr uint16_t AddrNumTimeModeEdit = 0x40a0;

constexpr uint16_t AddrStages = 0x7000;
constexpr uint16_t CmdPaintFillRectangle = 0x0004;	
constexpr uint16_t addrMainItem = 0x2220;
constexpr uint16_t addrNameProg = 0x2280;

constexpr uint16_t addrCurrentSound = 0x2300;
constexpr uint16_t addrCurrentVolume = 0x2301;


constexpr uint16_t addrPassword = 0x6870;
constexpr uint16_t addrK1 = 0x6860;
constexpr uint16_t addrK2 = 0x6864;
constexpr uint16_t addrPeriod = 0x6868;
constexpr uint16_t addrAddWater = 0x686a;


constexpr uint16_t addrUpT = 0x6890;
constexpr uint16_t addrDownT = 0x6892;
constexpr uint16_t addrEnFan = 0x6894;
constexpr uint16_t addrWater = 0x6896;
constexpr uint16_t addrStrTempTest = 0x6f40;
constexpr uint16_t addrDamperOpen = 0x6898;
constexpr uint16_t addrDamperClose = 0x689a;
constexpr uint16_t addrGreenLed = 0x689C;
constexpr uint16_t addrYellowLed = 0x689E;
constexpr uint16_t addrMainRele = 0x689f;

constexpr uint16_t addrStateWifi = 0x68E0;
constexpr uint16_t addrStateWifiIcon = 0x68E2;
constexpr uint16_t addrStateWifiSSID = 0x68A0;
constexpr uint16_t addrStateWifiPassword = 0x68c0;

constexpr uint16_t CmdCofirm = 0x4f4b;

/**
 * @brief Структура прямоугольника для отрисовки
 * 
 * Используется для команд отрисовки на дисплее
 */
struct Rectangle {
    u16be beginX; ///< Начальная координата X
    u16be beginY; ///< Начальная координата Y
    u16be endX; ///< Конечная координата X
    u16be endY; ///< Конечная координата Y
    u16be color; ///< Цвет прямоугольника
};



/**
 * @class DisplayDriver
 * @brief Драйвер для работы с дисплеем
 * 
 * Класс предоставляет функционал для отправки команд и данных на дисплей,
 * обработки входящих данных и управления состоянием дисплея.
 */
class DisplayDriver {
    /**
     * @enum StateParcePacketDisplay
     * @brief Состояния парсинга пакетов от дисплея
     */
    enum StateParcePacketDisplay {
        StateWaitByte1, ///< Ожидание первого байта пакета
        StateWaitByte2, ///< Ожидание второго байта пакета
        StateWaitLen, ///< Ожидание байта длины пакета
        StateReadByte       ///< Чтение данных пакета
    };

public:
    /**
     * @brief Конструктор драйвера дисплея
     */
    DisplayDriver();
    
    /**
     * @brief Получить экземпляр драйвера (синглтон)
     * @return Указатель на экземпляр DisplayDriver
     */
    static DisplayDriver* instance() {return m_instance;};
    
    /**
     * @brief Инициализация дисплея
     */
    static void initDisplay();    
    
    /**
     * @brief Отправить байт на дисплей
     * @param byte Байт для отправки
     */
    static void putByte(const uint8_t byte); 
    
    /**
     * @brief Отправить данные на дисплей
     * @param id Идентификатор команды/адреса
     * @param len Длина данных
     * @param data Указатель на данные
     */
    static void sendToDisplay(uint16_t id, uint8_t len, uint8_t *data);
    
    /**
     * @brief Отправить строку на дисплей
     * @param id Идентификатор команды/адреса
     * @param str Строка для отправки
     */
    static void sendToDisplay(const uint16_t id, const std::string &str);
    
    /**
     * @brief Отправить строку на дисплей (C-style)
     * @param id Идентификатор команды/адреса
     * @param len Длина строки
     * @param data Указатель на строку
     */
    static void sendToDisplayStr(uint16_t id, uint8_t len, char *data);

    /**
     * @brief Отправить 16-битное значение на дисплей
     * @param id Идентификатор команды/адреса
     * @param data Значение для отправки
     */
    static void sendToDisplay(uint16_t id, uint16_t data);
    
    /**
     * @brief Получить данные с дисплея
     * @param id Идентификатор команды/адреса
     * @param data Буфер для данных
     * @param len Ожидаемая длина данных
     */
    static void getDataFromDisplay(uint16_t id, uint16_t data, uint8_t len);
    
    /**
     * @brief Отправить float значение на дисплей
     * @param id Идентификатор команды/адреса
     * @param data Значение для отправки
     */
    static void sendToDisplayF(uint16_t id, float &data);
    
    /**
     * @brief Переключить страницу дисплея
     * @param page Номер страницы
     */
    static void switchPage(uint16_t page);
    
    /**
     * @brief Показать сообщение на дисплее
     * @param idPage Идентификатор страницы
     * @param ipMessage Идентификатор сообщения
     */
    static void showMessage(uint16_t idPage, uint16_t ipMessage);
    
    /**
     * @brief Скрыть сообщение на дисплее
     */
    static void hideMessage();
    
    /**
     * @brief Колбэк для новых команд от дисплея
     * 
     * Функция будет вызвана при получении новой команды от дисплея
     */
    static std::function<void(const uint16_t, uint8_t, uint8_t*)> newCmd;
    
    /**
     * @brief Сброс дисплея
     */
    static void reset();
    
    /**
     * @brief Воспроизвести звук
     * @param numSound Номер звука
     * @param volume Громкость (0-255)
     */
    void playSound(uint8_t numSound, uint8_t volume);
    
private:
    static DisplayDriver *m_instance; ///< Указатель на экземпляр (синглтон)
    
    /// Начальный байт пакета 1
    static const uint8_t startByte1 = 0x5a;
    
    /// Начальный байт пакета 2
    static const uint8_t startByte2 = 0xa5;
    
    /// Байт команды записи
    static const uint8_t cmdByteWrite = 0x82;
    
    /// Байт команды чтения
    static const uint8_t cmdByteRead = 0x83;
    
    static uint8_t bufParce[SizeBuffer]; ///< Буфер для парсинга пакетов
    static uint8_t lenPacket; ///< Длина текущего пакета
    static uint8_t currentIndex; ///< Текущий индекс в буфере
    
    /**
     * @brief Задача для работы с дисплеем
     * @param p Параметры задачи (не используется)
     */
    static void taskDisplay(void *p);
    
    /**
     * @brief Парсинг пакета от дисплея
     * @param len Длина пакета
     * @param data Данные пакета
     */
    static void parsePackFromDisplay(uint8_t len, uint8_t *data);
        
    static int cntPutByte; ///< Счетчик отправленных байт
    static xQueueHandle xQueueDisplay; ///< Очередь для дисплея
    static xSemaphoreHandle xSemDisplay; ///< Семафор для дисплея
    TaskHandle_t xHandleDisplay = NULL; ///< Хэндл задачи дисплея
    static StateParcePacketDisplay statePacket; ///< Текущее состояние парсера
};

