#include "appcore.h"
#include <qdebug.h>
#include <qhostaddress.h>
#include <qeventloop.h>
#include <qtimer.h>

AppCore::AppCore(QObject *parent) : QObject(parent),
    m_stateConnectToHost(false)
{
    connect(&client, &QTcpSocket::errorOccurred, this,[=](QAbstractSocket::SocketError er){
        qDebug()<<er;
    });

    connect(&client, &QTcpSocket::stateChanged, this,[=](QAbstractSocket::SocketState state){
        qDebug()<<state;
    });
}

AppCore::~AppCore()
{
   client.disconnectFromHost();
}


void AppCore::connectToServer(quint16 portNum)
{
    qDebug()<<__PRETTY_FUNCTION__;

    client.connectToHost(QHostAddress("192.168.4.1"), portNum);

    QEventLoop loop;
    connect(&client, &QTcpSocket::connected, &loop, &QEventLoop::quit);
    QTimer::singleShot(4000, &loop, &QEventLoop::quit);

    loop.exec();

    if(client.state() == QAbstractSocket::ConnectedState) {
        setStateConnectToHost(true);
    } else {
        emit showDialog(tr("Ошибка подключения к серверу"));
    }
}

void AppCore::disconnectFromHost()
{
    qDebug()<<__PRETTY_FUNCTION__;
    client.disconnectFromHost();
    setStateConnectToHost(false);
}

bool AppCore::stateConnectToHost() const
{
    return m_stateConnectToHost;
}

void AppCore::setStateConnectToHost(bool newStateConnectToHost)
{
    if (m_stateConnectToHost == newStateConnectToHost)
        return;
    m_stateConnectToHost = newStateConnectToHost;
    emit stateConnectToHostChanged();
}
