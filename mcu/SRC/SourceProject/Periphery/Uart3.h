#pragma once

#include <Uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "semphr.h"
#include <portmacro.h>
#include <functional>

#define uart3 Uart3::instance()

/**
 * @class Uart3
 * @brief ������� ��� ������ � UART3
 * 
 * ����� ��������� ����������� ����� ������� ����� UART3 
 * � �������������� DMA � RTOS. ������� Singleton.
 */
class Uart3 {
public:
    /**
     * @brief ���������� ������ Uart3
     */
    ~Uart3() {};
    
    /**
     * @brief �������� ��������� ������ (��������)
     * @return ������ �� ������������ ��������� Uart3
     */
    static Uart3 &instance() {
        static Uart3 m_instance;
        return m_instance;
    };
    
    /**
     * @brief ������ ������ ������ � UART3
     * @param p ��������� �� ��������� ������ (�� ������������)
     */
    void taskWrite(void *p);
    
    /**
     * @brief ������ ������ ������ �� UART3
     * @param p ��������� �� ��������� ������ (�� ������������)
     */
    void taskRead(void *p);
    
    /**
     * @brief ������������� UART3
     * @details ���������:
     * - ��������� ���������� UART
     * - ������������� DMA
     * - �������� ����� RTOS
     * - ������������� �������� � ���������
     */
    void init();
    
    /**
     * @brief ������ ������ � UART3
     * @param buf ��������� �� ����� ������
     * @param len ����� ������ ��� ��������
     * @note ������ ���������� � ������� �� ��������
     */
    void write(const uint8_t *buf, const uint8_t len);
    
    /**
     * @brief Callback-������� ��������� �������� �����
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
     * @details ���������� �� ���������� �������� DMA
     */
    void txEnd();

    /**
     * @brief ���������� ��������� ����� � �����
     * @param byte �������� ���� ������
     */
    void pushByteRx(uint8_t byte);
    
private:
    /**
     * @brief ��������� ����������� (singleton)
     */
    Uart3();
    
    /**
     * @brief ������������� ���������� UART3
     */
    void initUart();
    
    /**
     * @brief ������������� DMA ��� UART3
     */
    void initDma();
    
    /**
     * @brief �������� ������ ����� DMA
     * @param data ��������� �� ������
     * @param len ����� ������
     * @return ������ ���������� (0 - �������)
     */
    int txDma(const uint8_t *data, const uint8_t len);

    BaseType_t xReturned; ///< ��������� �������� ����� RTOS
    xTaskHandle xHandle; ///< ����� ������ ������
    xTaskHandle xHandleRead; ///< ����� ������ ������
    xQueueHandle xQueueWrite; ///< ������� �� ������ (FreeRTOS)
    xSemaphoreHandle xSemWrite; ///< ������� ������������� ������
    
    int indexWriteRx; ///< ������ ������ � �������� �����
    int indexReadRx; ///< ������ ������ �� ��������� ������  
    bool isRtosRun; ///< ���� ������������� RTOS
    
    bool mBusyDma; ///< ���� ��������� DMA
    ElementUart push; ///< ��������� ������������ ������
    ElementUart pop; ///< ��������� ����������� ������
};
