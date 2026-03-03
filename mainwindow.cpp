#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serialPort(new QSerialPort(this))
{
    ui->setupUi(this);

    // 初始化波特率
    ui->cbBaudRate->addItems(QStringList()
        << "9600" << "115200");
    ui->cbBaudRate->setCurrentText("115200");

    // 刷新串口列表
    refreshPortList();

    // 连接信号
    connect(m_serialPort, &QSerialPort::readyRead,
            this, &MainWindow::readSerialData);
}

MainWindow::~MainWindow()
{
    if(m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    delete ui;
}

void MainWindow::refreshPortList()
{
    ui->cbPortName->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->cbPortName->addItem(info.portName());
    }
}

void MainWindow::on_btnRefresh_clicked()
{
    refreshPortList();
}

void MainWindow::on_btnOpenPort_clicked()
{
    if(m_serialPort->isOpen()) {
        m_serialPort->close();
        updatePortStatus(false);
    } else {
        m_serialPort->setPortName(ui->cbPortName->currentText());
        m_serialPort->setBaudRate(ui->cbBaudRate->currentText().toInt());

        if(m_serialPort->open(QIODevice::ReadWrite)) {
            updatePortStatus(true);
        } else {
            QMessageBox::critical(this, "错误", "无法打开串口！");
        }
    }
}

void MainWindow::readSerialData()
{
    QByteArray data = m_serialPort->readAll();

    if(ui->cbHexDisplay->isChecked()) {
        QString hexData = data.toHex(' ').toUpper();
        ui->textReceive->append(hexData);
    } else {
        ui->textReceive->append(QString::fromLocal8Bit(data));
    }
}

void MainWindow::on_btnSend_clicked()
{
    if(!m_serialPort->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口！");
        return;
    }

    QByteArray data;
    if(ui->cbHexSend->isChecked()) {
        QString text = ui->textSend->toPlainText();
        text.remove(' ');
        data = QByteArray::fromHex(text.toLocal8Bit());
    } else {
        data = ui->textSend->toPlainText().toLocal8Bit();
    }

    m_serialPort->write(data);
}

void MainWindow::on_btnClearReceive_clicked()
{
    ui->textReceive->clear();
}

void MainWindow::on_btnClearSend_clicked()
{
    ui->textSend->clear();
}

void MainWindow::updatePortStatus(bool isOpen)
{
    if(isOpen) {
        ui->btnOpenPort->setText("关闭串口");
        ui->cbPortName->setEnabled(false);
        ui->cbBaudRate->setEnabled(false);
        ui->btnRefresh->setEnabled(false);
    } else {
        ui->btnOpenPort->setText("打开串口");
        ui->cbPortName->setEnabled(true);
        ui->cbBaudRate->setEnabled(true);
        ui->btnRefresh->setEnabled(true);
    }
}
