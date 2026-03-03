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
        << "1200" << "2400" << "4800" << "9600"
        << "19200" << "38400" << "57600" << "115200"
        << "230400" << "460800" << "921600");
    ui->cbBaudRate->setCurrentText("115200");

    // 初始化数据位
    ui->cbDataBits->addItems(QStringList() << "5" << "6" << "7" << "8");
    ui->cbDataBits->setCurrentText("8");

    // 初始化停止位
    ui->cbStopBits->addItems(QStringList() << "1" << "1.5" << "2");
    ui->cbStopBits->setCurrentText("1");

    // 初始化校验位
    ui->cbParity->addItems(QStringList() << "无" << "奇校验" << "偶校验" << "标志校验" << "空白校验");

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
        ui->cbDataBits->setEnabled(false);
        ui->cbStopBits->setEnabled(false);
        ui->cbParity->setEnabled(false);
        ui->btnRefresh->setEnabled(false);

        // 启用AT指令按钮
        ui->groupBoxAT->setEnabled(true);

        // 清空数据缓冲区
        m_dataBuffer.clear();
        if (m_dataTimer->isActive()) {
            m_dataTimer->stop();
        }
    } else {
        ui->btnOpenPort->setText("打开串口");
        ui->cbPortName->setEnabled(true);
        ui->cbBaudRate->setEnabled(true);
        ui->cbDataBits->setEnabled(true);
        ui->cbStopBits->setEnabled(true);
        ui->cbParity->setEnabled(true);
        ui->btnRefresh->setEnabled(true);

        // 禁用AT指令按钮
        ui->groupBoxAT->setEnabled(false);

        // 如果定时发送开启，关闭它
        if(ui->cbTimerSend->isChecked()) {
            ui->cbTimerSend->setChecked(false);
        }

        // 清空数据缓冲区
        m_dataBuffer.clear();
        if (m_dataTimer->isActive()) {
            m_dataTimer->stop();
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
    , m_dataTimer(new QTimer(this))  // 新增
    , m_isReceiving(false)            // 新增
    , m_lastReceiveTime(0)            // 新增
    , m_autoScroll(true)
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
        m_dataTimer->setSingleShot(true);  // 设置为单次触发
        m_dataTimer->setInterval(10);      // 设置10ms超时

    // 连接自定义AT指令输入框的回车信号
    connect(ui->lineEditCustomAT, &QLineEdit::returnPressed,
            this, &MainWindow::on_btnSendCustom_clicked);

    // 初始禁用AT指令按钮（直到串口打开）
    ui->groupBoxAT->setEnabled(false);
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
        switch(ui->cbDataBits->currentText().toInt()) {
            case 5: m_serialPort->setDataBits(QSerialPort::Data5); break;
            case 6: m_serialPort->setDataBits(QSerialPort::Data6); break;
            case 7: m_serialPort->setDataBits(QSerialPort::Data7); break;
            case 8: m_serialPort->setDataBits(QSerialPort::Data8); break;
            default: m_serialPort->setDataBits(QSerialPort::Data8); break;
        }

        // 设置停止位
        if(ui->cbStopBits->currentText() == "1") {
            m_serialPort->setStopBits(QSerialPort::OneStop);
        } else if(ui->cbStopBits->currentText() == "1.5") {
            m_serialPort->setStopBits(QSerialPort::OneAndHalfStop);
        } else if(ui->cbStopBits->currentText() == "2") {
            m_serialPort->setStopBits(QSerialPort::TwoStop);
        }

        // 设置校验位
        switch(ui->cbParity->currentIndex()) {
            case 0: m_serialPort->setParity(QSerialPort::NoParity); break;
            case 1: m_serialPort->setParity(QSerialPort::OddParity); break;
            case 2: m_serialPort->setParity(QSerialPort::EvenParity); break;
            case 3: m_serialPort->setParity(QSerialPort::MarkParity); break;
            case 4: m_serialPort->setParity(QSerialPort::SpaceParity); break;
            default: m_serialPort->setParity(QSerialPort::NoParity); break;
        }

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
void MainWindow::on_btnATRST_clicked() { sendATCommand("ATRST"); }
void MainWindow::on_btnATCWLIF_clicked() { sendATCommand("AT+CWLIF"); }
void MainWindow::on_btnATCIPMUX_clicked() { sendATCommand("AT+CIPMUX=0"); }     // 0:单连接    1:多连接
void MainWindow::on_btnATCIPMODE_clicked() { sendATCommand("AT+CIPMODE=1"); }   // 0:普通模式  1:透传模式

//=============================================================================
// AT指令槽函数 - 设备信息
//=============================================================================
void MainWindow::on_btnATCWMODESTA_clicked() { sendATCommand("AT+CWMODE=1"); }
void MainWindow::on_btnATCWMODEAP_clicked() { sendATCommand("AT+CWMODE=2"); }
void MainWindow::on_btnATCWMODEAPSTA_clicked() { sendATCommand("AT+CWMODE=3"); }

//=============================================================================
// AT指令槽函数 - 网络状态
//=============================================================================
void MainWindow::on_btnATCWLAP_clicked() { sendATCommand("AT+CWLAP"); }
void MainWindow::on_btnATCWJAPwifi_clicked() { sendATCommand("AT+CWJAP=\"Sheentec_work\",\"sheentec\""); }
void MainWindow::on_btnATCIFSR_clicked() { sendATCommand("AT+CIFSR"); }

//=============================================================================
// AT指令槽函数 - 通讯测试
//=============================================================================
void MainWindow::on_btnATCIPSTART_clicked() { sendATCommand("AT+CIPSTART=\"TCP\",\"192.168.20.2\",8888"); }
void MainWindow::on_btnATCIPSEND_clicked() { sendATCommand("AT+CIPSEND"); }
void MainWindow::on_btnATCIPEXIT_clicked() { sendATCommand("+++"); }

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

    // 自动添加AT前缀（如果没有）
    if(!customCmd.startsWith("AT", Qt::CaseInsensitive)) {
        customCmd = "AT+" + customCmd;
    }

    sendATCommand(customCmd);
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
        "作者：Your Name\n"
        "版本：2024.01");
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
