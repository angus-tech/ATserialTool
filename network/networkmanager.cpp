#include "networkmanager.h"
#include <QDateTime>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(5000); // 5秒超时

    connect(m_timeoutTimer, &QTimer::timeout,
            this, &NetworkManager::handleConnectionTimeout);
}

void NetworkManager::saveWiFiSettings(const QString &ssid, const QString &password)
{
    m_savedSSID = ssid;
    m_savedPassword = password;
    emit wifiSettingsChanged(ssid);
}

void NetworkManager::startConnection(int connId, const QString &ip, int port, const QString &type)
{
    m_waitingForConnection = true;
    m_pendingConnId = connId;
    m_pendingConnInfo = QString("%1:%2 (%3)").arg(ip).arg(port).arg(type);

    // 保存连接信息
    ConnectionInfo info;
    info.isConnected = false;
    info.ip = ip;
    info.port = port;
    info.type = type;
    info.remoteInfo = QString("%1:%2").arg(ip).arg(port);
    info.connectTime = QDateTime::currentDateTime();
    m_connections[connId] = info;

    // 启动超时定时器
    m_timeoutTimer->start();
}

void NetworkManager::updateConnectionStatus(int connId, bool connected, const QString &info)
{
    if(m_connections.contains(connId)) {
        m_connections[connId].isConnected = connected;
        if(connected && !info.isEmpty()) {
            m_connections[connId].remoteInfo = info;
        }
    }

    emit connectionStatusChanged(connId, connected, info);

    // 如果是正在等待的连接，清除等待状态
    if(connected && connId == m_pendingConnId) {
        m_waitingForConnection = false;
        m_pendingConnId = -1;
        m_pendingConnInfo.clear();
        m_timeoutTimer->stop();
    }
}

void NetworkManager::closeConnection(int connId)
{
    if(m_connections.contains(connId)) {
        m_connections[connId].isConnected = false;
    }
    emit connectionStatusChanged(connId, false, "已关闭");
}

NetworkManager::ConnectionInfo NetworkManager::getConnectionInfo(int connId) const
{
    if(m_connections.contains(connId)) {
        return m_connections[connId];
    }
    return ConnectionInfo();
}

void NetworkManager::saveTCPSettings(int connId, const QString &ip, int port)
{
    m_savedTCPSettings[connId] = qMakePair(ip, port);
}

void NetworkManager::loadTCPSettings(int connId, QString &ip, int &port) const
{
    if(m_savedTCPSettings.contains(connId)) {
        ip = m_savedTCPSettings[connId].first;
        port = m_savedTCPSettings[connId].second;
    }
}

void NetworkManager::handleConnectionTimeout()
{
    if(m_waitingForConnection) {
        emit connectionTimeout(m_pendingConnId);
        m_waitingForConnection = false;
        m_pendingConnId = -1;
        m_pendingConnInfo.clear();
    }
}
