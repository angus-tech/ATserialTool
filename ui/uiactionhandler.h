#ifndef UIACTIONHANDLER_H
#define UIACTIONHANDLER_H

#include <QObject>
#include "serial/serialmanager.h"
#include "atcommand/atcommandmanager.h"
#include "network/networkmanager.h"
#include "ui/uimanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class UIActionHandler : public QObject
{
    Q_OBJECT

public:
    explicit UIActionHandler(Ui::MainWindow *ui, 
                            SerialManager *serialMgr,
                            ATCommandManager *atCmdMgr,
                            NetworkManager *networkMgr,
                            UIManager *uiMgr,
                            QObject *parent = nullptr);

    // 串口操作
    void handleRefresh();
    void handleOpenPort();
    void handleSendData();
    void handleClearReceive();
    void handleClearSend();

    // AT指令
    void handleAT();
    void handleATGMR();
    void handleATE0();
    void handleATE1();
    void handleATRST();
    void handleATCWLIF();
    void handleATCWJAP();
    void handleATCIFSR();

    // 模式设置
    void handleModeAP();
    void handleModeSTA();
    void handleModeAPSTA();
    void handleNormalMode();
    void handleTransparentMode();
    void handleEnableMux();
    void handleEnableSingle();
    void handleSendDataCmd();
    void handleExitSend();

    // WiFi设置
    void handleSaveWiFi();
    void handleConnectWiFi();

    // TCP设置
    void handleSaveTCP();
    void handleConnectTCP();
    void handleCloseConn();
    void handleConnIDChanged(int index);

    // 自定义指令
    void handleSendCustom();

    // 定时发送
    void handleTimerSendToggled(bool checked);
    void handleTimerPeriodChanged(int value);
    void handleTimerTimeout();

    // 菜单
    void handleExit();
    void handleAbout();
    void handleOpenFile();
    void handleSaveFile();

private:
    void sendATCommand(const QString &command);
    bool validateIPAddress(const QString &ip);

private:
    Ui::MainWindow *ui;
    SerialManager *m_serialMgr;
    ATCommandManager *m_atCmdMgr;
    NetworkManager *m_networkMgr;
    UIManager *m_uiMgr;
    QTimer *m_timer;
};

#endif // UIACTIONHANDLER_H