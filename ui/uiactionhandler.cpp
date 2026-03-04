#include "uiactionhandler.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QRegularExpression>

UIActionHandler::UIActionHandler(Ui::MainWindow *ui,
                                 SerialManager *serialMgr,
                                 ATCommandManager *atCmdMgr,
                                 NetworkManager *networkMgr,
                                 UIManager *uiMgr,
                                 QObject *parent)
    : QObject(parent)
    , ui(ui)
    , m_serialMgr(serialMgr)
    , m_atCmdMgr(atCmdMgr)
    , m_networkMgr(networkMgr)
    , m_uiMgr(uiMgr)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &UIActionHandler::handleTimerTimeout);
}

void UIActionHandler::sendATCommand(const QString &command)
{
    qint64 bytes = m_serialMgr->writeCommand(command);
    if(bytes > 0) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        m_uiMgr->appendData(ui, command, timestamp, true);
        m_uiMgr->showStatusMessage(ui, QString("已发送: %1").arg(command), 1000);
    }
}

bool UIActionHandler::validateIPAddress(const QString &ip)
{
    QRegularExpression ipRegex("^(\\d{1,3}\\.){3}\\d{1,3}$");
    if(!ipRegex.match(ip).hasMatch()) return false;

    QStringList parts = ip.split(".");
    for(const QString &part : parts) {
        int val = part.toInt();
        if(val < 0 || val > 255) return false;
    }
    return true;
}

//=============================================================================
// 串口操作
//=============================================================================
void UIActionHandler::handleRefresh()
{
    m_uiMgr->refreshPortList(ui, m_serialMgr->getAvailablePorts());
}

void UIActionHandler::handleOpenPort()
{
    if(m_serialMgr->isOpen()) {
        m_serialMgr->closePort();
    } else {
        if(ui->cbPortName->currentText().isEmpty()) {
            QMessageBox::warning(nullptr, "警告", "请选择串口！");
            return;
        }

        QString portName = ui->cbPortName->currentText();
        qint32 baudRate = ui->cbBaudRate->currentText().toInt();

        if(m_serialMgr->openPort(portName, baudRate)) {
            m_uiMgr->showStatusMessage(ui,
                QString("串口 %1 已打开，波特率 %2").arg(portName).arg(baudRate));
        } else {
            QMessageBox::critical(nullptr, "错误",
                QString("无法打开串口 %1！\n%2")
                .arg(portName).arg(m_serialMgr->getLastError()));
        }
    }
}

void UIActionHandler::handleSendData()
{
    if(!m_serialMgr->isOpen()) {
        QMessageBox::warning(nullptr, "警告", "请先打开串口！");
        return;
    }

    if(ui->textSend->toPlainText().isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "请输入要发送的数据！");
        return;
    }

    QByteArray sendData;
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    if(ui->cbHexSend->isChecked()) {
        QString text = ui->textSend->toPlainText();
        text.remove(' ').remove('\n').remove('\r').remove('\t');

        QRegularExpression hexRegex("^[0-9A-Fa-f]*$");
        if(!hexRegex.match(text).hasMatch()) {
            QMessageBox::warning(nullptr, "警告", "十六进制格式不正确！");
            return;
        }

        if(text.length() % 2 != 0) {
            QMessageBox::warning(nullptr, "警告", "十六进制字符串长度必须为偶数！");
            return;
        }

        sendData = QByteArray::fromHex(text.toLocal8Bit());
    } else {
        sendData = ui->textSend->toPlainText().toLocal8Bit();
    }

    qint64 bytes = m_serialMgr->writeData(sendData);
    if(bytes > 0) {
        QString displayData = ui->cbHexDisplay->isChecked() ?
            m_uiMgr->formatHexData(sendData) :
            QString::fromLocal8Bit(sendData);
        m_uiMgr->appendData(ui, displayData, timestamp, true);
        m_uiMgr->showStatusMessage(ui, QString("已发送 %1 字节").arg(bytes));
    }
}

void UIActionHandler::handleClearReceive() { ui->textReceive->clear(); }
void UIActionHandler::handleClearSend() { ui->textSend->clear(); }

//=============================================================================
// AT指令
//=============================================================================
void UIActionHandler::handleAT() { sendATCommand(m_atCmdMgr->cmdAT()); }
void UIActionHandler::handleATGMR() { sendATCommand(m_atCmdMgr->cmdGMR()); }
void UIActionHandler::handleATE0() { sendATCommand(m_atCmdMgr->cmdATE0()); }
void UIActionHandler::handleATE1() { sendATCommand(m_atCmdMgr->cmdATE1()); }
void UIActionHandler::handleATRST() { sendATCommand(m_atCmdMgr->cmdRST()); }
void UIActionHandler::handleATCWLIF() { sendATCommand(m_atCmdMgr->cmdCWLIF()); }
void UIActionHandler::handleATCWJAP() { sendATCommand(m_atCmdMgr->cmdCWJAP()); }
void UIActionHandler::handleATCIFSR() { sendATCommand(m_atCmdMgr->cmdCIFSR()); }

//=============================================================================
// 模式设置
//=============================================================================
void UIActionHandler::handleModeAP() { sendATCommand(m_atCmdMgr->cmdModeAP()); }
void UIActionHandler::handleModeSTA() { sendATCommand(m_atCmdMgr->cmdModeSTA()); }
void UIActionHandler::handleModeAPSTA() { sendATCommand(m_atCmdMgr->cmdModeAPSTA()); }
void UIActionHandler::handleNormalMode() { sendATCommand(m_atCmdMgr->cmdCIPMODE(false)); }
void UIActionHandler::handleTransparentMode() { sendATCommand(m_atCmdMgr->cmdCIPMODE(true)); }
void UIActionHandler::handleEnableMux() { sendATCommand(m_atCmdMgr->cmdCIPMUX(true)); }
void UIActionHandler::handleEnableSingle() { sendATCommand(m_atCmdMgr->cmdCIPMUX(false)); }

void UIActionHandler::handleSendDataCmd()
{
    int connId = ui->comboConnID->currentText().toInt();
    sendATCommand(m_atCmdMgr->buildCIPSEND(connId, 5));
}

void UIActionHandler::handleExitSend() { sendATCommand(m_atCmdMgr->cmdExitSend()); }

//=============================================================================
// WiFi设置
//=============================================================================
void UIActionHandler::handleSaveWiFi()
{
    QString ssid = ui->lineEditSSID->text().trimmed();
    QString pwd = ui->lineEditPassword->text().trimmed();

    if(ssid.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "WiFi SSID不能为空！");
        return;
    }

    m_networkMgr->saveWiFiSettings(ssid, pwd);
    m_uiMgr->showStatusMessage(ui, QString("WiFi设置已保存: %1").arg(ssid));
}

void UIActionHandler::handleConnectWiFi()
{
    QString cmd = m_uiMgr->getWiFiSettings(ui, true);
    if(!cmd.isEmpty()) {
        sendATCommand(cmd);
    }
}

//=============================================================================
// TCP设置
//=============================================================================
void UIActionHandler::handleSaveTCP()
{
    QString ip = ui->lineEditIP->text().trimmed();
    int port = ui->spinPort->value();
    int connId = ui->comboConnID->currentText().toInt();

    if(ip.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "IP地址不能为空！");
        return;
    }

    m_networkMgr->saveTCPSettings(connId, ip, port);
    m_uiMgr->showStatusMessage(ui,
        QString("TCP设置已保存: %1:%2 (ID:%3)").arg(ip).arg(port).arg(connId));
}

void UIActionHandler::handleConnectTCP()
{
    if(!m_serialMgr->isOpen()) {
        QMessageBox::warning(nullptr, "警告", "请先打开串口！");
        return;
    }

    QString ip = ui->lineEditIP->text().trimmed();
    int port = ui->spinPort->value();
    int connId = ui->comboConnID->currentText().toInt();
    QString type = ui->comboConnType->currentText();

    if(ip.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "请输入IP地址！");
        return;
    }

    if(!validateIPAddress(ip)) {
        QMessageBox::warning(nullptr, "警告", "IP地址格式不正确！");
        return;
    }

    m_networkMgr->startConnection(connId, ip, port, type);
    QString cmd = m_atCmdMgr->buildCIPSTART(connId, type, ip, port);
    sendATCommand(cmd);
}

void UIActionHandler::handleCloseConn()
{
    int connId = ui->comboConnID->currentText().toInt();
    sendATCommand(m_atCmdMgr->cmdCIPCLOSE(connId));
}

void UIActionHandler::handleConnIDChanged(int index)
{
    auto info = m_networkMgr->getConnectionInfo(index);
    if(info.isConnected) {
        m_uiMgr->showStatusMessage(ui,
            QString("连接%1: %2:%3 - 已连接").arg(index).arg(info.ip).arg(info.port));
    }
}

//=============================================================================
// 自定义指令
//=============================================================================
void UIActionHandler::handleSendCustom()
{
    QString cmd = ui->lineEditCustomAT->text().trimmed();
    if(cmd.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "请输入AT指令！");
        return;
    }

    if(cmd == "+++") {
        sendATCommand("+++");
    } else if(cmd.toInt() > 0 && ui->comboConnID->isEnabled()) {
        int connId = ui->comboConnID->currentText().toInt();
        sendATCommand(m_atCmdMgr->buildCIPSEND(connId, cmd.toInt()));
    } else if(!cmd.startsWith("AT", Qt::CaseInsensitive)) {
        sendATCommand("AT" + cmd);
    } else {
        sendATCommand(cmd);
    }

    ui->lineEditCustomAT->clear();
}

//=============================================================================
// 定时发送
//=============================================================================
void UIActionHandler::handleTimerSendToggled(bool checked)
{
    if(checked) {
        if(!m_serialMgr->isOpen()) {
            QMessageBox::warning(nullptr, "警告", "请先打开串口！");
            return;
        }
        m_timer->start(ui->spinTimerPeriod->value());
        m_uiMgr->showStatusMessage(ui,
            QString("定时发送已开启，间隔 %1 ms").arg(ui->spinTimerPeriod->value()));
    } else {
        m_timer->stop();
        m_uiMgr->showStatusMessage(ui, "定时发送已关闭");
    }
}

void UIActionHandler::handleTimerPeriodChanged(int value)
{
    if(m_timer->isActive()) {
        m_timer->setInterval(value);
    }
}

void UIActionHandler::handleTimerTimeout()
{
    handleSendData();
}

//=============================================================================
// 菜单
//=============================================================================
void UIActionHandler::handleExit()
{
    QApplication::quit();
}

void UIActionHandler::handleAbout()
{
    QMessageBox::about(nullptr, "关于",
        "串口调试助手 - AT指令版 V1.0\n\n"
        "功能特点：\n"
        "• 支持常用AT指令一键发送\n"
        "• 十六进制/ASCII显示切换\n"
        "• 定时发送功能\n"
        "• 自定义AT指令输入\n\n"
        "作者：hbchen\n"
        "版本：2026.03");
}

void UIActionHandler::handleOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr,
        "打开发送文件", "", "文本文件 (*.txt);;所有文件 (*)");

    if(!fileName.isEmpty()) {
        QFile file(fileName);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ui->textSend->setPlainText(file.readAll());
            file.close();
            m_uiMgr->showStatusMessage(ui, QString("已加载文件: %1").arg(fileName));
        }
    }
}

void UIActionHandler::handleSaveFile()
{
    QString fileName = QFileDialog::getSaveFileName(nullptr,
        "保存接收数据", "", "文本文件 (*.txt);;所有文件 (*)");

    if(!fileName.isEmpty()) {
        QFile file(fileName);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(ui->textReceive->toPlainText().toLocal8Bit());
            file.close();
            m_uiMgr->showStatusMessage(ui, QString("数据已保存到: %1").arg(fileName));
        }
    }
}
