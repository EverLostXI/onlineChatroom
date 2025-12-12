#include "headers/handleClient.h"
#include "headers/socket.h"
#include "headers/logger.h"
#include "headers/userControl.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>

// 全局心跳超时时间（在main中定义）
extern const int HEARTBEAT_TIMEOUT;
extern std::mutex g_sessionMutex;

// 消息转发辅助函数(判断对面是否存在，是否在线，然后转发消息)
static void ForwardToUser(Packet& packet, uint8_t senderID, uint8_t receiverID, const std::string& msgType) {
    int userStatus = CheckUser(receiverID);

    switch (userStatus) {
        case 0:
            WriteLog(LogLevel::PASS, std::to_string(senderID) + "发送的" + msgType + ", 接收人不存在");
            break;
        case 1:
            SaveOfflineMessages(receiverID, packet);
            WriteLog(LogLevel::PASS, std::to_string(senderID) + "发送了" + msgType);
            WriteLog(LogLevel::PASS, std::to_string(receiverID) + "不在线, 保存至离线消息");
            break;
        case 2:
            {
                bool sent = false;
                {
                    // 再次检查并发送（处理TOCTOU问题）
                    std::lock_guard<std::mutex> lock(g_sessionMutex);
                    if (g_userSessions.count(receiverID) && g_userSessions[receiverID] != nullptr) {
                        SendPacket(g_userSessions[receiverID]->socket_fd, packet);
                        sent = true;
                    } else {
                        // 时序问题：检查时在线，但现在已离线
                        SaveOfflineMessages(receiverID, packet);
                    }
                }
                // 在释放锁后记录日志
                if (sent) {
                    WriteLog(LogLevel::PASS, "来自" + std::to_string(senderID) + "的" + msgType + "已转发给: " + std::to_string(receiverID));
                } else {
                    WriteLog(LogLevel::PASS, "用户" + std::to_string(receiverID) + "已离线，消息保存为离线");
                }
            }
            break;
    }
}

static void UpdateHeartbeat(std::chrono::steady_clock::time_point& lastHeartbeat, ClientSession* sessionPtr) {
    // 更新心跳时间
    lastHeartbeat = std::chrono::steady_clock::now();
    if (sessionPtr) {
        sessionPtr->lastHeartbeatTime = lastHeartbeat;  // 同步更新ClientSession的心跳时间（用于UI闪烁效果）
    }
}

static void HandleLogin(Packet& receivedPacket, ClientSession* sessionPtr) {
    // 从header获取userID，从field2获取密码
    uint8_t userID = receivedPacket.getsendid();
    std::string password = receivedPacket.getField2Str();
    
    WriteLog(LogLevel::PROCESS, "登录请求: " + std::to_string(userID));
    
    // 调用登录函数
    bool logSuccess = LoginConnect(userID, password, sessionPtr);
    
    // 构造响应包
    Packet response = Packet::makeLoginRe(logSuccess);
    if (logSuccess) {
        WriteLog(LogLevel::PROCESS, "登录成功: " + std::to_string(userID));
    } else {
        WriteLog(LogLevel::PROCESS, "登录失败: " + std::to_string(userID));
    }
    
    // 发送响应给客户端
    SendPacket(sessionPtr->socket_fd, response);
    
    // 如果登录成功，推送离线消息
    if (logSuccess) {
        // 延迟3秒开始运行
        std::this_thread::sleep_for(std::chrono::seconds(3));
        SendOfflineMessages(userID, sessionPtr);
    }
}

static void HandleCreateAccount(Packet& receivedPacket, ClientSession* sessionPtr) {
    // 从header获取userID，从field2获取密码
    uint8_t userID = receivedPacket.getsendid();
    std::string password = receivedPacket.getField2Str();
    
    // 调用注册函数
    bool success = Signup(userID, password);
    
    // 构造响应包并发送
    Packet response = Packet::makeRegiRe(success);
    SendPacket(sessionPtr->socket_fd, response);
    WriteLog(LogLevel::PROCESS, "新账号注册: " + std::to_string(userID));
}

static void PassAddFriend(Packet& receivedPacket, ClientSession* sessionPtr) {
    // 检查本会话是否已在线
    if (!CheckOnline(sessionPtr->userid)) {
        WriteLog(LogLevel::WARN, "离线用户尝试发送好友请求: " + std::to_string(sessionPtr->userid));
        return;
    }
    
    uint8_t senderID = receivedPacket.getsendid();
    uint8_t receiverID = receivedPacket.getrecvid();
    
    // 转发消息给接收者
    ForwardToUser(receivedPacket, senderID, receiverID, "好友请求");
}

static void PassAddFriendRe(Packet& receivedPacket, ClientSession* sessionPtr) {
    // 检查本会话是否是否已在线
    if (!CheckOnline(sessionPtr->userid)) {
        WriteLog(LogLevel::WARN, "离线用户尝试发送好友响应: " + std::to_string(sessionPtr->userid));
        return;
    }
    
    // 从header获取收发信息，从field1获取内容
    uint8_t senderID = receivedPacket.getsendid();
    uint8_t receiverID = receivedPacket.getrecvid();
    
    // 转发消息给接收者
    ForwardToUser(receivedPacket, senderID, receiverID, "好友响应");
}

static void PassCommonMessage(Packet& receivedPacket, ClientSession* sessionPtr) {
    // 检查本会话是否已在线
    if (!CheckOnline(sessionPtr->userid)) {
        WriteLog(LogLevel::WARN, "离线用户尝试发送私聊消息: " + std::to_string(sessionPtr->userid));
        return;
    }
    
    // 从header获取收发信息，从field1获取内容
    uint8_t senderID = receivedPacket.getsendid();
    uint8_t receiverID = receivedPacket.getrecvid();

    // 转发消息给接收者
    ForwardToUser(receivedPacket, senderID, receiverID, "私聊消息");
}

static void HandleCreateGroup(Packet& receivedPacket, ClientSession* sessionPtr) {
    // 检测本会话是否在线
    if (!CheckOnline(sessionPtr->userid)) {
        WriteLog(LogLevel::WARN, "离线用户尝试创建群组");
        return;
    }
    
    WriteLog(LogLevel::PROCESS, "收到创建群组请求来自: " + std::to_string(sessionPtr->userid));

    uint8_t creatorID = receivedPacket.getsendid();
    std::vector<uint8_t> memberList = receivedPacket.getField1();
    std::string groupName = receivedPacket.getField2Str();

    // 将创建者添加到成员列表，这样服务器才能正确转发群聊消息
    memberList.push_back(creatorID);
    
    // 调用创建群聊函数
    bool success = CreateGroup(groupName, memberList);

    // 向群聊成员转发创建群聊消息
    if (success) {
        for (uint8_t memberID : memberList) { // 发送过来的消息中的成员列表中不含创建者
            if (memberID == creatorID) {
                continue;
            }
            ForwardToUser(receivedPacket, creatorID, memberID, "创建群聊");
        }
    }

    // 向创建者返回成功
    Packet response = Packet::makeCreGroRe(success);
    SendPacket(sessionPtr->socket_fd, response);
    WriteLog(LogLevel::PROCESS, "群聊创建成功, 名称为: " + groupName);
}

static void PassGroupMsg(Packet& receivedPacket, ClientSession* sessionPtr) {
    // 检测本会话是否在线
    if (!CheckOnline(sessionPtr->userid)) {
        WriteLog(LogLevel::WARN, "离线用户尝试发送群聊消息");
        return;
    }

    uint8_t senderID = receivedPacket.getsendid();
    std::string groupName = receivedPacket.getField2Str();

    WriteLog(LogLevel::PASS, "收到群聊消息 - 发送者: " + std::to_string(senderID) + ", 群聊: " + groupName);

    // 复制群成员列表（最小化持锁时间）
    std::vector<uint8_t> memberList;
    {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        // 检查群聊是否存在
        if (!g_groupChat.count(groupName)) {
            std::string errorMsg = "群聊不存在: " + groupName + 
                                   ", 发送者为: " + std::to_string(senderID);
            WriteLog(LogLevel::PASS, errorMsg);
            return;
        }
        // 获取群成员列表
        memberList = g_groupChat[groupName];
    }


    for (uint8_t memberID : memberList) {
        // 跳过发送者自己
        if (memberID == senderID) {
            continue;
        }
        ForwardToUser(receivedPacket, senderID, memberID, "群聊消息");
    }
}

static void PassImage(Packet& receivedPacket, ClientSession* sessionPtr) {
    // 检测本会话是否在线
    if (!CheckOnline(sessionPtr->userid)) {
        WriteLog(LogLevel::WARN, "离线用户尝试发送图片消息");
        return;
    }

    bool isgroup = receivedPacket.success();
    uint8_t senderID = receivedPacket.getsendid();
    if (isgroup) {
        std::string groupName = receivedPacket.getField3Str();

        std::vector<uint8_t> memberList; // 复制一份减少锁时间
        {
            std::lock_guard<std::mutex> lock(g_sessionMutex);
            // 检查群聊是否存在
            if (!g_groupChat.count(groupName)) {
                std::string errorMsg = "群聊不存在: " + groupName + 
                                    ", 发送者为: " + std::to_string(senderID);
                WriteLog(LogLevel::PASS, errorMsg);
                return;
            }
            // 获取群成员列表
            memberList = g_groupChat[groupName];
        }
        for (uint8_t memberID : memberList) {
        // 跳过发送者自己
            if (memberID == senderID) {
                continue;
            }
            ForwardToUser(receivedPacket, senderID, memberID, "群聊图片");
        } 
    } else {
        uint8_t receiverID = receivedPacket.getrecvid();
            ForwardToUser(receivedPacket, senderID, receiverID, "私聊图片");
    }
}

static void HandleSetUserName(Packet& receivedPacket, ClientSession* sessionPtr) {
    // 检测本会话是否在线
    if (!CheckOnline(sessionPtr->userid)) {
        WriteLog(LogLevel::WARN, "离线用户尝试更改用户名");
        return;
    }

    uint8_t userID = receivedPacket.getsendid();
    std::string userName = receivedPacket.getField1Str();

    receivedPacket.setSuccess(SetUserName(userID, userName));
    SendPacket(sessionPtr->socket_fd, receivedPacket);
}

static void HandleCheckStatus(Packet& receivedPacket, ClientSession* sessionPtr) {
        // 检测本会话是否在线
    if (!CheckOnline(sessionPtr->userid)) {
        WriteLog(LogLevel::WARN, "离线用户尝试更改查询用户状态");
        return;
    }

    uint8_t userID = receivedPacket.getsendid();
    uint8_t targetID = receivedPacket.getrecvid();

    bool isOnline = false;
    std::string targetName = "";
    switch (CheckUser(targetID)) {
        case 0: {
            WriteLog(LogLevel::PROCESS, "查询的用户不存在");
            return;
        }
        case 1: {
            targetName = GetUserName(targetID);
            break;
        }
        case 2: {
            targetName = GetUserName(targetID);
            isOnline = true;
            break;
        }
    }
    receivedPacket.CheckUserStatusReply(targetName, isOnline);
    SendPacket(sessionPtr->socket_fd, receivedPacket);
}

// 工作线程入口函数：为每个客户端分配独立线程处理消息
void HandleClient(ClientSession* sessionPtr) { // 这个会话指针（sessionPtr)作为一个客户端在内存中的唯一代表
    SOCKET clientSocket = sessionPtr->socket_fd;
    std::string clientInfo = sessionPtr->client_ip + ":" + std::to_string(clientSocket); // 读取这个连接的ip和端口
    WriteLog(LogLevel::CONNECTION, "客户端处理线程启动: " + clientInfo);

    // 记录最后一次心跳时间
    auto lastHeartbeat = std::chrono::steady_clock::now();

    // 消息接收循环，持续接收并处理客户端消息
    // 逻辑是：如果socket没收到数据，就一直检查心跳是否超时，如果有数据，再读是什么数据，再决定要干啥
    // 判断socket中有没有数据，用的是select函数
    while (true) {
        // 检查心跳超时
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastHeartbeat); // 距离上次心跳的时间间隔
        if (duration.count() > HEARTBEAT_TIMEOUT) {
            WriteLog(LogLevel::CONNECTION, clientInfo + "的心跳超时, 断开连接" + 
                     " (超时: " + std::to_string(HEARTBEAT_TIMEOUT) + "秒)");
            break;
        }
        
        // 使用select检测是否有数据可读（超时1秒）
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sessionPtr->socket_fd, &readSet);
        // 1秒超时
        timeval timeout; // 为什么不设置成全局变量方便控制：select会修改timeout的数值，所以只能每个客户端分一个分别计算
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int selectResult = select(0, &readSet, nullptr, nullptr, &timeout);
        
        if (selectResult == SOCKET_ERROR) {
            WriteLog(LogLevel::WARN, "Select错误: " + clientInfo);
            break;
        }
        
        if (selectResult == 0) {
            // select超时，没有数据可读，继续下一次循环检查心跳
            continue;
        }
        
        // 有数据可读，接收数据包
        Packet receivedPacket; // 创建数据包对象

        if (!RecvPacket(sessionPtr->socket_fd, receivedPacket)) {
            // 连接断开或接收失败
            WriteLog(LogLevel::CONNECTION, "客户端断开连接: " + clientInfo);
            break;
        }

        // // 临时调试日志 - 接收数据后再打印
        // char typeBuf[8];
        // sprintf(typeBuf, "0x%02X", static_cast<uint8_t>(receivedPacket.type()));
        // WriteLog(LogLevel::PROCESS, "收到消息类型值: " + std::string(typeBuf) + 
        //  " (十进制: " + std::to_string(static_cast<int>(receivedPacket.type())) + ")");
        
        // 根据消息类型分别处理
        switch (receivedPacket.type()) {
            // 心跳包
            case MsgType::Heartbeat: {
                UpdateHeartbeat(lastHeartbeat, sessionPtr);
                break;
            }
            
            // 注册请求
            case MsgType::CreateAcc: {
                HandleCreateAccount(receivedPacket, sessionPtr);
                break;
            }

            // 登录请求
            case MsgType::LoginReq: {
                HandleLogin(receivedPacket, sessionPtr);
                break;
            }

            // 转发添加好友请求
            case MsgType::AddFriendReq: {
                PassAddFriend(receivedPacket, sessionPtr);
                break;
            }

            // 转发好友响应
            case MsgType::AddFriendRe: { 
                PassAddFriendRe(receivedPacket, sessionPtr);
                break;
            }

            // 转发私聊聊天消息
            case MsgType::NormalMsg: { 
                PassCommonMessage(receivedPacket, sessionPtr);
                break;
            }
            
            // 创建群组请求
            case MsgType::CreateGrope: { // chatmsg里是grope所以就grope吧
                HandleCreateGroup(receivedPacket, sessionPtr);
                break;
            }
            
            // 转发群聊消息
            case MsgType::GroupMsg: {
                PassGroupMsg(receivedPacket, sessionPtr);
                break;
            }

            // 转发图片
            case MsgType::ImageMsg: {
                PassImage(receivedPacket, sessionPtr);
                break;
            }

            case MsgType::SetName: {
                HandleSetUserName(receivedPacket, sessionPtr);
                break;
            }

            case MsgType::CheckUser: {
                HandleCheckStatus(receivedPacket, sessionPtr);
                break;
            }
            
            default: {
                WriteLog(LogLevel::WARN, 
                         "未知消息类型: " + std::to_string(static_cast<int>(receivedPacket.type())));
                break;
            }
        }
    }
    
    // 清理工作
    WriteLog(LogLevel::CONNECTION, "客户端断开连接: " + clientInfo);

    
    if (sessionPtr->userid != 0) {
        LogOff(sessionPtr->userid);
    } else {
        delete sessionPtr;
    }
    sessionPtr = nullptr;
    
    // 关闭socket
    closesocket(clientSocket);
}
