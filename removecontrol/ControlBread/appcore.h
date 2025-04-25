#ifndef APPCORE_H
#define APPCORE_H


#include <QObject>
#include <QTcpSocket>
#include <qstringlist.h>
#include <QTimer>
#include <modellogdatatransfer.h>
#include <programmsmodel.h>

class AppCore : public QObject
{


    Q_OBJECT
public:
    explicit AppCore(QObject *parent = nullptr);
    ~AppCore();
    Q_INVOKABLE void connectToServer(quint16 portNum);
    Q_INVOKABLE void disconnectFromHost();
    Q_INVOKABLE void testSendPackage();

    bool stateConnectToHost() const;
    void setStateConnectToHost(bool newStateConnectToHost);


    Q_INVOKABLE void startFirmware(const QString &nameFirmware);
    void taskFirmware(const QString &nameFirmware);
    Q_INVOKABLE void stopFirmware();
    Q_INVOKABLE ModelList *logModel() const;

    Q_INVOKABLE void getLog();
    Q_INVOKABLE void saveLog(const QString &nameFileSave);
    Q_INVOKABLE void clearLog();
    Q_INVOKABLE void addPrograms();
    Q_INVOKABLE void removePrograms(int index);

    Q_INVOKABLE void readProgramms();
    Q_INVOKABLE void switchModelPrograms(const int index);

    Q_INVOKABLE bool addStage();
    Q_INVOKABLE void removeStage(int index);


    void setLogModel(ModelList *newLogModel);
    void resetLogModel();

    int percentProgressbar() const;
    void setPercentProgressbar(int newPercentProgressbar);

    Q_INVOKABLE ProgrammsModel *modelWM() const;


    void setModelWM(ProgrammsModel *newModelWM);

    ParamsWorkMode *paramsWM() const;
    void setParamsWM(ParamsWorkMode *newParamsWM);

public slots:
    void readData();
    void sendPacket(const uint16_t id, const uint8_t *data, const uint16_t len);
signals:
    void signalSendPacket(const uint16_t id, const uint8_t *data, const uint16_t len);
    void stateConnectToHostChanged();
    void showDialog(const QString &text);
    void addItemToModel(const QString &s);
    void addStringLog(const QString &s);
    void progressFirmware(int val);
    void signalRcvCrc();

    void logModelChanged();

    void percentProgressbarChanged();

    void waitedAnswerParams();

    void modelWMChanged();

    void addedProgramms();

private:
    QTcpSocket client;
    bool m_stateConnectToHost;

    Q_PROPERTY(bool stateConnectToHost READ stateConnectToHost WRITE setStateConnectToHost NOTIFY stateConnectToHostChanged)
    uint16_t CRC_Calc_s16_CCITT(uint16_t *PayLoad, uint16_t iBitsNum);
    bool isProccesFirmware;
    uint16_t countTx;
    uint16_t stateCrc;
    uint16_t m_waitedAnswerId;
    bool f_waitAnswer{false};
    QByteArray b_answer;
    QByteArray b_programms;
    QString m_nameFirmware;
    QTimer timerWaitBootloader;
    void startThreadFirmware();
    ModelList *m_logModel;
    Q_PROPERTY(ModelList *logModel READ logModel WRITE setLogModel RESET resetLogModel NOTIFY logModelChanged)
    QByteArray bufLog;
    int m_percentProgressbar;
    Q_PROPERTY(int percentProgressbar READ percentProgressbar WRITE setPercentProgressbar NOTIFY percentProgressbarChanged)
    void getParam(const uint16_t id, QByteArray &ba);
    bool getParam16Bit(const uint16_t id, quint16 &param);

    ProgrammsModel *m_modelWM;
    ParamsWorkMode *m_paramsWM;



};

#endif // APPCORE_H
