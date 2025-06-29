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
 * @brief Драйвер для работы с UART3
 * 
 * Класс реализует асинхронный обмен данными через UART3 
 * с использованием DMA и RTOS. Паттерн Singleton.
 */
class Uart3 {
public:
    /**
     * @brief Деструктор класса Uart3
     */
    ~Uart3() {};
    
    /**
     * @brief Получить экземпляр класса (синглтон)
     * @return Ссылка на единственный экземпляр Uart3
     */
    static Uart3 &instance() {
        static Uart3 m_instance;
        return m_instance;
    };
    
    /**
     * @brief Задача записи данных в UART3
     * @param p Указатель на параметры задачи (не используется)
     */
    void taskWrite(void *p);
    
    /**
     * @brief Задача чтения данных из UART3
     * @param p Указатель на параметры задачи (не используется)
     */
    void taskRead(void *p);
    
    /**
     * @brief Инициализация UART3
     * @details Выполняет:
     * - Настройку параметров UART
     * - Инициализацию DMA
     * - Создание задач RTOS
     * - Инициализацию очередей и семафоров
     */
    void init();
    
    /**
     * @brief Запись данных в UART3
     * @param buf Указатель на буфер данных
     * @param len Длина данных для отправки
     * @note Данные помещаются в очередь на отправку
     */
    void write(const uint8_t *buf, const uint8_t len);
    
    /**
     * @brief Callback-функция обработки отправки байта
     */
    std::function<void(const uint8_t)> putByte;
    
    /**
     * @brief Проверка состояния DMA
     * @return true если DMA занят передачей данных
     */
    bool isBusyDma() {
        return mBusyDma;
    };
    
    /**
     * @brief Установка флага занятости DMA
     * @param f Состояние флага (true - занят, false - свободен)
     */
    void setBusyDma(const bool f) { mBusyDma = f; };
    
    /**
     * @brief Обработчик завершения передачи
     * @details Вызывается по завершении передачи DMA
     */
    void txEnd();

    /**
     * @brief Добавление принятого байта в буфер
     * @param byte Принятый байт данных
     */
    void pushByteRx(uint8_t byte);
    
private:
    /**
     * @brief Приватный конструктор (singleton)
     */
    Uart3();
    
    /**
     * @brief Инициализация параметров UART3
     */
    void initUart();
    
    /**
     * @brief Инициализация DMA для UART3
     */
    void initDma();
    
    /**
     * @brief Передача данных через DMA
     * @param data Указатель на данные
     * @param len Длина данных
     * @return Статус выполнения (0 - успешно)
     */
    int txDma(const uint8_t *data, const uint8_t len);

    BaseType_t xReturned; ///< Результат создания задач RTOS
    xTaskHandle xHandle; ///< Хэндл задачи записи
    xTaskHandle xHandleRead; ///< Хэндл задачи чтения
    xQueueHandle xQueueWrite; ///< Очередь на запись (FreeRTOS)
    xSemaphoreHandle xSemWrite; ///< Семафор синхронизации записи
    
    int indexWriteRx; ///< Индекс записи в приемный буфер
    int indexReadRx; ///< Индекс чтения из приемного буфера  
    bool isRtosRun; ///< Флаг инициализации RTOS
    
    bool mBusyDma; ///< Флаг занятости DMA
    ElementUart push; ///< Структура передаваемых данных
    ElementUart pop; ///< Структура принимаемых данных
};
