#pragma once

#include <functional>

/**
 * @file timer3.h
 * @brief Драйвер для работы с таймером 3
 * @details Содержит настройки и функции управления аппаратным таймером 3.
 */

/**
 * @def TimeoutTim3
 * @brief Таймаут по умолчанию для таймера 3
 * @note Значение 999 соответствует 1 секунде (при частоте таймера 1 кГц)
 */
#define TimeoutTim3 (999)

/**
 * @class Timer3
 * @brief Класс для управления таймером 3
 * 
 * Реализует функционал инициализации, запуска и остановки таймера 3,
 * а также обработку события таймаута через callback-функцию.
 */
class Timer3 {
public:
    /**
     * @brief Конструктор класса Timer3
     */
    Timer3();
    
    /**
     * @brief Деструктор класса Timer3
     */
    ~Timer3();
    
    /**
     * @brief Получить экземпляр класса (синглтон)
     * @return Указатель на единственный экземпляр Timer3
     */
    static Timer3* instance() {return m_instance;};
    
    /**
     * @brief Инициализация таймера 3
     * @details Настраивает аппаратный таймер 3 с параметрами:
     * - Предделитель: 0
     * - Период: 999 (1 сек при 1 кГц)
     * - Режим: вверх
     */
    static void init();
    
    /**
     * @brief Запуск таймера 3
     */
    static void start();
    
    /**
     * @brief Остановка таймера 3
     */
    static void stop();
    
    /**
     * @brief Callback-функция обработки таймаута
     * @details Пользовательская функция, вызываемая по истечении таймаута таймера.
     * 
     * Пример использования:
     * @code
     * Timer3::handlerTimeout = []() {
     *     // Действия по таймауту
     * };
     * @endcode
     */
    static std::function<void()> handlerTimeout;
        
private:
    static Timer3 *m_instance; ///< Указатель на единственный экземпляр класса
    
    /**
     * @brief Время таймаута таймера (в тиках)
     * @note Значение по умолчанию 999 (1 сек при 1 кГц)
     */
    static const int m_timeout = 999;
};
