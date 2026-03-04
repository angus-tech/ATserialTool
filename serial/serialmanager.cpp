#include "serialmanager.h"
#include <QDebug>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
{
    connect(m_serialPort, &QSerialPort::readyRead,
            this, &SerialManager::handleReadyRead);
}

SerialManager::~SerialManager()
{
    if(m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

bool SerialManager::openPort(const QString &portName, qint32 baudRate)
{
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setParity(QSerialPort::NoParity);

    bool success = m_serialPort->open(QIODevice::ReadWrite);
    if(success) {
        emit portStatusChanged(true);
    }
    return success;
}

void SerialManager::closePort()
{
    if(m_serialPort->isOpen()) {
        m_serialPort->close();
        emit portStatusChanged(false);
    }
}

QStringList SerialManager::getAvailablePorts()
{
    QStringList ports;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ports << info.portName();
    }
    return ports;
}

qint64 SerialManager::writeData(const QByteArray &data)
{
    return m_serialPort->write(data);
}

qint64 SerialManager::writeCommand(const QString &command)
{
    QString cmd = command;
    if(cmd != "+++" && !cmd.endsWith("\n")) {
        cmd += "\r\n";
    }

    QByteArray data = cmd.toLocal8Bit();
    qint64 bytes = m_serialPort->write(data);

    if(bytes > 0) {
        emit commandSent(command, bytes);
    }

    return bytes;
}

void SerialManager::handleReadyRead()
{
    QByteArray data = m_serialPort->readAll();
    emit dataReceived(data);
}
