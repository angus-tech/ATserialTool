#include "atcommandmanager.h"
#include <QRegularExpression>
#include <QDebug>

ATCommandManager::ATCommandManager(QObject *parent)
    : QObject(parent)
{
}

QString ATCommandManager::buildWiFiConnect(const QString &ssid, const QString &password)
{
    return QString("AT+CWJAP=\"%1\",\"%2\"").arg(ssid).arg(password);
}

QString ATCommandManager::cmdCIPMUX(bool enable) const
{
    return QString("AT+CIPMUX=%1").arg(enable ? "1" : "0");
}

QString ATCommandManager::cmdCIPMODE(bool transparent) const
{
    return QString("AT+CIPMODE=%1").arg(transparent ? "1" : "0");
}

QString ATCommandManager::buildCIPSTART(int connId, const QString &type,
                                       const QString &ip, int port)
{
    return QString("AT+CIPSTART=%1,\"%2\",\"%3\",%4")
            .arg(connId).arg(type).arg(ip).arg(port);
}

QString ATCommandManager::buildCIPSEND(int connId, int length)
{
    return QString("AT+CIPSEND=%1,%2").arg(connId).arg(length);
}

QString ATCommandManager::cmdCIPCLOSE(int connId) const
{
    return QString("AT+CIPCLOSE=%1").arg(connId);
}

ATCommandManager::ConnectResponse ATCommandManager::parseResponse(const QString &response)
{
    ConnectResponse result;
    result.success = false;
    result.connId = -1;

    // 检查WiFi连接
    if(isWiFiConnected(response)) {
        result.success = true;
        result.type = "WiFi";
        result.message = "WiFi Connected";
        return result;
    }

    // 检查TCP连接
    int connId = -1;
    if(isTCPConnected(response, connId)) {
        result.success = true;
        result.connId = connId;
        result.type = "TCP";
        result.message = "TCP Connected";
        return result;
    }

    return result;
}

bool ATCommandManager::isWiFiConnected(const QString &response) const
{
    return response.contains("WIFI CONNECTED") ||
           (response.contains("CONNECTED") && !response.contains(",CONNECT"));
}

bool ATCommandManager::isTCPConnected(const QString &response, int &connId) const
{
    QRegularExpression connRegex1("(\\d+),CONNECT\\b");
    QRegularExpression connRegex2("CONNECT\\s+(\\d+)\\b");

    QRegularExpressionMatch match;

    if(response.contains(connRegex1)) {
        match = connRegex1.match(response);
        if(match.hasMatch()) {
            connId = match.captured(1).toInt();
            return true;
        }
    }
    else if(response.contains(connRegex2)) {
        match = connRegex2.match(response);
        if(match.hasMatch()) {
            connId = match.captured(1).toInt();
            return true;
        }
    }
    else if(response.contains("\\bCONNECT\\b") && !response.contains("CONNECTED")) {
        connId = 0;
        return true;
    }

    return false;
}

bool ATCommandManager::isTCPClosed(const QString &response, int &connId) const
{
    QRegularExpression closeRegex1("(\\d+),CLOSED\\b");
    QRegularExpression closeRegex2("CLOSED\\s+(\\d+)\\b");

    QRegularExpressionMatch match;

    if(response.contains(closeRegex1)) {
        match = closeRegex1.match(response);
        if(match.hasMatch()) {
            connId = match.captured(1).toInt();
            return true;
        }
    }
    else if(response.contains(closeRegex2)) {
        match = closeRegex2.match(response);
        if(match.hasMatch()) {
            connId = match.captured(1).toInt();
            return true;
        }
    }
    else if(response.contains("\\bCLOSED\\b")) {
        connId = 0;
        return true;
    }

    return false;
}
