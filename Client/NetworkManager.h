#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>      // <--- 1. 添加 QTimer 头文件
#include "chatMsg.hpp" // <-- 改成这个正确的文件名
#include "mainwindow.h"
//static uint8_t selfId();
class NetworkManager : public QObject
{
    Q_OBJECT

public:
    // 单例模式
    static NetworkManager& instance();

    // --- 公共接口 (给UI调用) ---
    void connectToServer(const QString& host, quint16 port);
    void sendLoginRequest(uint8_t userId, const std::string& password);
    void sendRegisterRequest(uint8_t userId, const std::string& password);
    void sendAddFriendRequest(uint8_t selfId, uint8_t friendId);
    void sendAddFriendResponse(uint8_t originalRequesterId, uint8_t selfId, bool accepted);
    void sendMessage(uint8_t selfId, uint8_t targetId, const QString& text);
    // [新增] 发送群聊消息的公共接口
    void sendGroupMessage(uint8_t selfId, const QString& groupId, const QString& text);
    void sendCreateGroupRequest(const QString& groupName, const QVector<uint8_t>& memberIds);
    uint8_t selfId(); // 改为普通成员函数

signals:
    // --- 信号 (用来通知UI) ---
    void connected();
    void disconnected();
    void loginSuccess();
    void loginFailed();
    void requestTimeout();
    void registrationResult(bool success, const QString& message); // 保留一个即可
    void addFriendResult(bool success, uint8_t friendId);
    void autoAcceptFriendRequest(uint8_t requesterId);
    void newMessageReceived(const ChatMessage &message, const QString& conversationId);
    void createGroupResult(bool success, const QString& message);
    void addedToNewGroup(const QString& groupName, uint8_t creatorId, const QVector<uint8_t>& memberIds);

public slots:
    // --- 公共槽 (给其他类调用, 比如UI) ---
    void onRegistrationRequested(const QString& username, const QString& password);

private slots:
    // --- 私有槽 (只在类内部连接socket信号) ---
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onRequestTimeout();

private:
    // --- 私有构造/析构 ---
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    // 禁止拷贝和赋值
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    // --- 成员变量 ---
    QTcpSocket* m_socket;
    QByteArray m_buffer;
    QTimer* m_requestTimer;
};

#endif // NETWORKMANAGER_H
