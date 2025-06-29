/**
 * @file programmsmodel.h
 * @brief Модели данных для работы с программами и параметрами режимов
 * @version 1.0
 * @date [дата]
 *
 * @copyright Copyright (c) [год] [Ваше имя/компания]
 */

#ifndef PROGRAMMSMODEL_H
#define PROGRAMMSMODEL_H

#include <QAbstractTableModel>
#include <../mcu/SRC/SourceProject/display/WorkMode.h>
#include <qlist.h>

// Константы для ролей данных
#define NAME_MODE "namemode"        ///< Роль имени режима
#define VALUE_ROLE "value"          ///< Роль значения параметра
#define TYPE_DELEGATE_ROLE "typeDelegate" ///< Роль типа делегата
#define TEMPERATURE_ROLE "temperature"    ///< Роль температуры
#define FAN_ROLE "fan"              ///< Роль вентилятора
#define DAMPER_ROLE "damper"        ///< Роль заслонки

/**
 * @class ProgrammsModel
 * @brief Модель списка программ (режимов работы)
 *
 * Наследуется от QAbstractListModel для отображения списка
 * программ/режимов работы в QML интерфейсе.
 */
class ProgrammsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * @enum UserRoles
     * @brief Пользовательские роли данных
     */
    enum UserRoles{
        NameMode = Qt::UserRole,  ///< Роль имени режима (аналог NAME_MODE)
    };

    /**
     * @brief Конструктор модели
     * @param parent Родительский объект
     */
    explicit ProgrammsModel(QObject *parent = nullptr);

    /**
     * @brief Количество строк в модели
     * @param parent Родительский индекс
     * @return Количество программ
     */
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Количество столбцов в модели
     * @param parent Родительский индекс
     * @return Всегда 1 (список)
     */
    Q_INVOKABLE int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Получение данных
     * @param index Индекс элемента
     * @param role Роль данных
     * @return Запрошенные данные
     */
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Имена ролей для QML
     * @return Хеш-таблица ролей
     */
    Q_INVOKABLE QHash<int,QByteArray> roleNames() const override;

    /**
     * @brief Добавление новой программы
     */
    Q_INVOKABLE void addProgramms();

    /**
     * @brief Удаление программы
     * @param index Индекс программы для удаления
     */
    Q_INVOKABLE void deleteProgramms(int index);

    /**
     * @brief Очистка списка программ
     */
    Q_INVOKABLE void clearProgramms();

    /**
     * @brief Установка данных
     * @param index Индекс элемента
     * @param value Новое значение
     * @param role Роль данных
     * @return true если данные успешно установлены
     */
    Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    /**
     * @brief Получение режима работы по индексу
     * @param index Индекс программы
     * @return Указатель на WorkMode или nullptr
     */
    WorkMode *workMode(const int index);

    /**
     * @brief Добавление существующего режима работы
     * @param mode Режим для добавления
     */
    void addProgramms(const WorkMode &mode);

private:
    QVector<WorkMode> m_listWorkMode;  ///< Список режимов работы
};

/**
 * @class ParamsWorkMode
 * @brief Модель параметров режима работы
 *
 * Наследуется от QAbstractTableModel для табличного отображения
 * параметров выбранного режима работы.
 */
class ParamsWorkMode : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @enum UserRoles
     * @brief Пользовательские роли данных
     */
    enum UserRoles{
        ValueRole = Qt::UserRole,      ///< Роль значения параметра
        TypeDelegateRole               ///< Роль типа делегата отображения
    };

    /**
     * @brief Конструктор модели
     * @param parent Родительский объект
     */
    explicit ParamsWorkMode(QObject *parent = nullptr);

    /**
     * @brief Количество строк в модели
     * @param parent Родительский индекс
     * @return Количество этапов в режиме
     */
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Количество столбцов в модели
     * @param parent Родительский индекс
     * @return Количество параметров (фиксированное)
     */
    Q_INVOKABLE int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Получение данных
     * @param index Индекс элемента
     * @param role Роль данных
     * @return Запрошенные данные
     */
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Имена ролей для QML
     * @return Хеш-таблица ролей
     */
    Q_INVOKABLE QHash<int,QByteArray> roleNames() const override;

    /**
     * @brief Установка данных
     * @param index Индекс элемента
     * @param value Новое значение
     * @param role Роль данных
     * @return true если данные успешно установлены
     */
    Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    /**
     * @brief Получение текущего режима работы
     * @return Указатель на WorkMode
     */
    WorkMode *workMode() const;

    /**
     * @brief Установка режима работы
     * @param newWorkMode Новый режим работы
     */
    void setWorkMode(WorkMode *newWorkMode);

    /**
     * @brief Данные заголовков
     * @param section Секция заголовка
     * @param orientation Ориентация
     * @param role Роль данных
     * @return Данные заголовка
     */
    Q_INVOKABLE QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * @brief Добавление этапа в режим
     * @return true если этап успешно добавлен
     */
    bool addStage();

    /**
     * @brief Удаление этапа из режима
     * @param index Индекс этапа
     */
    void removeStage(int index);

    /**
     * @brief Горизонтальный заголовок таблицы
     * @param colum Номер колонки
     * @return Название параметра
     */
    Q_INVOKABLE QVariant horizontalHeader(int colum);

private:
    WorkMode *m_workMode;  ///< Текущий режим работы
};

#endif // PROGRAMMSMODEL_H
