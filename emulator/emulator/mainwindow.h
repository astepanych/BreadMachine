#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void putByte(const uint8_t byte);
private slots:
    void on_pbConnect_toggled(bool checked);
    void readyRead();

private:
    Ui::MainWindow *ui;
    QSerialPort *port;
    QByteArray readBuf;
    QByteArray readPackage;
    void swap(QByteArray &ba);
};
#endif // MAINWINDOW_H
