#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnRefresh_clicked();
    void on_btnOpenPort_clicked();
    void on_btnSend_clicked();
    void on_btnClearReceive_clicked();
    void on_btnClearSend_clicked();
    void readSerialData();

private:
    Ui::MainWindow *ui;
    QSerialPort *m_serialPort;

    void refreshPortList();
    void updatePortStatus(bool isOpen);
};

#endif // MAINWINDOW_H
