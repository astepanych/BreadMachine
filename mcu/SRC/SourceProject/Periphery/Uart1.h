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
 * @brief Драйвер для работы с UART1
 * 
 * Класс предоставляет функционал для асинхронной работы с UART1 
 * с использованием DMA и RTOS. Реализован как синглтон.
 */
class Uart1 {
public:
    /**
     * @brief Деструктор класса Uart1
     */
    ~Uart1() {};
    
    /**
     * @brief Получить экземпляр класса (синглтон)
     * @return Ссылка на единственный экземпляр Uart1
     */
    static Uart1 &instance() {
        static Uart1 m_instance;
        return m_instance;
    };
    
    /**
     * @brief Задача для записи данных в UART
     * @param p Указатель на параметры задачи (не используется)
     */
    void taskWrite(void *p);
    
    /**
     * @brief Задача для чтения данных из UART
     * @param p Указатель на параметры задачи (не используется)
     */
    void taskRead(void *p);
    
    /**
     * @brief Инициализация UART1
     * @details Выполняет:
     * - Настройку параметров UART
     * - Инициализацию DMA
     * - Создание задач и очередей RTOS
     */
    void init();
    
    /**
     * @brief Запись данных в UART
     * @param buf Указатель на буфер данных
     * @param len Длина данных для отправки
     */
    void write(uint8_t *buf, uint8_t len);
    
    /**
     * @brief Callback-функция для отправки байта
     * @details Пользовательская функция для обработки отправки каждого байта
     */
    std::function<void(const uint8_t)> putByte;
    
    /**
     * @brief Проверка занятости DMA
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
     * @details Вызывается после завершения передачи данных
     */
    void txEnd();

    /**
     * @brief Добавление байта в приемный буфер
     * @param byte Принятый байт
     */
    void pushByteRx(uint8_t byte);
    
private:
    /**
     * @brief Приватный конструктор (singleton)
     */
    Uart1();
    
    /**
     * @brief Инициализация параметров UART
     */
    void initUart();
    
    /**
     * @brief Инициализация DMA для UART
     */
    void initDma();
    
    /**
     * @brief Передача данных через DMA
     * @param data Указатель на данные
     * @param len Длина данных
     * @return Результат операции (0 - успешно)
     */
    int txDma(const uint8_t *data, const uint8_t len);

    BaseType_t xReturned; ///< Результат создания задач RTOS
    xTaskHandle xHandle; ///< Хэндл задачи записи
    xTaskHandle xHandleRead; ///< Хэндл задачи чтения
    xQueueHandle xQueueWrite; ///< Очередь для записи данных
    xSemaphoreHandle xSemWrite; ///< Семафор для синхронизации записи
    
    int indexWriteRx; ///< Индекс записи в приемный буфер
    int indexReadRx; ///< Индекс чтения из приемного буфера
    bool isRtosRun; ///< Флаг инициализации RTOS
    
    bool mBusyDma; ///< Флаг занятости DMA
    ElementUart push; ///< Элемент для передачи данных
    ElementUart pop; ///< Элемент для приема данных
};

