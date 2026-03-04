#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager();

    // 串口操作
    bool openPort(const QString &portName, qint32 baudRate);
    void closePort();
    bool isOpen() const { return m_serialPort->isOpen(); }
    QString getLastError() const { return m_serialPort->errorString(); }

    // 串口信息
    QStringList getAvailablePorts();

    // 数据发送
    qint64 writeData(const QByteArray &data);
    qint64 writeCommand(const QString &command);

signals:
    void dataReceived(const QByteArray &data);
    void portStatusChanged(bool isOpen);
    void commandSent(const QString &command, qint64 bytes);

private slots:
    void handleReadyRead();

private:
    QSerialPort *m_serialPort;
};

#endif // SERIALMANAGER_H
