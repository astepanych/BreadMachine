#ifndef APPCORE_H
#define APPCORE_H


#include <QObject>
#include <QTcpSocket>

class AppCore : public QObject
{
    Q_OBJECT
public:
    explicit AppCore(QObject *parent = nullptr);
    ~AppCore();
    Q_INVOKABLE void connectToServer(quint16 portNum);
    Q_INVOKABLE void disconnectFromHost();


    bool stateConnectToHost() const;
    void setStateConnectToHost(bool newStateConnectToHost);

signals:

    void stateConnectToHostChanged();
    void showDialog(const QString &text);
private:
    QTcpSocket client;
    bool m_stateConnectToHost;

    Q_PROPERTY(bool stateConnectToHost READ stateConnectToHost WRITE setStateConnectToHost NOTIFY stateConnectToHostChanged)
};

#endif // APPCORE_H
