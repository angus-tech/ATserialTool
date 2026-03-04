#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <QObject>
#include <QFont>
#include <QDateTime>
#include <QByteArray>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class UIManager : public QObject
{
    Q_OBJECT

public:
    explicit UIManager(QObject *parent = nullptr);

    // 初始化
    void initUI(Ui::MainWindow *ui);

    // 串口UI
    void refreshPortList(Ui::MainWindow *ui, const QStringList &ports);
    void updatePortStatus(Ui::MainWindow *ui, bool isOpen);

    // 数据显示
    void appendData(Ui::MainWindow *ui, const QString &data,
                    const QString &timestamp, bool isTx = false);
    void appendReceiveData(Ui::MainWindow *ui, const QByteArray &data,
                          bool hexDisplay, bool autoScroll);

    // 格式化函数 - 公有，供外部使用
    QString formatHexData(const QByteArray &data);
    QString formatTextData(const QByteArray &data);

    // 连接状态显示
    void updateConnStatus(Ui::MainWindow *ui, int id, bool connected,
                         const QString &info = "");

    // 状态栏
    void showStatusMessage(Ui::MainWindow *ui, const QString &msg, int timeout = 2000);

    // 获取设置
    QString getWiFiSettings(Ui::MainWindow *ui, bool showWarning = true);
    QString getTCPSettings(Ui::MainWindow *ui, bool showWarning = true);

    // IP地址验证
    bool validateIPAddress(const QString &ip, QString *errorMsg = nullptr);
};

#endif // UIMANAGER_H
