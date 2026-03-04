#include "uimanager.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>
#include <QDateTime>      // 添加这行
#include <cctype>

UIManager::UIManager(QObject *parent)
    : QObject(parent)
{
}

void UIManager::initUI(Ui::MainWindow *ui)
{
    // 初始化波特率
    ui->cbBaudRate->addItems(QStringList() << "9600" << "115200");
    ui->cbBaudRate->setCurrentText("115200");

    // 设置字体
    QFont font("Courier New", 10);
    ui->textReceive->setFont(font);
    ui->textSend->setFont(font);

    // 设置IP输入掩码
    ui->lineEditIP->setInputMask("000.000.000.000;_");

    // 设置连接ID默认值
    ui->comboConnID->setCurrentIndex(0);

    // 初始状态
    showStatusMessage(ui, "就绪");
}

void UIManager::refreshPortList(Ui::MainWindow *ui, const QStringList &ports)
{
    ui->cbPortName->clear();
    ui->cbPortName->addItems(ports);

    if(ports.size() > 0) {
        showStatusMessage(ui, QString("找到 %1 个串口").arg(ports.size()));
    }
}

void UIManager::updatePortStatus(Ui::MainWindow *ui, bool isOpen)
{
    if(isOpen) {
        ui->btnOpenPort->setText("关闭串口");
        ui->cbPortName->setEnabled(false);
        ui->cbBaudRate->setEnabled(false);
        ui->btnRefresh->setEnabled(false);

        ui->groupBoxAT->setEnabled(true);
        ui->groupBoxSettings->setEnabled(true);
    } else {
        ui->btnOpenPort->setText("打开串口");
        ui->cbPortName->setEnabled(true);
        ui->cbBaudRate->setEnabled(true);
        ui->btnRefresh->setEnabled(true);

        ui->groupBoxAT->setEnabled(false);
        ui->groupBoxSettings->setEnabled(false);

        if(ui->cbTimerSend->isChecked()) {
            ui->cbTimerSend->setChecked(false);
        }
    }
}

void UIManager::appendData(Ui::MainWindow *ui, const QString &data,
                          const QString &timestamp, bool isTx)
{
    QString prefix = isTx ? "TX" : "RX";
    QString displayData = QString("[%1] %2: %3")
                         .arg(timestamp).arg(prefix).arg(data);
    ui->textReceive->append(displayData);

    if(ui->cbAutoScroll->isChecked()) {
        QTextCursor cursor = ui->textReceive->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->textReceive->setTextCursor(cursor);
    }
}

QString UIManager::formatHexData(const QByteArray &data)
{
    QString hexData;
    for(int i = 0; i < data.size(); i++) {
        hexData += QString("%1 ").arg((unsigned char)data[i], 2, 16, QChar('0')).toUpper();
    }
    return hexData;
}

QString UIManager::formatTextData(const QByteArray &data)
{
    QString textData;
    for(int i = 0; i < data.size(); i++) {
        char c = data[i];
        if(isprint(c) || c == '\n' || c == '\r' || c == '\t') {
            textData += c;
        } else {
            textData += QString("[%1]").arg((unsigned char)c, 2, 16, QChar('0')).toUpper();
        }
    }
    return textData;
}

void UIManager::appendReceiveData(Ui::MainWindow *ui, const QByteArray &data,
                                 bool hexDisplay, bool autoScroll)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString displayData;

    if(hexDisplay) {
        displayData = formatHexData(data);
    } else {
        displayData = formatTextData(data);
    }

    appendData(ui, displayData, timestamp, false);
}

void UIManager::updateConnStatus(Ui::MainWindow *ui, int id, bool connected,
                                const QString &info)
{
    QString statusText = connected ? "已连接" : "未连接";
    QString color = connected ? "green" : "red";
    QString styleSheet = QString("color: %1; font-weight: %2;")
                        .arg(color).arg(connected ? "bold" : "normal");

    switch(id) {
        case 0:
            ui->labelConn0Status->setText(statusText);
            ui->labelConn0Status->setStyleSheet(styleSheet);
            if(!info.isEmpty()) ui->labelConn0Status->setToolTip(info);
            break;
        case 1:
            ui->labelConn1Status->setText(statusText);
            ui->labelConn1Status->setStyleSheet(styleSheet);
            if(!info.isEmpty()) ui->labelConn1Status->setToolTip(info);
            break;
        case 2:
            ui->labelConn2Status->setText(statusText);
            ui->labelConn2Status->setStyleSheet(styleSheet);
            if(!info.isEmpty()) ui->labelConn2Status->setToolTip(info);
            break;
        case 3:
            ui->labelConn3Status->setText(statusText);
            ui->labelConn3Status->setStyleSheet(styleSheet);
            if(!info.isEmpty()) ui->labelConn3Status->setToolTip(info);
            break;
        case 4:
            ui->labelConn4Status->setText(statusText);
            ui->labelConn4Status->setStyleSheet(styleSheet);
            if(!info.isEmpty()) ui->labelConn4Status->setToolTip(info);
            break;
    }
}

void UIManager::showStatusMessage(Ui::MainWindow *ui, const QString &msg, int timeout)
{
    ui->statusbar->showMessage(msg, timeout);
}

QString UIManager::getWiFiSettings(Ui::MainWindow *ui, bool showWarning)
{
    QString ssid = ui->lineEditSSID->text().trimmed();
    QString password = ui->lineEditPassword->text().trimmed();

    if(ssid.isEmpty() && showWarning) {
        QMessageBox::warning(nullptr, "警告", "请输入WiFi SSID！");
        return QString();
    }

    return QString("AT+CWJAP=\"%1\",\"%2\"").arg(ssid).arg(password);
}

QString UIManager::getTCPSettings(Ui::MainWindow *ui, bool showWarning)
{
    QString ip = ui->lineEditIP->text().trimmed();
    int port = ui->spinPort->value();
    int connId = ui->comboConnID->currentText().toInt();
    QString type = ui->comboConnType->currentText();

    if(ip.isEmpty() && showWarning) {
        QMessageBox::warning(nullptr, "警告", "请输入IP地址！");
        return QString();
    }

    // 检查IP格式
    QRegularExpression ipRegex("^(\\d{1,3}\\.){3}\\d{1,3}$");
    if(!ipRegex.match(ip).hasMatch() && showWarning) {
        QMessageBox::warning(nullptr, "警告", "IP地址格式不正确！");
        return QString();
    }

    return QString("AT+CIPSTART=%1,\"%2\",\"%3\",%4")
            .arg(connId).arg(type).arg(ip).arg(port);
}
