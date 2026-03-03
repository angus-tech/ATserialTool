#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QRegularExpression>

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
    // 串口操作
    void on_btnRefresh_clicked();
    void on_btnOpenPort_clicked();
    void on_btnSend_clicked();
    void on_btnClearReceive_clicked();
    void on_btnClearSend_clicked();

    // AT指令按钮 - 基础指令
    void on_btnAT_clicked();
    void on_btnATGMR_clicked();
    void on_btnATE0_clicked();
    void on_btnATE1_clicked();
    void on_btnATRST_clicked();
    void on_btnATV1_clicked();
    void on_btnATQ0_clicked();
    void on_btnATQ1_clicked();

    // AT指令按钮 - 设备信息
    void on_btnATCWMODEAP_clicked();
    void on_btnATCWMODESTA_clicked();
    void on_btnATCWMODEAPSTA_clicked();

    // AT指令按钮 - 网络状态
    void on_btnATCWLAP_clicked();
    void on_btnATCWJAPwifi_clicked();
    void on_btnATCIFSR_clicked();

    // AT指令按钮 - SIM卡信息
    void on_btnATCBC_clicked();
    void on_btnATCCID_clicked();
    void on_btnATCNUM_clicked();

    // 自定义AT指令
    void on_btnSendCustom_clicked();

    // 数据接收
    void readSerialData();

    // 定时发送
    void on_cbTimerSend_toggled(bool checked);
    void on_spinTimerPeriod_valueChanged(int value);
    void timerSendData();

    // 其他控件
    void on_cbAutoScroll_toggled(bool checked);

    // 菜单操作
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();

private:
    // 辅助函数 - 声明所有私有函数
    void initUI();                    // 初始化UI
    void sendATCommand(const QString &command);  // 发送AT指令
    void refreshPortList();            // 刷新串口列表
    void updatePortStatus(bool isOpen); // 更新串口状态
    void appendReceiveData(const QByteArray &data); // 添加接收数据

private:
    Ui::MainWindow *ui;
    QSerialPort *m_serialPort;
    QTimer *m_timer;
    bool m_autoScroll;
};

#endif // MAINWINDOW_H
