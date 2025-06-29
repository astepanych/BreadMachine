#pragma once
#include "Widget.h"
#include "WorkMode.h"
#include <list>
#include <string>
#include <vector>
#include <functional>

struct ElementList
{
	uint16_t addrColor;
	uint16_t addrText;
};

/**
 * @class MyList
 * @brief Прокручиваемый список с выбором элементов
 * 
 * @extends Widget
 * 
 * Класс предоставляет функционал для:
 * - Добавления/удаления элементов
 * - Навигации (вверх/вниз)
 * - Привязки данных WorkMode
 * - Обновления отображения
 * - Управления положением прокрутки
 * 
 * @note Предназначен для встраиваемых систем с ограниченными ресурсами.
 */
class MyList : public Widget {
public:
    /**
     * @brief Конструктор
     * @param p Указатель на драйвер дисплея
     * @param maxNumItem Максимальное количество отображаемых элементов
     */
    MyList(DisplayDriver *p, uint16_t maxNumItem);

    /**
     * @brief Обработчик нажатий клавиш
     * @param key Код клавиши (например, стрелки вверх/вниз)
     * @return Widget* Следующий виджет в цепочке обработки
     */
    Widget* keyEvent(uint16_t key) override;

    /**
     * @brief Добавляет элемент в список
     * @param el Ссылка на элемент типа ElementList
     */
    void addItemHard(ElementList &el);

    /**
     * @brief Привязывает данные WorkMode
     * @param p Указатель на вектор с данными WorkMode
     */
    void setWModes(std::vector<WorkMode> *p) { pWModes = p; };

    /**
     * @brief Возвращает текст элемента
     * @param index Позиция элемента
     * @return std::string Текст элемента
     */
    std::string text(uint16_t index);

    /**
     * @brief Доступ к элементу по индексу
     * @param index Позиция элемента
     * @return ElementList Ссылка на элемент
     */
    ElementList at(int index);

    /**
     * @brief Возвращает количество элементов
     * @return int Размер списка
     */
    int length();

    /**
     * @brief Очищает список
     */
    void clear();

    /**
     * @brief Обновляет отображение списка
     * @details Должен быть реализован для кастомного рендеринга
     */
    void updateDisplay() override;

    /**
     * @brief Сбрасывает состояние списка
     */
    void resetWidget() override;

    /**
     * @brief Устанавливает адрес значения полосы прокрутки
     * @param addr Адрес в памяти для обновления значения
     */
    void setAddrScrollValue(uint16_t addr) { addrScrollValue = addr; };

    /**
     * @brief Колбэк при изменении выбора
     * @param index Новый выбранный индекс
     */
    std::function<void(int)> changeValue;

    /**
     * @brief Устанавливает текущий выбранный индекс
     * @param newIndex Индекс для выбора
     * @note Вызывает колбэк changeValue
     */
    inline void setIndex(uint16_t newIndex) {
        currentIndex = newIndex;
        changeValue(currentIndex);
    }

    /**
     * @brief Возвращает текущий выбранный индекс
     * @return uint16_t Активный индекс
     */
    inline uint16_t currentIndexValue() { return currentIndex; };

protected:
    std::vector<ElementList> lst; ///< Хранилище элементов
    std::vector<WorkMode> *pWModes; ///< Источник данных WorkMode
    uint16_t indexDisplay; ///< Индекс первого видимого элемента
    int16_t startIndex; ///< Смещение для отрисовки
    int16_t indexCommon; ///< Общий индекс для навигации
    uint16_t maxItemDisplay; ///< Макс. видимых элементов
    uint16_t maxPosItemDisplay; ///< Макс. позиция прокрутки
    uint16_t currentIndex; ///< Текущий выбранный индекс
    uint16_t currentPos; ///< Текущая позиция прокрутки
    uint16_t addrScrollValue; ///< Адрес значения прокрутки
    const uint16_t maxValueScroll = 84; ///< Максимальное значение прокрутки
};

