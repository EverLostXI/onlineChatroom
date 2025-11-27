#include "networkmanager.h"
#include <QDebug>
#include "mainwindow.h"

// --- 单例实现 ---
NetworkManager& NetworkManager::instance()
{
    static NetworkManager instance;
    return instance;
}

// --- 构造与析构 ---
NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
{
    m_socket = new QTcpSocket(this);

    // 连接socket的信号到我们的槽
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);

    // === 新增代码：初始化定时器 ===
    m_requestTimer = new QTimer(this);
    m_requestTimer->setSingleShot(true); // 设置为单次触发定时器
    connect(m_requestTimer, &QTimer::timeout, this, &NetworkManager::onRequestTimeout);
}

NetworkManager::~NetworkManager()
{
    // m_socket 会因为是this的子对象而被Qt自动删除
}

// --- 公共接口实现 ---
void NetworkManager::connectToServer(const QString& host, quint16 port)
{
    if (m_socket->state() == QAbstractSocket::UnconnectedState) {
        qDebug() << "Connecting to server:" << host << port;
        m_socket->connectToHost(host, port);
    }
}

void NetworkManager::sendLoginRequest(uint8_t userId, const std::string& password)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Cannot send login request: not connected.";
        // 可以在这里发出一个错误信号
        return;
    }
    Packet p = Packet::makeLogin(userId, password);
    p.sendTo(m_socket);
    qDebug() << "Sent login request for user:" << userId;

    // === 新增代码：启动5秒超时定时器 ===
    m_requestTimer->start(10000); // 5000毫秒 = 5秒
}

void NetworkManager::sendRegisterRequest(uint8_t userId, const std::string& password)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Cannot send register request: not connected.";
        return;
    }
    Packet p = Packet::makeCreAcc(userId, password);
    p.sendTo(m_socket);
    qDebug() << "Sent register request for user:" << userId;

    // === 新增代码：启动5秒超时定时器 ===
    m_requestTimer->start(10000);
}

// [新增] 实现发送添加好友请求的函数
void NetworkManager::sendAddFriendRequest(uint8_t selfId, uint8_t friendId)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Cannot send add friend request: not connected.";
        // 直接通过信号反馈失败
        emit addFriendResult(false, friendId);
        return;
    }
    Packet p = Packet::makeAddFriend(selfId, friendId);
    p.sendTo(m_socket);
    qDebug() << "Sent add friend request from" << selfId << "to" << friendId;

    // 同样可以为这个请求启动超时定时器
    m_requestTimer->start(10000); // 10秒超时
}

// --- 槽函数实现 ---
void NetworkManager::onConnected()
{
    qDebug() << "Successfully connected to server.";
    emit connected(); // 发出连接成功信号
}

void NetworkManager::onDisconnected()
{
    qDebug() << "Disconnected from server.";
    emit disconnected(); // 发出断开连接信号
}

void NetworkManager::onReadyRead()
{

    //下一条为测试用
    qDebug() << "[Log 1] onReadyRead triggered! New data available.";


    // 1. 将所有可读数据追加到缓冲区
    m_buffer.append(m_socket->readAll());

    //下一条为测试用
    qDebug() << "[Log 2] onReadyRead triggered! New data available.";

    // 2. 循环处理缓冲区中的数据包
    while (true) {
        // 3. 检查缓冲区数据是否足够一个包头，如果不够就退出循环等待更多数据
        if (m_buffer.size() < sizeof(Header)) {
            break;
        }

        // 4. "窥探"包头以获取整个包的大小，但不立即解析
        Header peekHeader;
        memcpy(&peekHeader, m_buffer.constData(), sizeof(Header));
        size_t bodySize = n2h16(peekHeader.field1Len) + n2h16(peekHeader.field2Len) +
                          n2h16(peekHeader.field3Len) + n2h16(peekHeader.field4Len);
        size_t totalPacketSize = sizeof(Header) + bodySize;

        // 5. 检查缓冲区数据是否足够一个完整的包
        //    使用 static_cast 消除 signed/unsigned 比较的警告
        if (m_buffer.size() < static_cast<qsizetype>(totalPacketSize)) {
            break; // 数据不完整，等待下一次 onReadyRead
        }

        //下一条为测试用
        qDebug() << "[Log 3] onReadyRead triggered! New data available.";

        // 6. 数据完整，我们调用Packet自己的方法来处理
        Packet receivedPacket;
        // 让 packet 对象自己从缓冲区解析数据
        if (receivedPacket.parseFrom(m_buffer.constData(), totalPacketSize))
        {
            // === 新增代码：只要收到任何一个完整的包，就停止超时定时器 ===
            m_requestTimer->stop();

            // 7. 解析成功，根据包类型发出相应的信号

            //下一条为测试用
            qDebug() << "[Log 4] onReadyRead triggered! New data available.";

            switch (receivedPacket.type()) {
            case MsgType::Loginreturn:
                qDebug() << "Received login response.";
                if (receivedPacket.success()) {
                    emit loginSuccess();
                } else {
                    emit loginFailed();
                }
                break;

            case MsgType::regireturn:
                qDebug() << "Received registration response.";
                if (receivedPacket.success()) {
                    emit registrationResult(true, "新账号注册成功！");
                } else {
                    emit registrationResult(false, "注册失败：用户名可能已被占用。");
                }
                break;

            // ... 处理其他类型的消息

                // [新增] 处理添加好友的反馈
            case MsgType::AddFriendRe:
            {
                qDebug() << "Received add friend response.";
                // 根据你的协议，响应包的 sendid 是最初发起请求的人，recvid 是被添加的人
                // 但为了UI方便，我们更关心是哪个好友的请求有了结果，这里我们用recvid
                // **注意**：请根据你的服务器实现来确定哪个字段是目标好友ID。
                // 我这里的代码假设 recvid 是我们尝试添加的好友ID。
                // 根据你的截图，p.hdr.recvid 是被添加者，p.hdr.sendid是请求者。
                // 所以，响应包里，recvid 应该是我们自己，sendid 才是我们添加的好友。
                uint8_t friendId = receivedPacket.getsendid();
                emit addFriendResult(receivedPacket.success(), friendId);
                break;
            }

            case MsgType::AddFriendReq: // [新增] 处理收到的好友请求
            {
                // 假设 selfId() 能获取当前登录用户的ID
                if (receivedPacket.getrecvid() == selfId()) {
                    // 这是别人发给我的好友请求
                    uint8_t requesterId = receivedPacket.getsendid();
                    qDebug() << "[NetworkManager] Received and auto-accepting friend request from ID:" << requesterId;

                    // 发射一个新信号，通知 MainWindow 自动处理
                    emit autoAcceptFriendRequest(requesterId);
                }
                // 如果 recvid 不是自己，那就忽略这个包（理论上不应该发生）
                break;
            }

                // ↓↓↓ 添加这个处理聊天消息的新 CASE ↓↓↓
            case MsgType::NormalMsg:
            {
                uint8_t senderId = receivedPacket.getsendid();
                uint8_t targetId = receivedPacket.getrecvid(); // 这个ID要么是你自己，要么是一个群ID
                // 根据你的 packet.h, 消息内容在 field1
                QString content = QString::fromStdString(receivedPacket.getField1Str());

                ChatMessage msg;
                msg.senderId = senderId;
                msg.text = content;
                msg.timestamp = QDateTime::currentDateTime();

                // 为UI确定正确的对话ID (conversationId)
                // 这个逻辑假设你的服务器发送群消息时 recvid = 群ID，
                // 发送私聊消息时 recvid = 你的ID。
                int conversationId;
                if (targetId == selfId()) {
                    // 这是发给我的私聊消息。对话是和发送者(sender)的。
                    conversationId = senderId;
                } else {
                    // 这很可能是一条群消息。对话是和这个群(target)的。
                    conversationId = targetId;
                }

                qDebug() << "收到 NormalMsg. 对话ID:" << conversationId << "发送者:" << senderId << "内容:" << content;
                emit newMessageReceived(msg, conversationId);
                break;
            }


            default:
                qDebug() << "Received unknown message type:" << static_cast<int>(receivedPacket.type());
                break;
            }
        }

        // 8. 从缓冲区中移除已处理的数据包
        m_buffer.remove(0, totalPacketSize);
    }

}

// --- 实现新的槽函数 ---
void NetworkManager::onRequestTimeout()
{
    qDebug() << "Request timed out after 5 seconds.";
    emit requestTimeout(); // 发出超时信号，通知UI
}

void NetworkManager::onRegistrationRequested(const QString& username, const QString& password)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit registrationResult(false, "注册失败：未连接到服务器。");
        return;
    }

    bool ok;
    uint8_t userId = username.toUInt(&ok);
    if (!ok) {
        emit registrationResult(false, "注册失败：用户名必须是纯数字。");
        return;
    }

    std::string pwdStr = password.toStdString();
    Packet regPacket = Packet::makeCreAcc(userId, pwdStr);

    if (regPacket.sendTo(m_socket)) {
        qDebug() << "Sent registration request for user:" << userId;
        // 可以在这里启动一个超时定时器
    } else {
        emit registrationResult(false, "注册失败：网络发送异常。");
    }
}

// [新增] sendAddFriendResponse() 函数的实现
void NetworkManager::sendAddFriendResponse(uint8_t originalRequesterId, uint8_t selfId, bool accepted)
{
    // 构造 AddFriendRe 包
    // 注意：这里的 sendid 是最初发起请求的人，recvid 是你自己
    Packet responsePacket = Packet::makeAddFriendRe(originalRequesterId, selfId, accepted);
    responsePacket.sendTo(m_socket);
    qDebug() << "[NetworkManager] Sent auto-accepted friend response for requester" << originalRequesterId;
}


// [新增] selfId() 函数的实现
uint8_t NetworkManager::selfId()
{
    // 目前硬编码返回用户ID为 1
    // TODO: 将来这里需要替换为从登录信息中获取的真实ID
    return 1;
}

void NetworkManager::sendMessage(uint8_t selfId, uint8_t targetId, const QString& text)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "无法发送消息：未连接到服务器。";
        return;
    }

    // 使用你已有的 Packet::Message 函数
    // 注意：你的函数需要 std::string，所以我们用 toStdString() 转换
    // 最后一个时间戳参数暂时留空
    Packet p = Packet::Message(selfId, targetId, text.toStdString(), "");

    qDebug() << "正在发送 NormalMsg 从" << selfId << "到目标" << targetId;
    p.sendTo(m_socket);
}
