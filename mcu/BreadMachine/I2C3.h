#pragma once
#include "stm32f4xx_i2c.h"

#define EEPROM_MEMORY

/**
 * @class I2C3Interface
 * @brief ��������� ��� ������ � ����� I2C3
 * 
 * ��������� ������� Singleton ��� ����������� ������������� ����������.
 * ������������� ������� ������ ������/������ �� ���� I2C.
 */
class I2C3Interface {
public:
    /**
     * @brief ��������� ���������� ������ (Singleton)
     * @return ������ �� ������������ ��������� I2C3Interface
     */
    static I2C3Interface &instance();
    
    /**
     * @brief ������������� ���������� I2C3
     * @details ����������� ���������� ������ I2C3:
     * - �������� �������
     * - ������ ������
     * - ��������� GPIO
     */
    void init();
    
    /**
     * @brief ������ ������ �� ���� I2C
     * @param addr ����� ���������� �� ���� (7-������)
     * @param data ��������� �� ����� ������
     * @param len ����� ������ � ������
     * @note ����� ������������� ���������� ����� �� 1 ���
     */
    void write(uint16_t addr, uint8_t* data, uint8_t len);
    
    /**
     * @brief ������ ������ �� ���� I2C
     * @param addr ����� ���������� �� ���� (7-������)
     * @param data ��������� �� ����� ��� ������ ������
     * @param len ���������� ���� ��� ������
     * @warning ����� ������ ���� ������� �������
     */
    void read(uint16_t addr, uint8_t* data, uint8_t len);    
    
private:
    /**
     * @brief ��������� ����������� (Singleton)
     */
    I2C3Interface();
};

