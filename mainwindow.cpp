#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QScrollBar>
#include <QDateTime>
#include <cctype>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serialMgr(new SerialManager(this))
    , m_atCmdMgr(new ATCommandManager(this))
    , m_networkMgr(new NetworkManager(this))
    , m_uiMgr(new UIManager(this))
    , m_actionHandler(new UIActionHandler(ui, m_serialMgr, m_atCmdMgr,
                                          m_networkMgr, m_uiMgr, this))
    , m_dataTimer(new QTimer(this))
    , m_autoScroll(true)
{
    ui->setupUi(this);

    // 初始化UI
    m_uiMgr->initUI(ui);

    // 刷新串口列表
    on_btnRefresh_clicked();

    // 设置信号连接
    setupConnections();

    // 初始状态
    m_uiMgr->updatePortStatus(ui, false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    // 串口信号
    connect(m_serialMgr, &SerialManager::dataReceived,
            this, &MainWindow::handleSerialData);
    connect(m_serialMgr, &SerialManager::portStatusChanged,
            this, &MainWindow::handlePortStatusChanged);

    // 网络信号
    connect(m_networkMgr, &NetworkManager::connectionStatusChanged,
            this, &MainWindow::handleConnectionStatusChanged);
    connect(m_networkMgr, &NetworkManager::connectionTimeout,
            this, &MainWindow::handleConnectionTimeout);

    // 数据定时器
    connect(m_dataTimer, &QTimer::timeout,
            this, &MainWindow::processBufferedData);
    m_dataTimer->setSingleShot(true);
    m_dataTimer->setInterval(10);

    // 自定义指令回车
    connect(ui->lineEditCustomAT, &QLineEdit::returnPressed,
            this, &MainWindow::on_btnSendCustom_clicked);
}

void MainWindow::handleSerialData(const QByteArray &data)
{
    m_dataBuffer.append(data);

    // 解析响应
    QString response = QString::fromLocal8Bit(data);
    ATCommandManager::ConnectResponse resp = m_atCmdMgr->parseResponse(response);

    if(resp.success) {
        if(resp.type == "TCP") {
            m_networkMgr->updateConnectionStatus(resp.connId, true, response);
        } else if(resp.type == "WiFi") {
            m_uiMgr->showStatusMessage(ui, "WiFi已连接");
        }
    } else {
        // 检查关闭连接
        int connId;
        if(m_atCmdMgr->isTCPClosed(response, connId)) {
            m_networkMgr->updateConnectionStatus(connId, false, "已关闭");
        }
    }

    // 启动数据定时器
    if(!m_dataTimer->isActive()) {
        m_dataTimer->start();
    } else {
        m_dataTimer->stop();
        m_dataTimer->start();
    }
}

void MainWindow::handlePortStatusChanged(bool isOpen)
{
    m_uiMgr->updatePortStatus(ui, isOpen);
}

void MainWindow::handleConnectionStatusChanged(int connId, bool connected, const QString &info)
{
    m_uiMgr->updateConnStatus(ui, connId, connected, info);
    if(connected) {
        m_uiMgr->showStatusMessage(ui, QString("TCP连接%1已建立").arg(connId));
    } else {
        m_uiMgr->showStatusMessage(ui, QString("TCP连接%1已关闭").arg(connId));
    }
}

void MainWindow::handleConnectionTimeout(int connId)
{
    m_uiMgr->showStatusMessage(ui, QString("连接%1超时").arg(connId));
}

void MainWindow::processBufferedData()
{
    if(m_dataBuffer.isEmpty()) return;

    m_uiMgr->appendReceiveData(ui, m_dataBuffer,
                               ui->cbHexDisplay->isChecked(),
                               ui->cbAutoScroll->isChecked());
    m_dataBuffer.clear();
}
