#ifndef APPCORE_H
#define APPCORE_H


#include <QObject>
#include <QTcpSocket>
#include <qstringlist.h>
#include <QTimer>
#include <modellogdatatransfer.h>

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


    void setLogModel(ModelList *newLogModel);
    void resetLogModel();

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

private:
    QTcpSocket client;
    bool m_stateConnectToHost;

    Q_PROPERTY(bool stateConnectToHost READ stateConnectToHost WRITE setStateConnectToHost NOTIFY stateConnectToHostChanged)
    uint16_t CRC_Calc_s16_CCITT(uint16_t *PayLoad, uint16_t iBitsNum);
    bool isProccesFirmware;
    uint16_t countTx;
    uint16_t stateCrc;
    QString m_nameFirmware;
    QTimer timerWaitBootloader;
    void startThreadFirmware();
    ModelList *m_logModel;
    Q_PROPERTY(ModelList *logModel READ logModel WRITE setLogModel RESET resetLogModel NOTIFY logModelChanged)
    QByteArray bufLog;
};

#endif // APPCORE_H
