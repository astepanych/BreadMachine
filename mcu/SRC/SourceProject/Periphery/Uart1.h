#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>
#include <Uart.h>
#include <functional>

#define uart2 Uart1::instance()

/**
 * @class Uart1
 * @brief ������� ��� ������ � UART1
 * 
 * ����� ������������� ���������� ��� ����������� ������ � UART1 
 * � �������������� DMA � RTOS. ���������� ��� ��������.
 */
class Uart1 {
public:
    /**
     * @brief ���������� ������ Uart1
     */
    ~Uart1() {};
    
    /**
     * @brief �������� ��������� ������ (��������)
     * @return ������ �� ������������ ��������� Uart1
     */
    static Uart1 &instance() {
        static Uart1 m_instance;
        return m_instance;
    };
    
    /**
     * @brief ������ ��� ������ ������ � UART
     * @param p ��������� �� ��������� ������ (�� ������������)
     */
    void taskWrite(void *p);
    
    /**
     * @brief ������ ��� ������ ������ �� UART
     * @param p ��������� �� ��������� ������ (�� ������������)
     */
    void taskRead(void *p);
    
    /**
     * @brief ������������� UART1
     * @details ���������:
     * - ��������� ���������� UART
     * - ������������� DMA
     * - �������� ����� � �������� RTOS
     */
    void init();
    
    /**
     * @brief ������ ������ � UART
     * @param buf ��������� �� ����� ������
     * @param len ����� ������ ��� ��������
     */
    void write(uint8_t *buf, uint8_t len);
    
    /**
     * @brief Callback-������� ��� �������� �����
     * @details ���������������� ������� ��� ��������� �������� ������� �����
     */
    std::function<void(const uint8_t)> putByte;
    
    /**
     * @brief �������� ��������� DMA
     * @return true ���� DMA ����� ��������� ������
     */
    bool isBusyDma() {
        return mBusyDma;
    };
    
    /**
     * @brief ��������� ����� ��������� DMA
     * @param f ��������� ����� (true - �����, false - ��������)
     */
    void setBusyDma(const bool f) { mBusyDma = f; };
    
    /**
     * @brief ���������� ���������� ��������
     * @details ���������� ����� ���������� �������� ������
     */
    void txEnd();

    /**
     * @brief ���������� ����� � �������� �����
     * @param byte �������� ����
     */
    void pushByteRx(uint8_t byte);
    
private:
    /**
     * @brief ��������� ����������� (singleton)
     */
    Uart1();
    
    /**
     * @brief ������������� ���������� UART
     */
    void initUart();
    
    /**
     * @brief ������������� DMA ��� UART
     */
    void initDma();
    
    /**
     * @brief �������� ������ ����� DMA
     * @param data ��������� �� ������
     * @param len ����� ������
     * @return ��������� �������� (0 - �������)
     */
    int txDma(const uint8_t *data, const uint8_t len);

    BaseType_t xReturned; ///< ��������� �������� ����� RTOS
    xTaskHandle xHandle; ///< ����� ������ ������
    xTaskHandle xHandleRead; ///< ����� ������ ������
    xQueueHandle xQueueWrite; ///< ������� ��� ������ ������
    xSemaphoreHandle xSemWrite; ///< ������� ��� ������������� ������
    
    int indexWriteRx; ///< ������ ������ � �������� �����
    int indexReadRx; ///< ������ ������ �� ��������� ������
    bool isRtosRun; ///< ���� ������������� RTOS
    
    bool mBusyDma; ///< ���� ��������� DMA
    ElementUart push; ///< ������� ��� �������� ������
    ElementUart pop; ///< ������� ��� ������ ������
};

