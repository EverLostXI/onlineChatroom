#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>      // <--- 1. 添加 QTimer 头文件
#include "chatMsg.hpp" // <-- 改成这个正确的文件名
#include "mainwindow.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    // 单例模式：获取全局唯一的实例
    static NetworkManager& instance();

    // 公共接口：给UI调用
    void connectToServer(const QString& host, quint16 port);
    void sendLoginRequest(uint8_t userId, const std::string& password);
    void sendRegisterRequest(uint8_t userId, const std::string& password);

    // [新增] 发送添加好友请求的公共接口
    void sendAddFriendRequest(uint8_t selfId, uint8_t friendId);
    // [新增] 添加下面这一行，你的 MainWindow.cpp 正是想调用它！
    void sendAddFriendResponse(uint8_t originalRequesterId, uint8_t selfId, bool accepted);

    void sendMessage(uint8_t selfId, uint8_t targetId, const QString& text);
    static uint8_t selfId();

signals:
    // 信号：通知UI网络事件的结果
    void connected();
    void disconnected();
    void loginSuccess();
    void loginFailed();
    void registrationSuccess();
    void registrationFailed();
    // [新增] 添加好友结果的信号
    // 参数: success - 是否成功, friendId - 尝试添加的好友ID
    void addFriendResult(bool success, uint8_t friendId);
    void autoAcceptFriendRequest(uint8_t requesterId); // [新增] 自动同意好友请求信号

    void newMessageReceived(const ChatMessage &message, int conversationId);
    void requestTimeout(); // <--- 2. 添加一个新的信号，用于通知UI请求超时


private slots:
    // 槽：处理QTcpSocket的内部信号
    void onConnected();
    void onDisconnected();
    void onReadyRead(); // 核心：当收到服务器数据时被调用

    void onRequestTimeout(); // <--- 3. 添加一个新的槽函数，用于处理定时器超时事件


public slots:
    // 添加一个新的槽，用来接收UI的注册请求
    void onRegistrationRequested(const QString& username, const QString& password);

signals:
    // ...
    // 添加一个新的信号，用来通知整个应用注册的结果
    void registrationResult(bool success, const QString& message);

private:
    // 私有构造函数，确保外部不能直接创建
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    // 禁止拷贝和赋值
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    QTcpSocket* m_socket; // 指向我们的TCP socket
    QByteArray m_buffer;  // 用于处理粘包、半包问题的缓冲区

    QTimer* m_requestTimer; // <--- 4. 添加一个 QTimer 指针成员
};

#endif // NETWORKMANAGER_H
