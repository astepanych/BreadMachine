#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/qserialportinfo.h>
#include <qdebug.h>
#include <qbytearray.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
        ui->cbListSerialPort->addItem(info.portName());
    }
    port = new QSerialPort(this);
    connect(port,&QSerialPort::readyRead, this, &MainWindow::readyRead);
    QString str = "Батон нарезной";
    QByteArray ba;


    char *p = (char*)str.data();

    ba.append(p,str.length()*2);
    swap(ba);
    qDebug()<<ba.toHex();
}

void MainWindow::swap(QByteArray &ba) {
    if(ba.length() % 2)
        ba.append((char)0);

    for(int i = 0; i<ba.length(); i+=2) {
        char a = ba[i];
        ba[i] = ba[i+1];
        ba[i+1] = a;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pbConnect_toggled(bool checked)
{
    if(checked){
        port->setPortName(ui->cbListSerialPort->currentText());
        port->setBaudRate(QSerialPort::Baud115200);
        if(port->open(QIODevice::ReadWrite))
            ui->pbConnect->setText("Отключить");
        else {
            qDebug()<<"no open port";
        }

    } else {
        port->close();
        ui->pbConnect->setText("Подключить");
    }
}

void MainWindow::readyRead()
{
    if(port->bytesAvailable()>0) {
        readBuf = port->readAll();
        qDebug()<<readBuf.toHex();
    }
}

