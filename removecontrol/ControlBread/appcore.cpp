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
#include <qxmlstream.h>


AppCore::AppCore(QObject *parent) : QObject(parent),
    m_stateConnectToHost(false)
{

    connect(&client, &QTcpSocket::errorOccurred, this,[=](QAbstractSocket::SocketError er){
        qDebug()<<er;
    });

    connect(&client, &QTcpSocket::stateChanged, this,[=](QAbstractSocket::SocketState state){
        qDebug()<<state;
        if(state == QAbstractSocket::ClosingState) {
            setStateConnectToHost(false);
        }

    });
    m_modelWM = new ProgrammsModel(this);
    m_modelWM->addProgramms();
    m_modelWM->addProgramms();
    m_modelWM->addProgramms();
    m_modelWM->addProgramms();

    m_paramsWM = new ParamsWorkMode(this);
    m_paramsWM->setWorkMode(m_modelWM->workMode(0));


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

int AppCore::percentProgressbar() const
{
    return m_percentProgressbar;
}

void AppCore::setPercentProgressbar(int newPercentProgressbar)
{
    if (m_percentProgressbar == newPercentProgressbar)
        return;
    m_percentProgressbar = newPercentProgressbar;
    emit percentProgressbarChanged();
}

ProgrammsModel *AppCore::modelWM() const
{
    return m_modelWM;
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

void AppCore::addPrograms() {
    m_modelWM->addProgramms();
    switchModelPrograms(m_modelWM->rowCount()-1);
    emit addedProgramms();
}

void AppCore::removePrograms(int index) {
    if(m_modelWM->rowCount() != 0 && (index >=0 || index < m_modelWM->rowCount())) {
        m_modelWM->deleteProgramms(index);
        switchModelPrograms(index-1);
    }
}

void AppCore::readProgramms() {
    uint16_t numPrograms;
    getParam16Bit(IdNumPrograms, numPrograms);
    qDebug()<<numPrograms;
    m_modelWM->clearProgramms();
    for(uint16_t i = 0 ; i < numPrograms; i++) {
        stateCrc = StateCrcNoRcv;
        QEventLoop loop ;
        connect(this, &AppCore::signalRcvCrc, &loop, &QEventLoop::quit);
        sendPacket(IdReadPrograms,(uint8_t*)&i,sizeof(uint16_t));
        QTimer::singleShot(2000, &loop, &QEventLoop::quit);
        loop.exec();
        connect(this, &AppCore::signalRcvCrc, &loop, &QEventLoop::quit);
        if(stateCrc == StateCrcOk) {
            WorkMode w;
            memcpy(&w, b_programms.data(), sizeof(w));
            m_modelWM->addProgramms(w);
            qDebug()<<"crc ok";

        }

        setPercentProgressbar((i+1)*100/numPrograms);
    }
}

void AppCore::writeProgramms()
{
    QEventLoop loop ;
    uint16_t numPrograms = m_modelWM->rowCount(QModelIndex());
    sendPacket(IdNumPrograms, (uint8_t*)&numPrograms, sizeof(uint16_t));
    for(uint16_t i = 0 ; i < numPrograms; i++) {
        stateCrc = StateCrcNoRcv;
        sendPacket(IdWritePrograms,(uint8_t*)&i,sizeof(uint16_t));
        b_programms.clear();
        b_programms.append((const char*)m_modelWM->workMode(i), sizeof(WorkMode));

        uint8_t *data = (uint8_t*)m_modelWM->workMode(i);
        for(int x = 0; x<sizeof(WorkMode); x+=BODY_BYTE_COUNT) {
            sendPacket(IdDataPrograms, data+x, ((sizeof(WorkMode) - x) > BODY_BYTE_COUNT)?BODY_BYTE_COUNT:sizeof(WorkMode) - x);
        }
        QTimer::singleShot(2000, &loop, &QEventLoop::quit);
        sendPacket(IdCrcPrograms, nullptr, 0);
        loop.exec();
        connect(this, &AppCore::signalRcvCrc, &loop, &QEventLoop::quit);
        if(stateCrc == StateCrcOk) {
            qDebug()<<"crc ok";
        } else {
            return;
        }
        setPercentProgressbar((i+1)*100/numPrograms);
    }
    sendPacket(IdStartCopyPrograms, nullptr, 0);
    qDebug()<<numPrograms;
}

void AppCore::switchModelPrograms(const int index)
{
    qDebug()<<"index = "<<index;
    if(m_modelWM->rowCount() != 0){
        if(index >=0 && index < m_modelWM->rowCount()){
            m_paramsWM->setWorkMode(m_modelWM->workMode(index));
        } else if(index < 0){
            m_paramsWM->setWorkMode(m_modelWM->workMode(0));
        }
    }
    else
        m_paramsWM->setWorkMode(nullptr);
}

bool AppCore::addStage()
{
    return m_paramsWM->addStage();
}

void AppCore::removeStage(int index)
{
    m_paramsWM->removeStage(index);
}

void AppCore::readProgrammsFromFile(const QString &fileName)
{
    qDebug()<<fileName;
    QString s;
#ifdef Q_OS_ANDROID
       s = fileName;
#else
       QUrl url(fileName);
       s = url.path().remove(0,1);
#endif
    QFile f(s);
    if(f.open(QIODevice::ReadOnly) == false) {
        return;
    }
    QXmlStreamReader xml(&f);
    WorkMode mode;
    int indexStage = 0;
    m_modelWM->clearProgramms();
    QString val;
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        qDebug() << xml.tokenString();
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "Workmode") {
                indexStage = 0;
                memset(&mode, 0, sizeof(mode));
            } else if (xml.name() == "name") {
                  val = xml.readElementText();
                  QString ba = QTextCodec::codecForName("utf-8")->toUnicode(val.toUtf8());

                  QByteArray s  = QTextCodec::codecForName("Windows-1251")->fromUnicode(ba);
                  mode.lenNameMode = s.length();
                  memset(mode.nameMode,0, MaxLengthNameMode);
                  memcpy(mode.nameMode, s.data(), s.length());
            } else if (xml.name() == "num_stages") {


                mode.numStage = xml.readElementText().toInt();
            } else if (xml.name() == "dur") {
                 val = xml.readElementText();
                 mode.stages[indexStage].duration = val.toInt();
            } else if (xml.name() == "water") {
                val = xml.readElementText();
                mode.stages[indexStage].waterVolume = val.toInt();
            } else if (xml.name() == "temp") {
                val = xml.readElementText();
                mode.stages[indexStage].temperature = val.toInt();
            } else if (xml.name() == "fan") {
                val = xml.readElementText();
                mode.stages[indexStage].fan = (val == "true") ? 1 : 0;
            } else if (xml.name() == "damper") {
                val = xml.readElementText();
                mode.stages[indexStage].damper = (val == "true") ? 1 : 0;
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (xml.name() == "Workmode") {
                qDebug()<<xml.name();
                m_modelWM->addProgramms(mode);
            } else if (xml.name() == "stage") {
                 qDebug()<<xml.name();
                 indexStage++;
            }
        }

        if (xml.hasError()) {
            qDebug() << "XML error:" << xml.error();
        }

    }

    /*
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    for(int i = 0; i< m_modelWM->rowCount(QModelIndex()); i++) {
        stream.writeStartElement("Programm");
        stream.writeAttribute("name", m_modelWM->data(m_modelWM->index(i),ProgrammsModel::NameMode).toString());
        stream.writeTextElement("num stages", QString("%1").arg(m_modelWM->workMode(i)->numStage));
        for(int j = 0; j< m_modelWM->workMode(i)->numStage; j++) {
            stream.writeTextElement("stage", QString("%1").arg(j+1));
            stream.writeAttribute("dur", QString("%1").arg(m_modelWM->workMode(i)->stages[j].duration));
            stream.writeAttribute("temp", QString("%1").arg(m_modelWM->workMode(i)->stages[j].temperature));
            stream.writeAttribute("water", QString("%1").arg(m_modelWM->workMode(i)->stages[j].waterVolume));
            stream.writeAttribute("fan", QString("%1").arg(m_modelWM->workMode(i)->stages[j].fan?"true":"false"));
            stream.writeAttribute("damper", QString("%1").arg(m_modelWM->workMode(i)->stages[j].damper?"true":"false"));
        }
        stream.writeEndElement(); // Programms
    }
*/

}

void AppCore::saveProgrammsToFile(const QString &fileName)
{
    qDebug()<<fileName;
    QString s;
#ifdef Q_OS_ANDROID
       s = fileName;
#else
       QUrl url(fileName);
       s = url.path().remove(0,1);
#endif
    QFile f(s);
    if(f.open(QIODevice::WriteOnly) == false) {
        return;
    }
    QXmlStreamWriter stream(&f);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("Programms");
    for(int i = 0; i< m_modelWM->rowCount(QModelIndex()); i++) {
        stream.writeStartElement("Workmode");
        stream.writeTextElement("name", m_modelWM->data(m_modelWM->index(i),ProgrammsModel::NameMode).toString());
        stream.writeTextElement("num_stages", QString("%1").arg(m_modelWM->workMode(i)->numStage));
        for(int j = 0; j< m_modelWM->workMode(i)->numStage; j++) {
            stream.writeStartElement("stage");
            stream.writeAttribute("num ", QString("%1").arg(j+1));
            stream.writeTextElement("dur", QString("%1").arg(m_modelWM->workMode(i)->stages[j].duration));
            stream.writeTextElement("temp", QString("%1").arg(m_modelWM->workMode(i)->stages[j].temperature));
            stream.writeTextElement("water", QString("%1").arg(m_modelWM->workMode(i)->stages[j].waterVolume));
            stream.writeTextElement("fan", QString("%1").arg(m_modelWM->workMode(i)->stages[j].fan?"true":"false"));
            stream.writeTextElement("damper", QString("%1").arg(m_modelWM->workMode(i)->stages[j].damper?"true":"false"));
            stream.writeEndElement(); // stage
        }
        stream.writeEndElement(); // Programm
    }
    stream.writeEndElement(); // Programms

   /* stream.writeStartElement("bookmark");
    stream.writeAttribute("href", "http://qt-project.org/");
    stream.writeTextElement("title", "Qt Project");
    stream.writeEndElement(); // bookmark

    stream.writeStartElement("bookmark");
    stream.writeAttribute("href", "http://qt-project.org/");
    stream.writeTextElement("title", "Qt Project");
    stream.writeEndElement(); // bookmark
*/



    stream.writeEndDocument();


    f.close();
}

void AppCore::readData()
{
   QByteArray ba = client.readAll();
   qDebug()<<ba.length();
   qDebug()<<ba.toHex(',');
   while(ba.length() >= sizeof(PackageNetworkFormat)) {
       PackageNetworkFormat *p = (PackageNetworkFormat *)ba.data();
       if(f_waitAnswer) {
           if(m_waitedAnswerId == p->cmdId) {
               f_waitAnswer = false;
               b_answer.clear();
               b_answer.append((const char*)p->data, p->dataSize);
               emit waitedAnswerParams();
               continue;
           }
       }
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
       case IdStartWrite:
           qDebug()<<"IdStartWrite";
           emit addStringLog(tr("Начата запись в ПЗУ"));
           break;
       case IdStartEnd:
          emit addStringLog(tr("Запись в ПЗУ завершена"));
           break;
       case IdStartCopyPrograms:
           b_programms.clear();
           break;
       case IdDataPrograms:
           b_programms.append((const char*)p->data, p->dataSize);
           break;
        case IdCrcPrograms:
       {
           uint32_t crcRcv;
            uint32_t crccalc = CRC_Calc_s16_CCITT((uint16_t*)b_programms.data(), b_programms.length()/sizeof (uint16_t));
           memcpy(&crcRcv, p->data, sizeof(uint32_t));
           if(crcRcv == crccalc){
               stateCrc = StateCrcOk;
               emit signalRcvCrc();
           }
       }
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
    p.msgType = MessageTypeSet;
    p.cmdId = id;
    p.dataSize = len;
    if(len != 0)
        memcpy(p.data, data, p.dataSize);
    p.crc = CRC_Calc_s16_CCITT((uint16_t*)&p, sizeof(PackageNetworkFormat)/sizeof(uint16_t)-1);
    client.write((const char*)&p, sizeof(PackageNetworkFormat));
}


void AppCore::getParam(const uint16_t id, QByteArray &ba)
{
    PackageNetworkFormat p;
    memset(&p, 0, sizeof (PackageNetworkFormat));
    f_waitAnswer = true;
    m_waitedAnswerId = id;
    p.counter = countTx++;
    countTx %=0xff;
    p.msgType = MessageTypeGet;
    p.cmdId = id;
    p.dataSize = 0;

    p.crc = CRC_Calc_s16_CCITT((uint16_t*)&p, sizeof(PackageNetworkFormat)/sizeof(uint16_t)-1);
    client.write((const char*)&p, sizeof(PackageNetworkFormat));
    QEventLoop loop ;
    ba.clear();
    connect(this, &AppCore::waitedAnswerParams, &loop, &QEventLoop::quit);
    QTimer::singleShot(6000, &loop, &QEventLoop::quit);
    loop.exec();
    disconnect(this, &AppCore::waitedAnswerParams, &loop, &QEventLoop::quit);
    f_waitAnswer = false;
    if(b_answer.length() == 0) {
        return;
    }
    ba.append(b_answer);
}

bool AppCore::getParam16Bit(const uint16_t id, quint16 &param)
{
   QByteArray ba;
   getParam(id, ba);
   if(ba.length() != 0) {

       memcpy(&param, ba.data(), sizeof (uint16_t));
       return true;
   }
   return false;
}

ParamsWorkMode *AppCore::paramsWM() const
{
    return m_paramsWM;
}

void AppCore::setParamsWM(ParamsWorkMode *newParamsWM)
{
    m_paramsWM = newParamsWM;
}

void AppCore::setModelWM(ProgrammsModel *newModelWM)
{
    if (m_modelWM == newModelWM)
        return;
    m_modelWM = newModelWM;
    emit modelWMChanged();
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
            QTimer::singleShot(6000, &l, &QEventLoop::quit);
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






