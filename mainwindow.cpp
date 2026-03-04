#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QScrollBar>

//=============================================================================
// 初始化函数
//=============================================================================
void MainWindow::initUI()
{
    // 初始化波特率下拉框
    ui->cbBaudRate->addItems(QStringList()
        << "9600" << "115200");
    ui->cbBaudRate->setCurrentText("115200");

    // 初始化数据位
//    ui->cbDataBits->addItems(QStringList() << "5" << "6" << "7" << "8");
//    ui->cbDataBits->setCurrentText("8");

    // 初始化停止位
//    ui->cbStopBits->addItems(QStringList() << "1" << "1.5" << "2");
//    ui->cbStopBits->setCurrentText("1");

    // 初始化校验位
//    ui->cbParity->addItems(QStringList() << "无" << "奇校验" << "偶校验" << "标志校验" << "空白校验");

    // 刷新串口列表
    refreshPortList();

    // 初始状态
    updatePortStatus(false);

    // 状态栏显示
    ui->statusbar->showMessage("就绪");

    // 设置接收区字体为等宽字体，更好显示十六进制
    QFont font("Courier New", 10);
    ui->textReceive->setFont(font);
    ui->textSend->setFont(font);
    // 初始化TCP连接状态显示
    for(int i = 0; i < 5; i++) {
        updateConnStatus(i, false);
    }

    // 初始化保存的变量
    m_currentConnId = 0;
    m_savedPort = 8080;

    // 设置连接ID下拉框默认值
    ui->comboConnID->setCurrentIndex(0);

    // 设置IP输入框的输入掩码（可选）
    ui->lineEditIP->setInputMask("000.000.000.000;_");
}

void MainWindow::refreshPortList()
{
    ui->cbPortName->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->cbPortName->addItem(info.portName());
    }

    if(ui->cbPortName->count() > 0) {
        ui->statusbar->showMessage(QString("找到 %1 个串口").arg(ui->cbPortName->count()), 2000);
    }
}

void MainWindow::updatePortStatus(bool isOpen)
{
    if(isOpen) {
        ui->btnOpenPort->setText("关闭串口");
        ui->cbPortName->setEnabled(false);
        ui->cbBaudRate->setEnabled(false);
        ui->btnRefresh->setEnabled(false);

        // 启用所有功能组
        ui->groupBoxAT->setEnabled(true);
        ui->groupBoxSettings->setEnabled(true);

        // 清空数据缓冲区
        m_dataBuffer.clear();
        if (m_dataTimer->isActive()) {
            m_dataTimer->stop();
        }

        // 重置连接状态
        for(int i = 0; i < 5; i++) {
            updateConnStatus(i, false);
        }
    } else {
        ui->btnOpenPort->setText("打开串口");
        ui->cbPortName->setEnabled(true);
        ui->cbBaudRate->setEnabled(true);
        ui->btnRefresh->setEnabled(true);

        // 禁用功能组
        ui->groupBoxAT->setEnabled(false);
        ui->groupBoxSettings->setEnabled(false);

        // 如果定时发送开启，关闭它
        if(ui->cbTimerSend->isChecked()) {
            ui->cbTimerSend->setChecked(false);
        }

        // 清空数据缓冲区
        m_dataBuffer.clear();
        if (m_dataTimer->isActive()) {
            m_dataTimer->stop();
        }

        // 重置连接状态
        for(int i = 0; i < 5; i++) {
            updateConnStatus(i, false);
        }
    }
}

//=============================================================================
// 数据发送接收函数
//=============================================================================
void MainWindow::sendATCommand(const QString &command)
{
    if(!m_serialPort->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口！");
        return;
    }

    QString cmd = command;
    if(cmd != "+++" && !cmd.endsWith("\n")) {
        cmd += "\r\n";  // AT指令通常需要以回车结尾
    }

    QByteArray data = cmd.toLocal8Bit();
    qint64 bytesWritten = m_serialPort->write(data);

    if(bytesWritten > 0) {
        // 在接收区显示发送的命令
        QString displayData = QString("[%1] TX: %2")
                             .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                             .arg(command);
        ui->textReceive->append(displayData);

        // 自动滚屏
        if(ui->cbAutoScroll->isChecked()) {
            QTextCursor cursor = ui->textReceive->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textReceive->setTextCursor(cursor);
        }

        ui->statusbar->showMessage(QString("已发送AT指令: %1").arg(command), 1000);
    }
}

void MainWindow::appendReceiveData(const QByteArray &data)
{
    QString displayData;
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    if(ui->cbHexDisplay->isChecked()) {
        // 十六进制显示
        QString hexData;
        for(int i = 0; i < data.size(); i++) {
            hexData += QString("%1 ").arg((unsigned char)data[i], 2, 16, QChar('0')).toUpper();
        }
        displayData = QString("[%1] RX: %2").arg(timestamp).arg(hexData);
    } else {
        // 文本显示 - 过滤掉非ASCII字符
        QString textData;
        for(int i = 0; i < data.size(); i++) {
            char c = data[i];
            if(isprint(c) || c == '\n' || c == '\r' || c == '\t') {
                textData += c;
            } else {
                textData += QString("[%1]").arg((unsigned char)c, 2, 16, QChar('0')).toUpper();
            }
        }
        displayData = QString("[%1] RX: %2").arg(timestamp).arg(textData);
    }

    ui->textReceive->append(displayData);

    // 自动滚屏
    if(ui->cbAutoScroll->isChecked()) {
        QTextCursor cursor = ui->textReceive->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->textReceive->setTextCursor(cursor);
    }
}

void MainWindow::processBufferedData()
{
    // 检查缓冲区是否为空
    if (m_dataBuffer.isEmpty()) {
        return;
    }

    // 获取当前时间戳
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    // 处理缓冲区的数据
    if (ui->cbHexDisplay->isChecked()) {
        // 十六进制显示
        QString hexData;
        for(int i = 0; i < m_dataBuffer.size(); i++) {
            hexData += QString("%1 ").arg((unsigned char)m_dataBuffer[i], 2, 16, QChar('0')).toUpper();
        }
        QString displayData = QString("[%1] RX: %2").arg(timestamp).arg(hexData);
        ui->textReceive->append(displayData);
    } else {
        // 文本显示 - 过滤掉非ASCII字符
        QString textData;
        for(int i = 0; i < m_dataBuffer.size(); i++) {
            char c = m_dataBuffer[i];
            if(isprint(c) || c == '\n' || c == '\r' || c == '\t') {
                textData += c;
            } else {
                textData += QString("[%1]").arg((unsigned char)c, 2, 16, QChar('0')).toUpper();
            }
        }
        QString displayData = QString("[%1] RX: %2").arg(timestamp).arg(textData);
        ui->textReceive->append(displayData);
    }

    // 自动滚屏
    if(ui->cbAutoScroll->isChecked()) {
        QTextCursor cursor = ui->textReceive->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->textReceive->setTextCursor(cursor);
    }

    // 清空缓冲区
    m_dataBuffer.clear();
}


//=============================================================================
// 构造函数和析构函数
//=============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serialPort(new QSerialPort(this))
    , m_timer(new QTimer(this))
    , m_dataTimer(new QTimer(this))
    , m_isReceiving(false)
    , m_lastReceiveTime(0)
    , m_autoScroll(true)
    , m_currentConnId(0)
    , m_savedPort(8080)
{
    ui->setupUi(this);

    // 初始化UI
    initUI();

    // 信号连接
   connect(m_serialPort, &QSerialPort::readyRead,
           this, &MainWindow::readSerialData);
   connect(m_timer, &QTimer::timeout,
           this, &MainWindow::timerSendData);
   connect(m_dataTimer, &QTimer::timeout,
           this, &MainWindow::processBufferedData);

   m_dataTimer->setSingleShot(true);
   m_dataTimer->setInterval(10);

   // 连接自定义AT指令输入框的回车信号
   connect(ui->lineEditCustomAT, &QLineEdit::returnPressed,
           this, &MainWindow::on_btnSendCustom_clicked);

   // 初始禁用AT指令按钮（直到串口打开）
   ui->groupBoxAT->setEnabled(false);
   ui->groupBoxSettings->setEnabled(false);
}

MainWindow::~MainWindow()
{
    if(m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    delete ui;
}

//=============================================================================
// 串口操作槽函数
//=============================================================================
void MainWindow::on_btnRefresh_clicked()
{
    refreshPortList();
}

void MainWindow::on_btnOpenPort_clicked()
{
    if(m_serialPort->isOpen()) {
        // 关闭串口
        m_serialPort->close();
        updatePortStatus(false);
        ui->statusbar->showMessage("串口已关闭");
    } else {
        // 检查是否选择了端口
        if(ui->cbPortName->currentText().isEmpty()) {
            QMessageBox::warning(this, "警告", "请选择串口！");
            return;
        }

        // 设置串口参数
        m_serialPort->setPortName(ui->cbPortName->currentText());
        m_serialPort->setBaudRate(ui->cbBaudRate->currentText().toInt());

        // 设置数据位
//        switch(ui->cbDataBits->currentText().toInt()) {
//            case 5: m_serialPort->setDataBits(QSerialPort::Data5); break;
//            case 6: m_serialPort->setDataBits(QSerialPort::Data6); break;
//            case 7: m_serialPort->setDataBits(QSerialPort::Data7); break;
//            case 8: m_serialPort->setDataBits(QSerialPort::Data8); break;
//            default: m_serialPort->setDataBits(QSerialPort::Data8); break;
//        }
        m_serialPort->setDataBits(QSerialPort::Data8);

        // 设置停止位
        m_serialPort->setStopBits(QSerialPort::OneStop);
//        if(ui->cbStopBits->currentText() == "1") {
//            m_serialPort->setStopBits(QSerialPort::OneStop);
//        } else if(ui->cbStopBits->currentText() == "1.5") {
//            m_serialPort->setStopBits(QSerialPort::OneAndHalfStop);
//        } else if(ui->cbStopBits->currentText() == "2") {
//            m_serialPort->setStopBits(QSerialPort::TwoStop);
//        }

        // 设置校验位
//        switch(ui->cbParity->currentIndex()) {
//            case 0: m_serialPort->setParity(QSerialPort::NoParity); break;
//            case 1: m_serialPort->setParity(QSerialPort::OddParity); break;
//            case 2: m_serialPort->setParity(QSerialPort::EvenParity); break;
//            case 3: m_serialPort->setParity(QSerialPort::MarkParity); break;
//            case 4: m_serialPort->setParity(QSerialPort::SpaceParity); break;
//            default: m_serialPort->setParity(QSerialPort::NoParity); break;
//        }
        m_serialPort->setParity(QSerialPort::NoParity);

        // 打开串口
        if(m_serialPort->open(QIODevice::ReadWrite)) {
            updatePortStatus(true);
            ui->statusbar->showMessage(QString("串口 %1 已打开，波特率 %2")
                                      .arg(ui->cbPortName->currentText())
                                      .arg(ui->cbBaudRate->currentText()));
        } else {
            QMessageBox::critical(this, "错误",
                QString("无法打开串口 %1！\n错误信息：%2")
                .arg(ui->cbPortName->currentText())
                .arg(m_serialPort->errorString()));
        }
    }
}

void MainWindow::readSerialData()
{
    // 读取所有可用数据
    QByteArray newData = m_serialPort->readAll();

    // 将新数据添加到缓冲区
    m_dataBuffer.append(newData);

    // 将接收到的数据转换为字符串用于解析
    QString response = QString::fromLocal8Bit(newData);

    // 调试输出，查看实际接收到的内容
    qDebug() << "收到数据:" << response;

    // 检查响应中是否包含连接成功信息
    if(response.contains("CONNECT")) {
        qDebug() << "检测到CONNECT响应";

        // 尝试匹配 "0,CONNECT" 或 "CONNECT" 后面跟数字
        QRegularExpression connRegex1("(\\d+),CONNECT");  // 匹配 "0,CONNECT"
        QRegularExpression connRegex2("CONNECT\\s+(\\d+)"); // 匹配 "CONNECT 0"
        QRegularExpression connRegex3("CONNECT"); // 只检测CONNECT，默认ID为0

        QRegularExpressionMatch match;
        int connId = -1;

        if(response.contains(connRegex1)) {
            match = connRegex1.match(response);
            if(match.hasMatch()) {
                connId = match.captured(1).toInt();
                qDebug() << "匹配到格式1: 连接ID =" << connId;
            }
        }
        else if(response.contains(connRegex2)) {
            match = connRegex2.match(response);
            if(match.hasMatch()) {
                connId = match.captured(1).toInt();
                qDebug() << "匹配到格式2: 连接ID =" << connId;
            }
        }
        else if(response.contains("CONNECT")) {
            // 如果没有数字，默认是单连接模式，ID为0
            connId = 0;
            qDebug() << "匹配到格式3: 连接ID = 0 (默认)";
        }

        if(connId >= 0 && connId <= 4) {
            updateConnStatus(connId, true, response);
            ui->statusbar->showMessage(QString("连接%1已建立").arg(connId), 2000);
        }
    }
    else if(response.contains("CLOSED")) {
        qDebug() << "检测到CLOSED响应";

        QRegularExpression closeRegex1("(\\d+),CLOSED");
        QRegularExpression closeRegex2("CLOSED\\s+(\\d+)");
        QRegularExpression closeRegex3("CLOSED");

        QRegularExpressionMatch match;
        int connId = -1;

        if(response.contains(closeRegex1)) {
            match = closeRegex1.match(response);
            if(match.hasMatch()) {
                connId = match.captured(1).toInt();
                qDebug() << "匹配到格式1: 关闭连接ID =" << connId;
            }
        }
        else if(response.contains(closeRegex2)) {
            match = closeRegex2.match(response);
            if(match.hasMatch()) {
                connId = match.captured(1).toInt();
                qDebug() << "匹配到格式2: 关闭连接ID =" << connId;
            }
        }
        else if(response.contains("CLOSED")) {
            connId = 0;
            qDebug() << "匹配到格式3: 关闭连接ID = 0 (默认)";
        }

        if(connId >= 0 && connId <= 4) {
            updateConnStatus(connId, false, "已关闭");
            ui->statusbar->showMessage(QString("连接%1已关闭").arg(connId), 2000);
        }
    }
    else if(response.contains("ALREADY CONNECT")) {
        // 处理已经连接的情况
        QRegularExpression connRegex("(\\d+),ALREADY CONNECT");
        QRegularExpressionMatch match = connRegex.match(response);
        if(match.hasMatch()) {
            int connId = match.captured(1).toInt();
            updateConnStatus(connId, true, "已连接");
            ui->statusbar->showMessage(QString("连接%1已经存在").arg(connId), 2000);
        }
    }

    // 记录最后接收时间
    m_lastReceiveTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // 如果定时器未启动，启动它
    if (!m_dataTimer->isActive()) {
        m_dataTimer->start();
    } else {
        // 如果定时器已启动，重新启动（重置10ms计时）
        m_dataTimer->stop();
        m_dataTimer->start();
    }
}

//=============================================================================
// 发送区操作
//=============================================================================
void MainWindow::on_btnSend_clicked()
{
    if(!m_serialPort->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口！");
        return;
    }

    if(ui->textSend->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要发送的数据！");
        return;
    }

    QByteArray sendData;
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    if(ui->cbHexSend->isChecked()) {
        // 十六进制发送
        QString text = ui->textSend->toPlainText();
        text.remove(' ');
        text.remove('\n');
        text.remove('\r');
        text.remove('\t');

        // 检查是否是有效的十六进制字符串
        QRegularExpression hexRegex("^[0-9A-Fa-f]*$");
        if(!hexRegex.match(text).hasMatch()) {
            QMessageBox::warning(this, "警告", "十六进制格式不正确！\n只能包含0-9,A-F,a-f和空格");
            return;
        }

        // 确保是偶数个字符
        if(text.length() % 2 != 0) {
            QMessageBox::warning(this, "警告", "十六进制字符串长度必须为偶数！");
            return;
        }

        sendData = QByteArray::fromHex(text.toLocal8Bit());
    } else {
        // 文本发送
        sendData = ui->textSend->toPlainText().toLocal8Bit();
    }

    // 发送数据
    qint64 bytesWritten = m_serialPort->write(sendData);

    if(bytesWritten > 0) {
        // 在接收区显示发送的数据
        QString displayData;
        if(ui->cbHexDisplay->isChecked()) {
            QString hexData;
            for(int i = 0; i < sendData.size(); i++) {
                hexData += QString("%1 ").arg((unsigned char)sendData[i], 2, 16, QChar('0')).toUpper();
            }
            displayData = QString("[%1] TX: %2").arg(timestamp).arg(hexData);
        } else {
            displayData = QString("[%1] TX: %2").arg(timestamp).arg(QString::fromLocal8Bit(sendData));
        }
        ui->textReceive->append(displayData);
        ui->statusbar->showMessage(QString("已发送 %1 字节").arg(bytesWritten), 2000);

        // 自动滚屏
        if(ui->cbAutoScroll->isChecked()) {
            QTextCursor cursor = ui->textReceive->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textReceive->setTextCursor(cursor);
        }
    }
}

void MainWindow::on_btnClearReceive_clicked()
{
    ui->textReceive->clear();
}

void MainWindow::on_btnClearSend_clicked()
{
    ui->textSend->clear();
}

//=============================================================================
// AT指令槽函数 - 基础指令
//=============================================================================
void MainWindow::on_btnAT_clicked() { sendATCommand("AT"); }
void MainWindow::on_btnATGMR_clicked() { sendATCommand("AT+GMR"); }
void MainWindow::on_btnATE0_clicked() { sendATCommand("ATE0"); }
void MainWindow::on_btnATE1_clicked() { sendATCommand("ATE1"); }
void MainWindow::on_btnATRST_clicked() { sendATCommand("AT+RST"); }
void MainWindow::on_btnATCWLIF_clicked() { sendATCommand("AT+CWLIF"); }
void MainWindow::on_btnATCWJAP_clicked() { sendATCommand("AT+CWJAP?"); }
void MainWindow::on_btnATCIFSR_clicked() { sendATCommand("AT+CIFSR"); }

void MainWindow::on_btnModeAP_clicked() { sendATCommand("AT+CWMODE=1"); }
void MainWindow::on_btnModeSTA_clicked() { sendATCommand("AT+CWMODE=2"); }
void MainWindow::on_btnModeAPSTA_clicked() { sendATCommand("AT+CWMODE=3"); }

void MainWindow::on_btnSendData_clicked() { sendATCommand(QString("AT+CIPSEND=%1,5").arg(ui->comboConnID->currentText().toInt())); }
void MainWindow::on_btnExitSend_clicked() { sendATCommand("+++"); }

void MainWindow::on_btnNormalMode_clicked() { sendATCommand("AT+CIPMUX=0"); }
void MainWindow::on_btnTransparentMode_clicked() { sendATCommand("AT+CIPMUX=1"); }

// //=============================================================================
// // AT指令槽函数 - 设备信息
// //=============================================================================
// void MainWindow::on_btnATCWMODESTA_clicked() { sendATCommand("AT+CWMODE=1"); }
// void MainWindow::on_btnATCWMODEAP_clicked() { sendATCommand("AT+CWMODE=2"); }
// void MainWindow::on_btnATCWMODEAPSTA_clicked() { sendATCommand("AT+CWMODE=3"); }

// //=============================================================================
// // AT指令槽函数 - 网络状态
// //=============================================================================
// void MainWindow::on_btnScanWIFI_clicked() { sendATCommand("AT+CWLAP"); }
// void MainWindow::on_btnConnectWIFI_clicked() { sendATCommand("AT+CWJAP=\"Sheentec_work\",\"sheentec\""); }
// void MainWindow::on_btnReadIP_clicked() { sendATCommand("AT+CIFSR"); }

// //=============================================================================
// // AT指令槽函数 - 通讯测试
// //=============================================================================
// void MainWindow::on_btnConnectAP_clicked() { sendATCommand("AT+CIPSTART=\"TCP\",\"192.168.20.2\",8888"); }
// void MainWindow::on_btnIntoSend_clicked() { sendATCommand("AT+CIPSEND"); }
// void MainWindow::on_btnExitSend_clicked() { sendATCommand("+++"); }

//=============================================================================
// 自定义AT指令
//=============================================================================
void MainWindow::on_btnSendCustom_clicked()
{
    QString customCmd = ui->lineEditCustomAT->text().trimmed();
    if(customCmd.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入AT指令！");
        return;
    }

    // 如果输入的是+++，直接发送
    if(customCmd == "+++") {
        sendATCommand("+++");
    }
    // 如果输入的是数字，可能是发送数据长度命令
    else if(customCmd.toInt() > 0 && ui->comboConnID->isEnabled()) {
        int connId = ui->comboConnID->currentText().toInt();
        QString cmd = QString("AT+CIPSEND=%1,%2").arg(connId).arg(customCmd);
        sendATCommand(cmd);
    }
    // 自动添加AT前缀（如果没有且不是特殊指令）
    else if(!customCmd.startsWith("AT", Qt::CaseInsensitive) &&
            !customCmd.startsWith("+++", Qt::CaseInsensitive)) {
        customCmd = "AT" + customCmd;
        sendATCommand(customCmd);
    }
    else {
        sendATCommand(customCmd);
    }

    ui->lineEditCustomAT->clear(); // 发送后清空
}

//=============================================================================
// 定时发送功能
//=============================================================================
void MainWindow::on_cbTimerSend_toggled(bool checked)
{
    if(checked) {
        if(!m_serialPort->isOpen()) {
            ui->cbTimerSend->setChecked(false);
            QMessageBox::warning(this, "警告", "请先打开串口！");
            return;
        }
        m_timer->start(ui->spinTimerPeriod->value());
        ui->statusbar->showMessage(QString("定时发送已开启，间隔 %1 ms").arg(ui->spinTimerPeriod->value()), 2000);
    } else {
        m_timer->stop();
        ui->statusbar->showMessage("定时发送已关闭", 2000);
    }
}

void MainWindow::on_spinTimerPeriod_valueChanged(int value)
{
    if(ui->cbTimerSend->isChecked()) {
        m_timer->setInterval(value);
        ui->statusbar->showMessage(QString("定时间隔已设为 %1 ms").arg(value), 1000);
    }
}

void MainWindow::timerSendData()
{
    on_btnSend_clicked();
}

//=============================================================================
// 其他控件槽函数
//=============================================================================
void MainWindow::on_cbAutoScroll_toggled(bool checked)
{
    m_autoScroll = checked;
    if(checked) {
        // 立即滚动到底部
        QTextCursor cursor = ui->textReceive->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->textReceive->setTextCursor(cursor);
    }
}

//=============================================================================
// 菜单操作
//=============================================================================
void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "关于",
        "串口调试助手 - AT指令版 V1.0\n\n"
        "功能特点：\n"
        "• 支持常用AT指令一键发送\n"
        "• 十六进制/ASCII显示切换\n"
        "• 定时发送功能\n"
        "• 自定义AT指令输入\n\n"
        "作者：hbchen\n"
        "版本：2026.03");
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "打开发送文件", "", "文本文件 (*.txt);;所有文件 (*)");

    if(!fileName.isEmpty()) {
        QFile file(fileName);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            ui->textSend->setPlainText(stream.readAll());
            file.close();
            ui->statusbar->showMessage(QString("已加载文件: %1").arg(fileName), 2000);
        } else {
            QMessageBox::warning(this, "错误", "无法打开文件！");
        }
    }
}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "保存接收数据", "", "文本文件 (*.txt);;所有文件 (*)");

    if(!fileName.isEmpty()) {
        QFile file(fileName);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << ui->textReceive->toPlainText();
            file.close();
            ui->statusbar->showMessage(QString("数据已保存到: %1").arg(fileName), 2000);
        } else {
            QMessageBox::warning(this, "错误", "无法保存文件！");
        }
    }
}
//=============================================================================
// 设定信息组相关函数
//=============================================================================

// 获取当前WiFi设置字符串
QString MainWindow::getCurrentWiFiSettings()
{
    QString ssid = ui->lineEditSSID->text().trimmed();
    QString password = ui->lineEditPassword->text().trimmed();

    if(ssid.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入WiFi SSID！");
        return QString();
    }

    return QString("AT+CWJAP=\"%1\",\"%2\"").arg(ssid).arg(password);
}

// 获取当前TCP设置字符串
QString MainWindow::getCurrentTCPSettings()
{
    QString ip = ui->lineEditIP->text().trimmed();
    int port = ui->spinPort->value();
    int connId = ui->comboConnID->currentText().toInt();
    QString type = ui->comboConnType->currentText();

    if(ip.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入IP地址！");
        return QString();
    }

    return QString("AT+CIPSTART=%1,\"%2\",\"%3\",%4")
            .arg(connId).arg(type).arg(ip).arg(port);
}

// 保存WiFi设置
void MainWindow::on_btnSaveWiFi_clicked()
{
    m_savedSSID = ui->lineEditSSID->text().trimmed();
    m_savedPassword = ui->lineEditPassword->text().trimmed();

    if(m_savedSSID.isEmpty()) {
        QMessageBox::warning(this, "警告", "WiFi SSID不能为空！");
        return;
    }

    ui->statusbar->showMessage(QString("WiFi设置已保存: %1").arg(m_savedSSID), 2000);
}

// 使用设置连接WiFi
void MainWindow::on_btnConnectWithSettings_clicked()
{
    QString cmd = getCurrentWiFiSettings();
    if(!cmd.isEmpty()) {
        sendATCommand(cmd);
    }
}

// 启用多连接
void MainWindow::on_btnEnableMux_clicked()
{
    sendATCommand("AT+CIPMUX=1");

    // 初始化连接状态显示
    for(int i = 0; i < 5; i++) {
        updateConnStatus(i, false);
    }
}

// 禁用多连接
void MainWindow::on_btnEnableSingle_clicked()
{
    sendATCommand("AT+CIPMUX=0");
}

// 保存TCP设置
void MainWindow::on_btnSaveTCP_clicked()
{
    m_savedIP = ui->lineEditIP->text().trimmed();
    m_savedPort = ui->spinPort->value();
    m_currentConnId = ui->comboConnID->currentText().toInt();

    if(m_savedIP.isEmpty()) {
        QMessageBox::warning(this, "警告", "IP地址不能为空！");
        return;
    }

    ui->statusbar->showMessage(QString("TCP设置已保存: %1:%2 (ID:%3)")
                               .arg(m_savedIP).arg(m_savedPort).arg(m_currentConnId), 2000);
}

// 建立TCP连接
void MainWindow::on_btnConnectTCP_clicked()
{
    QString ip = ui->lineEditIP->text().trimmed();
    int port = ui->spinPort->value();
    int connId = ui->comboConnID->currentText().toInt();
    QString type = ui->comboConnType->currentText();

    if(ip.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入IP地址！");
        return;
    }

    // 检查IP格式
    QRegularExpression ipRegex("^(\\d{1,3}\\.){3}\\d{1,3}$");
    if(!ipRegex.match(ip).hasMatch()) {
        QMessageBox::warning(this, "警告", "IP地址格式不正确！");
        return;
    }

    QString cmd = QString("AT+CIPSTART=%1,\"%2\",\"%3\",%4")
            .arg(connId).arg(type).arg(ip).arg(port);

    sendATCommand(cmd);

    // 保存连接信息
    ConnectionInfo info;
    info.isConnected = false;  // 等待连接成功响应后再更新
    info.ip = ip;
    info.port = port;
    info.type = type;
    info.remoteInfo = QString("%1:%2").arg(ip).arg(port);
    m_connections[connId] = info;

    ui->statusbar->showMessage(QString("正在连接 %1:%2 (ID:%3)...")
                               .arg(ip).arg(port).arg(connId), 2000);
}

// 关闭指定连接
void MainWindow::on_btnCloseConn_clicked()
{
    int connId = ui->comboConnID->currentText().toInt();
    QString cmd = QString("AT+CIPCLOSE=%1").arg(connId);
    sendATCommand(cmd);
}

// 连接ID改变
void MainWindow::on_comboConnID_currentIndexChanged(int index)
{
    m_currentConnId = index;

    // 如果有保存的设置，显示当前连接的状态
    if(m_connections.contains(index)) {
        ConnectionInfo info = m_connections[index];
        if(info.isConnected) {
            ui->statusbar->showMessage(QString("连接%1: %2:%3 (%4) - 已连接")
                                       .arg(index).arg(info.ip).arg(info.port).arg(info.type), 2000);
        }
    }
}

// 更新连接状态显示
void MainWindow::updateConnStatus(int id, bool connected, const QString &info)
{
    qDebug() << "更新连接状态: ID =" << id << " connected =" << connected << " info =" << info;

    QString statusText = connected ? "已连接" : "未连接";
    QString color = connected ? "green" : "red";

    switch(id) {
        case 0:
            ui->labelConn0Status->setText(statusText);
            ui->labelConn0Status->setStyleSheet(QString("color: %1; font-weight: %2;")
                                               .arg(color)
                                               .arg(connected ? "bold" : "normal"));
            if(!info.isEmpty()) ui->labelConn0Status->setToolTip(info);
            break;
        case 1:
            ui->labelConn1Status->setText(statusText);
            ui->labelConn1Status->setStyleSheet(QString("color: %1; font-weight: %2;")
                                               .arg(color)
                                               .arg(connected ? "bold" : "normal"));
            if(!info.isEmpty()) ui->labelConn1Status->setToolTip(info);
            break;
        case 2:
            ui->labelConn2Status->setText(statusText);
            ui->labelConn2Status->setStyleSheet(QString("color: %1; font-weight: %2;")
                                               .arg(color)
                                               .arg(connected ? "bold" : "normal"));
            if(!info.isEmpty()) ui->labelConn2Status->setToolTip(info);
            break;
        case 3:
            ui->labelConn3Status->setText(statusText);
            ui->labelConn3Status->setStyleSheet(QString("color: %1; font-weight: %2;")
                                               .arg(color)
                                               .arg(connected ? "bold" : "normal"));
            if(!info.isEmpty()) ui->labelConn3Status->setToolTip(info);
            break;
        case 4:
            ui->labelConn4Status->setText(statusText);
            ui->labelConn4Status->setStyleSheet(QString("color: %1; font-weight: %2;")
                                               .arg(color)
                                               .arg(connected ? "bold" : "normal"));
            if(!info.isEmpty()) ui->labelConn4Status->setToolTip(info);
            break;
        default:
            qDebug() << "未知的连接ID:" << id;
            return;
    }

    // 更新存储的连接状态
    ConnectionInfo connInfo;
    if(m_connections.contains(id)) {
        connInfo = m_connections[id];
    }

    connInfo.isConnected = connected;
    if(connected && !info.isEmpty()) {
        // 尝试从info中提取IP和端口信息
        QRegularExpression ipPortRegex("(\\d+\\.\\d+\\.\\d+\\.\\d+),?(\\d*)");
        QRegularExpressionMatch match = ipPortRegex.match(info);
        if(match.hasMatch()) {
            connInfo.ip = match.captured(1);
            if(match.captured(2).toInt() > 0) {
                connInfo.port = match.captured(2).toInt();
            }
        }
    }

    m_connections[id] = connInfo;

    // 如果当前选中的是这个ID，更新状态栏
    if(id == ui->comboConnID->currentIndex()) {
        if(connected) {
            ui->statusbar->showMessage(QString("连接%1当前状态: 已连接").arg(id), 2000);
        }
    }
}
