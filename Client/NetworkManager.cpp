#include "networkmanager.h"
#include <QDebug>

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
                qDebug() << "Received register response.";
                if (receivedPacket.success()) {
                    // 发射新的、带信息的信号
                    emit registrationResult(true, "新账号注册成功！");
                } else {
                    // 发射新的、带信息的信号
                    emit registrationResult(false, "注册失败，该用户名可能已被占用。");
                }
                break;

            // ... 处理其他类型的消息
            default:
                qDebug() << "Received unknown message type:" << static_cast<int>(receivedPacket.type());
                break;
            }
        }

        // 8. 从缓冲区中移除已处理的数据包
        m_buffer.remove(0, totalPacketSize);
    }
}

// 实现新的槽，负责发送注册请求
void NetworkManager::onRegistrationRequested(const QString& username, const QString& password)
{
    // 0. 检查网络连接状态
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Cannot send register request: not connected.";
        emit registrationResult(false, "注册失败：未连接到服务器。");
        return;
    }

    // 1. 数据转换和验证
    bool ok;
    uint8_t userId = username.toUInt(&ok);
    if (!ok || username.isEmpty()) {
        // 如果用户名不是纯数字或为空，就直接通知失败，不发送网络包
        emit registrationResult(false, "注册失败：用户名必须是0-255的数字。");
        return;
    }

    std::string pwdStr = password.toStdString();

    // 2. 封包
    Packet regPacket = Packet::makeCreAcc(userId, pwdStr);

    // 3. 发送
    if (regPacket.sendTo(m_socket)) {
        qDebug() << "Sent register request for user:" << userId;
        // 4. 启动超时定时器
        m_requestTimer->start(10000); // 10秒超时
    } else {
        qDebug() << "Failed to send register request.";
        emit registrationResult(false, "注册失败：数据发送异常。");
    }
}

// --- 实现新的槽函数 ---
void NetworkManager::onRequestTimeout()
{
    qDebug() << "Request timed out after 5 seconds.";
    emit requestTimeout(); // 发出超时信号，通知UI
}
