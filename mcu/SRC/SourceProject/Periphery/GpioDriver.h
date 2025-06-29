#pragma once
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "functional"

/**
 * @class GpioDriver
 * @brief ������� ��� ���������� GPIO-������
 * 
 * ����� ������������� ��������� ��� ���������� ��������� GPIO-������,
 * ������� ��������� ���������, ������������ � ���������� ������������.
 */
class GpioDriver {
public:
    /**
     * @enum PinsGpioOut
     * @brief ������������ �������� ����� GPIO
     */
    enum PinsGpioOut {
        PinFan = 0, ///< ��� ���������� ������������
        PinH2O, ///< ��� ���������� �����
        PinTemperatureUp, ///< ��� ���������� ������� ������������� ��������
        PinTemperatureDown, ///< ��� ���������� ������ ������������� ��������
        PinShiberX, ///< ��� ���������� ��������� X
        PinShiberO,
        ///< ��� ���������� ��������� O
        PinGreen, ///< ��� �������� ����������
        PinYellow, ///< ��� ������� ����������
        GlobalEnable, ///< ��� ����������� ����������
        PinX2, ///< �������������� ��� X2
        Led, ///< �������� ���������
        Led1                    ///< �������������� ��������� 1
    };

    /**
     * @enum StatesPin
     * @brief ��������� GPIO-�����
     */
    enum StatesPin {
        StatePinZero = Bit_RESET, ///< ������ ������� (���������� 0)
        StatePinOne                 ///< ������� ������� (���������� 1)
    };

    /**
     * @brief ����������� �������� GPIO
     */
    GpioDriver();

    /**
     * @brief ���������� �������� GPIO
     */
    ~GpioDriver();

    /**
     * @brief ������������� ������ GPIO
     */
    void initModule();

    /**
     * @brief ��������� ��������� ����
     * @param pin ����� ���� �� ������������ PinsGpioOut
     * @param state �������� ��������� ���� �� ������������ StatesPin
     */
    void setPin(PinsGpioOut pin, StatesPin state);

    /**
     * @brief ������������ ��������� ����
     * @param pin ����� ���� �� ������������ PinsGpioOut
     */
    void togglePin(PinsGpioOut pin);

    /**
     * @brief ��������� ������� ����������
     */
    void enableYellowLed();

    /**
     * @brief ���������� ������� ����������
     */
    void disableYellowLed();

    /**
     * @brief ��������� �������� ����������
     */
    void enableGreenLed();

    /**
     * @brief ���������� �������� ����������
     */
    void disableGreenLed();

    /**
     * @brief ���������� ����������
     */
    void disableInt();

    /**
     * @brief ��������� ����������
     */
    void enableInt();

    /**
     * @brief ��������� ���������� �������� (��������)
     * @return ��������� �� ��������� GpioDriver
     */
    static GpioDriver* instace() {return ins;};

    /**
     * @brief ������ ��� ������� �� �����
     * 
     * ������� ����� ������� ��� ������������� ������� �� ����
     * @param state ��������� ���� (true - ������� �������, false - ������)
     */
    std::function<void(bool)> pinEvent;

private:
    static GpioDriver *ins; ///< ��������� �� ��������� �������� (��������)
};


