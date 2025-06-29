/**
 * @file modellogdatatransfer.h
 * @brief Модели данных для отображения логов и списков
 * @version 1.0
 * @date [дата]
 *
 * @copyright Copyright (c) [год] [Ваше имя/компания]
 */

#ifndef MODELLOGDATATRANSFER_H
#define MODELLOGDATATRANSFER_H

#include <QAbstractListModel>
#include <QList>
#include <QTime>

/**
 * @class ModelLogDataTransfer
 * @brief Модель табличного представления логов передачи данных
 *
 * Наследуется от QAbstractTableModel для отображения логов
 * в табличном виде с временными метками.
 */
class ModelLogDataTransfer : public QAbstractTableModel
{
public:
    /**
     * @enum Roles
     * @brief Роли данных для доступа из QML
     */
    enum Roles {
        TimeRole = Qt::UserRole + 1,  ///< Роль для времени записи
        TextRole,                     ///< Роль для текста сообщения
        RowNum                        ///< Роль для номера строки
    };

    /**
     * @brief Конструктор модели
     */
    ModelLogDataTransfer();

    /**
     * @brief Количество столбцов
     * @param parent Родительский индекс
     * @return Всегда 2 (время + сообщение)
     */
    int columnCount(const QModelIndex &parent) const override;

    /**
     * @brief Количество строк
     * @param parent Родительский индекс
     * @return Количество записей в логе
     */
    int rowCount(const QModelIndex &parent) const override;

    /**
     * @brief Получение данных
     * @param index Индекс данных
     * @param role Роль данных
     * @return Запрошенные данные
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Имена ролей для QML
     * @return Хеш-таблица ролей
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Очистка модели
     */
    Q_INVOKABLE void clearModel();

    /**
     * @brief Флаги элементов
     * @param index Индекс элемента
     * @return Флаги элемента
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

public slots:
    /**
     * @brief Добавление записи в лог
     * @param text Текст сообщения
     */
    void addItem(const QString &text);

private:
    QList<QString> m_list;  ///< Список строк лога
};

/**
 * @struct ElementModelList
 * @brief Элемент списка модели
 */
struct ElementModelList {
    QString str;         ///< Основной текст элемента
    QVariant otherParam; ///< Дополнительный параметр
};

/**
 * @class ModelList
 * @brief Модель списка с дополнительными параметрами
 *
 * Наследуется от QAbstractListModel для отображения
 * списка элементов с дополнительными параметрами.
 */
class ModelList : public QAbstractListModel
{
public:
    /**
     * @enum Roles
     * @brief Роли данных для доступа из QML
     */
    enum Roles {
        TextRole = Qt::UserRole + 1,  ///< Роль основного текста
        OtherParam                    ///< Роль дополнительного параметра
    };

    /**
     * @brief Конструктор модели
     */
    ModelList();

    /**
     * @brief Количество столбцов
     * @param parent Родительский индекс
     * @return Всегда 1 (список)
     */
    int columnCount(const QModelIndex &parent) const override;

    /**
     * @brief Количество строк
     * @param parent Родительский индекс
     * @return Количество элементов
     */
    int rowCount(const QModelIndex &parent) const override;

    /**
     * @brief Получение данных
     * @param index Индекс данных
     * @param role Роль данных
     * @return Запрошенные данные
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Имена ролей для QML
     * @return Хеш-таблица ролей
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Очистка модели
     */
    Q_INVOKABLE void clearModel();

    /**
     * @brief Добавление элемента
     * @param text Элемент для добавления
     */
    void addItem(const ElementModelList &text);

    /**
     * @brief Добавление элемента по строке
     * @param text Текст элемента
     */
    void addItem(const QString &text);

    /**
     * @brief Добавление списка элементов
     * @param text Список элементов
     */
    void addItems(const QList<ElementModelList> &text);

    /**
     * @brief Получение списка элементов
     * @return Константная ссылка на список
     */
    const QList<ElementModelList> &list() const;

    /**
     * @brief Установка нового списка
     * @param newList Новый список элементов
     */
    void setList(const QList<ElementModelList> &newList);

    /**
     * @brief Поиск индекса по значению
     * @param role Роль для поиска
     * @param value Значение для поиска
     * @return Индекс элемента или -1 если не найден
     */
    int index(int role, QVariant value);

private:
    QList<ElementModelList> m_list;  ///< Список элементов модели
};

#endif // MODELLOGDATATRANSFER_H
