#pragma once
#include "stm32f4xx_i2c.h"

#define EEPROM_MEMORY

/**
 * @class I2C3Interface
 * @brief Интерфейс для работы с шиной I2C3
 * 
 * Реализует паттерн Singleton для обеспечения единственного экземпляра.
 * Предоставляет базовые методы чтения/записи по шине I2C.
 */
class I2C3Interface {
public:
    /**
     * @brief Получение экземпляра класса (Singleton)
     * @return Ссылка на единственный экземпляр I2C3Interface
     */
    static I2C3Interface &instance();
    
    /**
     * @brief Инициализация интерфейса I2C3
     * @details Настраивает аппаратный модуль I2C3:
     * - Тактовая частота
     * - Режимы работы
     * - Параметры GPIO
     */
    void init();
    
    /**
     * @brief Запись данных по шине I2C
     * @param addr Адрес устройства на шине (7-битный)
     * @param data Указатель на буфер данных
     * @param len Длина данных в байтах
     * @note Адрес автоматически сдвигается влево на 1 бит
     */
    void write(uint16_t addr, uint8_t* data, uint8_t len);
    
    /**
     * @brief Чтение данных по шине I2C
     * @param addr Адрес устройства на шине (7-битный)
     * @param data Указатель на буфер для приема данных
     * @param len Количество байт для чтения
     * @warning Буфер должен быть заранее выделен
     */
    void read(uint16_t addr, uint8_t* data, uint8_t len);    
    
private:
    /**
     * @brief Приватный конструктор (Singleton)
     */
    I2C3Interface();
};

