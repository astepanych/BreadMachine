/**
 * @file appcore.h
 * @brief Основной класс ядра приложения
 * @version 1.0
 * @date 2023
 * @author [Ваше имя/компания]
 *
 * @copyright Copyright (c) 2023
 *
 * @details Класс реализует основную бизнес-логику приложения,
 * включая работу с сетевым соединением, управление прошивкой,
 * работу с журналом событий и управление программами.
 */

#ifndef APPCORE_H
#define APPCORE_H

#include <QObject>
#include <QTcpSocket>
#include <QStringList>
#include <QTimer>
#include <modellogdatatransfer.h>
#include <programmsmodel.h>

/**
 * @class AppCore
 * @brief Основной класс ядра приложения
 *
 * Наследуется от QObject для использования сигналов/слотов Qt.
 * Обеспечивает:
 * - Сетевое взаимодействие через TCP
 * - Управление процессом прошивки
 * - Работу с журналом событий
 * - Управление программами и этапами
 */
class AppCore : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор класса
     * @param parent Указатель на родительский объект
     */
    explicit AppCore(QObject *parent = nullptr);

    /**
     * @brief Деструктор класса
     */
    ~AppCore();

    // Сетевые функции
    /**
     * @brief Подключение к серверу
     * @param portNum Номер порта для подключения
     */
    Q_INVOKABLE void connectToServer(quint16 portNum);

    /**
     * @brief Отключение от сервера
     */
    Q_INVOKABLE void disconnectFromHost();

    /**
     * @brief Тестовая отправка пакета
     */
    Q_INVOKABLE void testSendPackage();

    /**
     * @brief Получение состояния подключения
     * @return true если подключение активно
     */
    bool stateConnectToHost() const;

    /**
     * @brief Установка состояния подключения
     * @param newStateConnectToHost Новое состояние
     */
    void setStateConnectToHost(bool newStateConnectToHost);

    // Функции прошивки
    /**
     * @brief Запуск процесса прошивки
     * @param nameFirmware Имя файла прошивки
     */
    Q_INVOKABLE void startFirmware(const QString &nameFirmware);
    /**
     * @brief Выполнение процесса прошивки устройства
     * @param nameFirmware Имя файла прошивки для загрузки
     *
     * @details Метод выполняет следующие действия:
     * 1. Открывает файл прошивки и проверяет его валидность
     * 2. Инициализирует процесс обмена с загрузчиком устройства
     * 3. Последовательно передает блоки прошивки
     * 4. Контролирует процесс с помощью CRC проверок
     * 5. Обновляет прогресс бар через сигнал progressFirmware()
     *
     * @note Для работы требует предварительного установления соединения
     * @warning Прерывание процесса может привести к "зависанию" устройства
     *
     * @see startFirmware()
     * @see stopFirmware()
     */
    void taskFirmware(const QString &nameFirmware);

    /**
     * @brief Остановка процесса прошивки
     */
    Q_INVOKABLE void stopFirmware();

    /**
     * @brief Получение модели журнала
     * @return Указатель на модель журнала
     */
    Q_INVOKABLE ModelList *logModel() const;

    // Функции работы с журналом
    /**
     * @brief Получение журнала
     */
    Q_INVOKABLE void getLog();

    /**
     * @brief Сохранение журнала в файл
     * @param nameFileSave Имя файла для сохранения
     */
    Q_INVOKABLE void saveLog(const QString &nameFileSave);

    /**
     * @brief Очистка журнала
     */
    Q_INVOKABLE void clearLog();

    // Функции работы с программами
    /**
     * @brief Добавление программы
     */
    Q_INVOKABLE void addPrograms();

    /**
     * @brief Удаление программы
     * @param index Индекс программы для удаления
     */
    Q_INVOKABLE void removePrograms(int index);

    /**
     * @brief Чтение программ
     */
    Q_INVOKABLE void readProgramms();

    /**
     * @brief Запись программ
     */
    Q_INVOKABLE void writeProgramms();

    /**
     * @brief Переключение модели программ
     * @param index Индекс программы
     */
    Q_INVOKABLE void switchModelPrograms(const int index);

    /**
     * @brief Добавление этапа
     * @return true если этап успешно добавлен
     */
    Q_INVOKABLE bool addStage();

    /**
     * @brief Удаление этапа
     * @param index Индекс этапа
     */
    Q_INVOKABLE void removeStage(int index);

    /**
     * @brief Чтение программ из файла
     * @param fileName Имя файла
     */
    Q_INVOKABLE void readProgrammsFromFile(const QString &fileName);

    /**
     * @brief Сохранение программ в файл
     * @param fileName Имя файла
     */
    Q_INVOKABLE void saveProgrammsToFile(const QString &fileName);

    // Управление моделями
    /**
     * @brief Установка модели журнала
     * @param newLogModel Новая модель журнала
     */
    void setLogModel(ModelList *newLogModel);

    /**
     * @brief Сброс модели журнала
     */
    void resetLogModel();

    /**
     * @brief Получение прогресса
     * @return Значение прогресса (0-100)
     */
    int percentProgressbar() const;

    /**
     * @brief Установка прогресса
     * @param newPercentProgressbar Новое значение прогресса
     */
    void setPercentProgressbar(int newPercentProgressbar);

    /**
     * @brief Получение модели программ
     * @return Указатель на модель программ
     */
    Q_INVOKABLE ProgrammsModel *modelWM() const;

    /**
     * @brief Установка модели программ
     * @param newModelWM Новая модель программ
     */
    void setModelWM(ProgrammsModel *newModelWM);

    /**
     * @brief Получение параметров режима работы
     * @return Указатель на параметры
     */
    ParamsWorkMode *paramsWM() const;

    /**
     * @brief Установка параметров режима работы
     * @param newParamsWM Новые параметры
     */
    void setParamsWM(ParamsWorkMode *newParamsWM);

public slots:
    /**
     * @brief Чтение данных из сокета
     */
    void readData();

    /**
     * @brief Отправка пакета
     * @param id Идентификатор пакета
     * @param data Данные пакета
     * @param len Длина данных
     */
    void sendPacket(const uint16_t id, const uint8_t *data, const uint16_t len);

signals:
    /**
     * @brief Сигнал отправки пакета
     */
    void signalSendPacket(const uint16_t id, const uint8_t *data, const uint16_t len);

    /**
     * @brief Изменение состояния подключения
     */
    void stateConnectToHostChanged();

    /**
     * @brief Показать диалоговое окно
     * @param text Текст сообщения
     */
    void showDialog(const QString &text);

    /**
     * @brief Добавление элемента в модель
     */
    void addItemToModel(const QString &s);

    /**
     * @brief Добавление строки в журнал
     */
    void addStringLog(const QString &s);

    /**
     * @brief Прогресс прошивки
     */
    void progressFirmware(int val);

    /**
     * @brief Получение CRC
     */
    void signalRcvCrc();

    /**
     * @brief Изменение модели журнала
     */
    void logModelChanged();

    /**
     * @brief Изменение прогресса
     */
    void percentProgressbarChanged();

    /**
     * @brief Ожидание параметров
     */
    void waitedAnswerParams();

    /**
     * @brief Изменение модели программ
     */
    void modelWMChanged();

    /**
     * @brief Добавление программ
     */
    void addedProgramms();

private:
    QTcpSocket client;                      ///< TCP-клиент
    bool m_stateConnectToHost;              ///< Состояние подключения

    Q_PROPERTY(bool stateConnectToHost READ stateConnectToHost WRITE setStateConnectToHost NOTIFY stateConnectToHostChanged)

    /**
     * @brief Расчет CRC
     */
    uint16_t CRC_Calc_s16_CCITT(uint16_t *PayLoad, uint16_t iBitsNum);

    bool isProccesFirmware;                 ///< Флаг процесса прошивки
    uint16_t countTx;                       ///< Счетчик передач
    uint16_t stateCrc;                      ///< Состояние CRC
    uint16_t m_waitedAnswerId;              ///< Ожидаемый ID ответа
    bool f_waitAnswer{false};               ///< Флаг ожидания ответа
    QByteArray b_answer;                    ///< Буфер ответа
    QByteArray b_programms;                 ///< Буфер программ
    QString m_nameFirmware;                 ///< Имя прошивки
    QTimer timerWaitBootloader;             ///< Таймер ожидания загрузчика

    /**
     * @brief Запуск потока прошивки
     */
    void startThreadFirmware();

    ModelList *m_logModel;                  ///< Модель журнала
    Q_PROPERTY(ModelList *logModel READ logModel WRITE setLogModel RESET resetLogModel NOTIFY logModelChanged)
    QByteArray bufLog;                      ///< Буфер журнала
    int m_percentProgressbar;               ///< Прогресс (0-100)
    Q_PROPERTY(int percentProgressbar READ percentProgressbar WRITE setPercentProgressbar NOTIFY percentProgressbarChanged)

    /**
     * @brief Получение параметра
     */
    void getParam(const uint16_t id, QByteArray &ba);

    /**
     * @brief Получение 16-битного параметра
     */
    bool getParam16Bit(const uint16_t id, quint16 &param);

    ProgrammsModel *m_modelWM;              ///< Модель программ
    ParamsWorkMode *m_paramsWM;             ///< Параметры режима работы
};

#endif // APPCORE_H
