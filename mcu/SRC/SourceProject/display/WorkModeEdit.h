#pragma once
#include "MyList.h"

/**
 * @file work_mode_edit.h
 * @brief Заголовочный файл для редактирования рабочих режимов
 * @details Содержит определения адресов и класс для работы с редактором рабочих режимов
 */

/**
 * @defgroup display_addresses Адреса дисплея
 * @brief Адреса областей дисплея для отображения и редактирования рабочих режимов
 * @{
 */
#define StartAddrListEditItemsVP 0x64a0  ///< Стартовый адрес списка режимов (просмотр)
#define StartAddrListEditItemsSP 0x6400  ///< Стартовый адрес списка режимов (редактирование)
#define OffsetColorsText 3               ///< Смещение для цветов текста
#define StepListEditVP 0x40              ///< Шаг между элементами в режиме просмотра
#define StepListEditSP 0x20              ///< Шаг между элементами в режиме редактирования
#define AddrScrolBar 0x65e0              ///< Адрес полосы прокрутки

/// @name Адреса полей в режиме просмотра
/// @{
#define AddrNameV  0x6620        ///< Адрес названия режима (просмотр)
#define AddrNumStageV  0x6660     ///< Адрес номера этапа (просмотр)
#define AddrTimeStageV  0x6670    ///< Адрес времени этапа (просмотр)
#define AddrTempStageV  0x6680    ///< Адрес температуры этапа (просмотр)
#define AddrWaterStageV  0x6682   ///< Адрес уровня воды этапа (просмотр)
#define AddrDamperStageV  0x6684  ///< Адрес положения заслонки этапа (просмотр)
#define AddrFanStageV  0x6686     ///< Адрес скорости вентилятора этапа (просмотр)
/// @}

/// @name Адреса полей в режиме редактирования
/// @{
#define AddrNameE  0x6720         ///< Адрес названия режима (редактирование)
#define AddrNumStageE  0x6760     ///< Адрес номера этапа (редактирование)
#define AddrTimeStageE  0x6770    ///< Адрес времени этапа (редактирование)
#define AddrTempStageE  0x6780    ///< Адрес температуры этапа (редактирование)
#define AddrWaterStageE  0x6782   ///< Адрес уровня воды этапа (редактирование)
#define AddrDamperStageE  0x6784  ///< Адрес положения заслонки этапа (редактирование)
#define AddrFanStageE  0x6786     ///< Адрес скорости вентилятора этапа (редактирование)
/// @}
/** @} */ // Конец группы display_addresses

/**
 * @enum StateEditWorkMode
 * @brief Состояния редактора рабочих режимов
 */
typedef enum 
{
    StateView = 0,  ///< Режим просмотра
    StateEdit,      ///< Режим редактирования
    StateNew        ///< Режим создания нового
} StateEditWorkMode;

/**
 * @class WorkModeEdit
 * @brief Класс для редактирования рабочих режимов
 * @extends MyList
 * 
 * Предоставляет функционал для просмотра, редактирования и создания
 * рабочих режимов с отображением на дисплее.
 */
class WorkModeEdit : public MyList
{
public:
    /**
     * @brief Конструктор редактора рабочих режимов
     * @param p Указатель на драйвер дисплея
     * @param maxNumItem Максимальное количество элементов
     */
    WorkModeEdit(DisplayDriver *p, uint16_t maxNumItem);
    
    /**
     * @brief Обработчик событий клавиш
     * @param key Код нажатой клавиши
     * @return Указатель на виджет для обработки события
     */
    Widget* keyEvent(uint16_t key);
    
    /**
     * @brief Изменение параметров режима
     * @param id Идентификатор параметра
     * @param len Длина данных
     * @param data Указатель на данные
     */
    void changeParams(const uint16_t id, uint8_t len, uint8_t* data);
    
    /**
     * @brief Callback-функция сохранения рабочих режимов
     * @details Вызывается при сохранении изменений в рабочих режимах
     */
    std::function<void()> saveWorkModes;
    
private:
    /**
     * @brief Отрисовка названия рабочего режима
     * @param addrItem Адрес отображения на дисплее
     */
    void paintNameWorkMode(uint16_t addrItem);
    
    /**
     * @brief Отрисовка параметров рабочего режима
     * @param isEdited Флаг режима редактирования
     */
    void paintSettingsWorkMode(bool isEdited = false);
    
    /**
     * @brief Вывод всех временных параметров режима
     */
    void printAllTimeMode();
    
    /**
     * @brief Конвертация в внутреннюю кодировку
     * @param buf Буфер данных
     * @param len Длина данных
     */
    void toMyCodec(uint8_t *buf, const uint16_t len);
    
    /**
     * @brief Конвертация в кодировку Windows-1251
     * @param buf Буфер данных
     * @param len Длина данных
     */
    void to1251(uint8_t *buf, const uint16_t len);
    
    WorkMode tempWMode;              ///< Временный рабочий режим для редактирования
    int16_t currentStage;            ///< Текущий этап редактирования
    StateEditWorkMode stateEdited;   ///< Текущее состояние редактора
};

