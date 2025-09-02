#pragma once
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "functional"

/**
 * @class GpioDriver
 * @brief Драйвер для управления GPIO-пинами
 * 
 * Класс предоставляет интерфейс для управления выходными GPIO-пинами,
 * включая установку состояния, переключение и управление светодиодами.
 */
class GpioDriver {
public:
    /**
     * @enum PinsGpioOut
     * @brief Перечисление выходных пинов GPIO
     */
    enum PinsGpioOut {
        PinFanLowSpeed = 0, ///< Пин управления вентилятором
        PinH2O, ///< Пин управления водой
        PinTemperatureUp, ///< Пин управления верхним температурным пределом
        PinTemperatureDown, ///< Пин управления нижним температурным пределом
        PinShiberX, ///< Пин управления заслонкой X
        PinShiberO, ///< Пин управления заслонкой O
        PinGreen, ///< Пин зеленого светодиода
        PinYellow, ///< Пин желтого светодиода
        GlobalEnable, ///< Пин глобального разрешения
        HoodVisor, ///< Вытяжка козырек
        MainHood,                   ///< вытяжка козырек
        EnableLightDoorLight,        ///< посдсветка двери
        PinFanFastSpeed, 
        PinLoadBread,       ///< 1        
        PinDownloadBread,   ///< 2
    };
    enum PinsGpioIn {
        InputPinWater,
        InputPinDamperState
    };

    /**
     * @enum StatesPin
     * @brief Состояния GPIO-пинов
     */
    enum StatesPin {
        StatePinZero = Bit_RESET, ///< Низкий уровень (логический 0)
        StatePinOne                 ///< Высокий уровень (логический 1)
    };

    /**
     * @brief Конструктор драйвера GPIO
     */
    GpioDriver();

    /**
     * @brief Деструктор драйвера GPIO
     */
    ~GpioDriver();

    /**
     * @brief Инициализация модуля GPIO
     */
    void initModule();

    /**
     * @brief Установка состояния пина
     * @param pin Номер пина из перечисления PinsGpioOut
     * @param state Желаемое состояние пина из перечисления StatesPin
     */
    void setPin(PinsGpioOut pin, StatesPin state);

    /**
     * @brief Переключение состояния пина
     * @param pin Номер пина из перечисления PinsGpioOut
     */
    void togglePin(PinsGpioOut pin);

    /**
     * @brief Включение желтого светодиода
     */
    void enableYellowLed();

    /**
     * @brief Выключение желтого светодиода
     */
    void disableYellowLed();

    /**
     * @brief Включение зеленого светодиода
     */
    void enableGreenLed();

    /**
     * @brief Выключение зеленого светодиода
     */
    void disableGreenLed();

    /**
     * @brief Отключение прерываний
     */
    void disableInt();

    /**
     * @brief Включение прерываний
     */
    void enableInt();
    /**
     * @brief Отключение прерываний
     */
    void disableIntDamperState();

    /**
     * @brief Включение прерываний
     */
    void enableIntDamperState();
    /**
     * @brief Получение экземпляра драйвера (синглтон)
     * @return Указатель на экземпляр GpioDriver
     */
    static GpioDriver* instace() {return ins;};

    /**
     * @brief Колбэк для событий на пинах
     * 
     * Функция будет вызвана при возникновении события на пине
     * @param state Состояние пина (true - высокий уровень, false - низкий)
     */
    std::function<void(int, bool)> pinEvent;



    bool isEventLoadBread();
    bool isEventDownloadBread();
    bool isEventSensorTempDrive1();
    bool isEventSensorTempDrive2();
    bool isEventSensorTempDrive3();
    bool isDoorClosed();
    bool isStartKey();
    bool isStopLoadKey();
    bool isDamperStateStart();

private:
    static GpioDriver *ins; ///< Указатель на экземпляр драйвера (синглтон)
};


