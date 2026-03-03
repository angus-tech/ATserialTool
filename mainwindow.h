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
#include <QMap>
#include <QString>  // 添加这行

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
    void on_btnATCWLIF_clicked();
    void on_btnATCIPMODE0_clicked();
    void on_btnATCIPMODE1_clicked();

    // AT指令按钮 - 设备信息
    void on_btnATCWMODEAP_clicked();
    void on_btnATCWMODESTA_clicked();
    void on_btnATCWMODEAPSTA_clicked();

    // AT指令按钮 - 网络状态
    void on_btnScanWIFI_clicked();
    void on_btnConnectWIFI_clicked();
    void on_btnReadIP_clicked();

    // AT指令按钮 - TCP/IP
    void on_btnConnectAP_clicked();
    void on_btnIntoSend_clicked();
    void on_btnExitSend_clicked();

    // 新增：设定信息组相关槽函数
    void on_btnSaveWiFi_clicked();              // 保存WiFi设置
    void on_btnConnectWithSettings_clicked();    // 使用设置连接WiFi
    void on_btnEnableMux_clicked();              // 启用多连接
    void on_btnDisableMux_clicked();             // 禁用多连接
    void on_btnSaveTCP_clicked();                // 保存TCP设置
    void on_btnConnectTCP_clicked();             // 建立TCP连接
    void on_btnCloseConn_clicked();              // 关闭指定连接
    void on_comboConnID_currentIndexChanged(int index); // 连接ID改变

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

    void processBufferedData();  // 处理缓存的串口数据

private:
    // 辅助函数
    void initUI();                    // 初始化UI
    void sendATCommand(const QString &command);  // 发送AT指令
    void refreshPortList();            // 刷新串口列表
    void updatePortStatus(bool isOpen); // 更新串口状态
    void appendReceiveData(const QByteArray &data); // 添加接收数据
    void updateConnStatus(int id, bool connected, const QString &info = ""); // 更新连接状态
    QString getCurrentWiFiSettings();  // 获取当前WiFi设置字符串
    QString getCurrentTCPSettings();   // 获取当前TCP设置字符串

    // TCP连接状态管理
    struct ConnectionInfo {
        bool isConnected;
        QString ip;
        int port;
        QString type;
        QString remoteInfo;
    };

    QMap<int, ConnectionInfo> m_connections;  // 存储多连接状态
    int m_currentConnId;                       // 当前选中的连接ID
    QString m_savedSSID;                        // 保存的SSID
    QString m_savedPassword;                     // 保存的密码
    QString m_savedIP;                           // 保存的IP
    int m_savedPort;                             // 保存的端口

private:
    Ui::MainWindow *ui;
    QSerialPort *m_serialPort;
    QTimer *m_timer;
    bool m_autoScroll;
    QTimer *m_dataTimer;          // 数据延迟处理定时器
    QByteArray m_dataBuffer;      // 串口数据缓存
    bool m_isReceiving;            // 是否正在接收数据
    qint64 m_lastReceiveTime;      // 最后接收数据的时间
};

#endif // MAINWINDOW_H
