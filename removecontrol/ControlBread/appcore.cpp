#include "appcore.h"
#include <qdebug.h>
#include <qhostaddress.h>
#include <qeventloop.h>
#include <qtimer.h>
#include "common_project.h"
#include <qthread.h>
#include <qthreadpool.h>
#include <QtConcurrent/QtConcurrent>
#include <qfile.h>

AppCore::AppCore(QObject *parent) : QObject(parent),
    m_stateConnectToHost(false)
{

    connect(&client, &QTcpSocket::errorOccurred, this,[=](QAbstractSocket::SocketError er){
        qDebug()<<er;
    });

    connect(&client, &QTcpSocket::stateChanged, this,[=](QAbstractSocket::SocketState state){
        qDebug()<<state;

    });
    timerWaitBootloader.setSingleShot(true);
    connect(&client, &QTcpSocket::readyRead, this, &AppCore::readData);
    connect(this, &AppCore::signalSendPacket, this, &AppCore::sendPacket, Qt::BlockingQueuedConnection);
    //глобальный пул потоко увеличим на 2, а то в Ubuntu не хватало места для создаваемых ниже потоков
    QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount() + 2);
    connect(&timerWaitBootloader, &QTimer::timeout, this, [=](){
        emit addStringLog("Нет события от загрузчика");
    });
    m_logModel = new ModelList();

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
        client.setSocketOption(QAbstractSocket::LowDelayOption, 1);
         client.setSocketOption(QAbstractSocket::KeepAliveOption, 1);
        setStateConnectToHost(true);
        countTx = 0;
        sendPacket(IdConnectExternSoft, nullptr, 0);
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

/*!
    \brief Рассчитывает контрольную сумму последовательности
    \param PayLoad поледоветельность для которой будет рассчитываться контрольная сумма
    \param iBitsNum длина последовательности
    \return посчитанная контрольная сумма
*/
uint16_t AppCore::CRC_Calc_s16_CCITT(uint16_t *PayLoad, uint16_t iBitsNum) {
    // Update the CRC for transmitted and received data using
    // the CCITT 16bit algorithm (X^16 + X^12 + X^5 + 1).
    uint16_t i;
    uint16_t crc = 0;

    for (i = 0; i < iBitsNum; i++) {
        crc = ((crc >> 8) & 0xFF) | (crc << 8); //перестановка полуслов
        crc ^= PayLoad[i];
        crc ^= ((crc & 0xff) >> 4);
        crc ^= (crc << 12);
        crc ^= ((crc & 0xff) << 5);
    }
    return crc;
}

void AppCore::testSendPackage()
{
    static int count = 0;
    if(stateConnectToHost() == false)
        return;
    for(int i = 0; i < 2; i++){
    PackageNetworkFormat p;
    memset(&p, 0, sizeof (PackageNetworkFormat));
    p.counter = count++;
    p.msgType = 0;
    p.cmdId = IdSoftVersion;
    p.dataSize = 16;
    memset(p.data, 0xa5, p.dataSize);
    p.crc = CRC_Calc_s16_CCITT((uint16_t*)&p, sizeof(PackageNetworkFormat)/sizeof(uint16_t)-1);

        client.write((const char*)&p, sizeof(PackageNetworkFormat));
        QThread::msleep(10);
    }

}

void AppCore::startFirmware(const QString &nameFirmware)
{
#ifdef Q_OS_ANDROID
       m_nameFirmware = nameFirmware;
#else
       QUrl url(nameFirmware);
       m_nameFirmware = url.path().remove(0,1);
#endif

       QFile file(m_nameFirmware);

       qDebug()<<file.exists();
       QFileInfo fileInfo(nameFirmware);
       qDebug()<<fileInfo.size();

    sendPacket(IdStartBootloader, nullptr, 0);


   emit addStringLog("Ожидаем событие от загрузчика");
   timerWaitBootloader.start(5000);
   //sendPacket(IdStartBootloader, nullptr, 0);
}

void AppCore::stopFirmware()
{
    isProccesFirmware = false;
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


void AppCore::startThreadFirmware()
{
    timerWaitBootloader.stop();
    isProccesFirmware = true;
     emit addStringLog("Получили событие от загрузчика");
    //создаем потоки
    QtConcurrent::run(QThreadPool::globalInstance(),this,&AppCore::taskFirmware, m_nameFirmware);
}

void AppCore::setLogModel(ModelList *newLogModel)
{
    if (m_logModel == newLogModel)
        return;
    m_logModel = newLogModel;
    emit logModelChanged();
}

void AppCore::resetLogModel()
{
    setLogModel({}); // TODO: Adapt to use your actual default value
}

ModelList *AppCore::logModel() const
{
    return m_logModel;
}

void AppCore::getLog()
{
    bufLog.clear();
    sendPacket(IdGetLog, nullptr, 0);
}

void AppCore::saveLog(const QString &nameFileSave)
{
    qDebug()<<nameFileSave;
    if(nameFileSave.length() == 0)
        return;
    QString nameFileSaveTemp;
#ifdef Q_OS_ANDROID
       nameFileSaveTemp = nameFileSave;
#else
       QUrl url(nameFileSave);
       nameFileSaveTemp = url.path().remove(0,1);
#endif




    QFile f(nameFileSaveTemp);
    if(f.open(QIODevice::WriteOnly)) {
        foreach(ElementModelList e, m_logModel->list()) {
            e.str += "\n";
            f.write((const char*)e.str.toLocal8Bit(), e.str.length());
        }
    }
    f.close();


}

void AppCore::clearLog()
{
    m_logModel->clearModel();
}

void AppCore::readData()
{
   QByteArray ba = client.readAll();
   qDebug()<<ba.length();
   qDebug()<<ba.toHex(',');
   while(ba.length() >= sizeof(PackageNetworkFormat)) {
       PackageNetworkFormat *p = (PackageNetworkFormat *)ba.data();
       switch(p->cmdId){
       case IdEndLog:{
            QString s(bufLog);

            QStringList lst = s.split('*');
            foreach( QString ss , lst){
                if(ss.at(0) == '~') {
                    ss.remove(0,1);
                    m_logModel->addItem(ss);
                }
            }
       }break;
       case IdDataLog:
           bufLog.append((const char *)p->data, p->dataSize);
           break;
       case IdFirmwareResult:
           stateCrc = p->data[0];
            qDebug()<<tr("rcv stateCrc %1").arg(stateCrc);
            emit signalRcvCrc();
           break;
       case IdDoneBootloader:
           startThreadFirmware();
           qDebug()<<"startThreadFirmware";
           break;
        default:
           break;
       }
       ba.remove(0, sizeof(PackageNetworkFormat));
   }

}

void AppCore::sendPacket(const uint16_t id, const uint8_t *data, const uint16_t len)
{
    PackageNetworkFormat p;
    memset(&p, 0, sizeof (PackageNetworkFormat));
    p.counter = countTx++;
    countTx %=0xff;
    p.msgType = 1;
    p.cmdId = id;
    qDebug()<<countTx;
    p.dataSize = len;
    if(len != 0)
        memcpy(p.data, data, p.dataSize);
    p.crc = CRC_Calc_s16_CCITT((uint16_t*)&p, sizeof(PackageNetworkFormat)/sizeof(uint16_t)-1);
    qDebug()<<client.write((const char*)&p, sizeof(PackageNetworkFormat));
}


void AppCore::taskFirmware(const QString &nameFirmware)
{
    const uint16_t sizeWorkBuf = 1024;
    const uint16_t attempsSend = 3;
    uint16_t crcCalc, lenSend;
    uint16_t index;
    uint16_t attemps;
    int commonIndex = 0;

    QByteArray ba;
    QFile file(nameFirmware);
    QFileInfo fileInfo(file);
    emit progressFirmware(0);
    if(file.open(QIODevice::ReadOnly)) {
        emit addStringLog("Файл прошивки открыт");
    } else {
        emit addStringLog("Не удалось открыть файл прошивки");
        return;
    }

    emit signalSendPacket(IdFirmwareStart, nullptr, 0);
    QThread::msleep(5000);

    while(true) {
        if(isProccesFirmware == false)
            break;
        attemps = 0;
        emit progressFirmware(commonIndex*100/fileInfo.size());
        ba = file.read(sizeWorkBuf);
        qDebug()<<ba.length();
        if(ba.length()%2)
            ba.append((char)0xff);
        crcCalc = CRC_Calc_s16_CCITT((uint16_t*)ba.data(), ba.length()/sizeof(uint16_t));

        for(attemps = 0;  attemps < attempsSend; attemps++) {
            index=0;

            while(index < ba.length()) {
                lenSend = ba.length() - index >= BODY_WORD_COUNT*sizeof(int16_t)? BODY_WORD_COUNT*sizeof(uint16_t) : ba.length() - index;
                emit signalSendPacket(IdFirmwareData, (uint8_t*)(ba.data()+index), lenSend);
                index+=BODY_WORD_COUNT*sizeof(int16_t);
                QThread::msleep(20);
            }
            emit addStringLog(tr("send block %1").arg(commonIndex));
            stateCrc = StateCrcNoRcv;
            emit signalSendPacket(IdFirmwareCrc, (uint8_t*)&crcCalc, sizeof(uint16_t));

            QEventLoop l;
            connect(this, &AppCore::signalRcvCrc, &l, &QEventLoop::quit);
            QTimer::singleShot(3000, &l, &QEventLoop::quit);
            l.exec();
            disconnect(this, &AppCore::signalRcvCrc, &l, &QEventLoop::quit);
            qDebug()<<tr("wait stateCrc %1").arg(stateCrc);

            //crc ok
            if(stateCrc == StateCrcOk){
                emit addStringLog(tr("crc ok block %1").arg(commonIndex));
                commonIndex+= sizeWorkBuf;
                break;
            }


        }
        //передали все данные
        if(commonIndex >= fileInfo.size())
            break;
        // не  получили ответ црц
        if(attemps == attempsSend) {
            emit addStringLog(tr("no sended firmware"));
            break;
        }
    }
    if(commonIndex >= fileInfo.size()) {
        emit signalSendPacket(IdResetMcu, nullptr, 0);
        emit progressFirmware(100);
        emit addStringLog(tr("firmware OK"));

    }
    file.close();
    qDebug()<<"end thread";
}






