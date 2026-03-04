#ifndef ATCOMMANDMANAGER_H
#define ATCOMMANDMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>

class ATCommandManager : public QObject
{
    Q_OBJECT

public:
    explicit ATCommandManager(QObject *parent = nullptr);

    // 基础指令
    QString cmdAT() const { return "AT"; }
    QString cmdGMR() const { return "AT+GMR"; }
    QString cmdATE0() const { return "ATE0"; }
    QString cmdATE1() const { return "ATE1"; }
    QString cmdRST() const { return "AT+RST"; }
    QString cmdCWLIF() const { return "AT+CWLIF"; }
    QString cmdCWJAP() const { return "AT+CWJAP?"; }
    QString cmdCIFSR() const { return "AT+CIFSR"; }

    // WiFi模式
    QString cmdModeAP() const { return "AT+CWMODE=2"; }
    QString cmdModeSTA() const { return "AT+CWMODE=1"; }
    QString cmdModeAPSTA() const { return "AT+CWMODE=3"; }

    // WiFi连接
    QString buildWiFiConnect(const QString &ssid, const QString &password);

    // TCP/IP指令
    QString cmdCIPMUX(bool enable) const;
    QString cmdCIPMODE(bool transparent) const;
    QString buildCIPSTART(int connId, const QString &type,
                         const QString &ip, int port);
    QString buildCIPSEND(int connId, int length);
    QString cmdCIPCLOSE(int connId) const;
    QString cmdExitSend() const { return "+++"; }

    // 响应解析
    struct ConnectResponse {
        bool success;
        int connId;
        QString message;
        QString type;  // "TCP", "UDP", "WiFi"
    };

    ConnectResponse parseResponse(const QString &response);
    bool isWiFiConnected(const QString &response) const;
    bool isTCPConnected(const QString &response, int &connId) const;
    bool isTCPClosed(const QString &response, int &connId) const;

private:
    QMap<QString, QString> m_commandMap;
};

#endif // ATCOMMANDMANAGER_H
