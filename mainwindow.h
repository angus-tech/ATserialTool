#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "serial/serialmanager.h"
#include "atcommand/atcommandmanager.h"
#include "network/networkmanager.h"
#include "ui/uimanager.h"
#include "ui/uiactionhandler.h"

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
    // 这些槽函数现在只是简单地调用 actionHandler 的方法
    void on_btnRefresh_clicked() { m_actionHandler->handleRefresh(); }
    void on_btnOpenPort_clicked() { m_actionHandler->handleOpenPort(); }
    void on_btnSend_clicked() { m_actionHandler->handleSendData(); }
    void on_btnClearReceive_clicked() { m_actionHandler->handleClearReceive(); }
    void on_btnClearSend_clicked() { m_actionHandler->handleClearSend(); }

    void on_btnAT_clicked() { m_actionHandler->handleAT(); }
    void on_btnATGMR_clicked() { m_actionHandler->handleATGMR(); }
    void on_btnATE0_clicked() { m_actionHandler->handleATE0(); }
    void on_btnATE1_clicked() { m_actionHandler->handleATE1(); }
    void on_btnATRST_clicked() { m_actionHandler->handleATRST(); }
    void on_btnATCWLIF_clicked() { m_actionHandler->handleATCWLIF(); }
    void on_btnATCWJAP_clicked() { m_actionHandler->handleATCWJAP(); }
    void on_btnATCIFSR_clicked() { m_actionHandler->handleATCIFSR(); }

    void on_btnModeAP_clicked() { m_actionHandler->handleModeAP(); }
    void on_btnModeSTA_clicked() { m_actionHandler->handleModeSTA(); }
    void on_btnModeAPSTA_clicked() { m_actionHandler->handleModeAPSTA(); }
    void on_btnNormalMode_clicked() { m_actionHandler->handleNormalMode(); }
    void on_btnTransparentMode_clicked() { m_actionHandler->handleTransparentMode(); }
    void on_btnEnableMux_clicked() { m_actionHandler->handleEnableMux(); }
    void on_btnEnableSingle_clicked() { m_actionHandler->handleEnableSingle(); }
    void on_btnSendData_clicked() { m_actionHandler->handleSendDataCmd(); }
    void on_btnExitSend_clicked() { m_actionHandler->handleExitSend(); }

    void on_btnSaveWiFi_clicked() { m_actionHandler->handleSaveWiFi(); }
    void on_btnConnectWithSettings_clicked() { m_actionHandler->handleConnectWiFi(); }

    void on_btnSaveTCP_clicked() { m_actionHandler->handleSaveTCP(); }
    void on_btnConnectTCP_clicked() { m_actionHandler->handleConnectTCP(); }
    void on_btnCloseConn_clicked() { m_actionHandler->handleCloseConn(); }
    void on_comboConnID_currentIndexChanged(int index) { m_actionHandler->handleConnIDChanged(index); }

    void on_btnSendCustom_clicked() { m_actionHandler->handleSendCustom(); }

    void on_cbTimerSend_toggled(bool checked) { m_actionHandler->handleTimerSendToggled(checked); }
    void on_spinTimerPeriod_valueChanged(int value) { m_actionHandler->handleTimerPeriodChanged(value); }
    void timerSendData() { m_actionHandler->handleTimerTimeout(); }

    void on_cbAutoScroll_toggled(bool checked) { m_autoScroll = checked; }

    void on_actionExit_triggered() { m_actionHandler->handleExit(); }
    void on_actionAbout_triggered() { m_actionHandler->handleAbout(); }
    void on_actionOpen_triggered() { m_actionHandler->handleOpenFile(); }
    void on_actionSave_triggered() { m_actionHandler->handleSaveFile(); }

    // 信号处理函数（这些保留在 MainWindow 中）
    void handleSerialData(const QByteArray &data);
    void handlePortStatusChanged(bool isOpen);
    void handleConnectionStatusChanged(int connId, bool connected, const QString &info);
    void handleConnectionTimeout(int connId);
    void processBufferedData();

private:
    void setupConnections();

private:
    Ui::MainWindow *ui;
    SerialManager *m_serialMgr;
    ATCommandManager *m_atCmdMgr;
    NetworkManager *m_networkMgr;
    UIManager *m_uiMgr;
    UIActionHandler *m_actionHandler;  // 新增的动作处理器

    QTimer *m_dataTimer;
    QByteArray m_dataBuffer;
    bool m_autoScroll;
};

#endif // MAINWINDOW_H
