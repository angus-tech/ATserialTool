#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QDateTime>  // 添加这行，为 connectTime 提供完整类型

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);

    // 连接信息结构
    struct ConnectionInfo {
        bool isConnected = false;
        QString ip;
        int port = 0;
        QString type;
        QString remoteInfo;
        QDateTime connectTime;
    };

    // WiFi设置
    void saveWiFiSettings(const QString &ssid, const QString &password);
    QString getSavedSSID() const { return m_savedSSID; }
    QString getSavedPassword() const { return m_savedPassword; }

    // TCP连接管理
    void startConnection(int connId, const QString &ip, int port, const QString &type);
    void updateConnectionStatus(int connId, bool connected, const QString &info = "");
    void closeConnection(int connId);
    ConnectionInfo getConnectionInfo(int connId) const;
    bool isWaitingForConnection() const { return m_waitingForConnection; }
    int getPendingConnId() const { return m_pendingConnId; }

    // 设置管理
    void saveTCPSettings(int connId, const QString &ip, int port);
    void loadTCPSettings(int connId, QString &ip, int &port) const;

signals:
    void connectionStatusChanged(int connId, bool connected, const QString &info);
    void connectionTimeout(int connId);
    void wifiSettingsChanged(const QString &ssid);

private slots:
    void handleConnectionTimeout();

private:
    QMap<int, ConnectionInfo> m_connections;

    // 等待连接状态
    bool m_waitingForConnection = false;
    int m_pendingConnId = -1;
    QString m_pendingConnInfo;
    QTimer *m_timeoutTimer;

    // 保存的设置
    QString m_savedSSID;
    QString m_savedPassword;
    QMap<int, QPair<QString, int>> m_savedTCPSettings;
};

#endif // NETWORKMANAGER_H
