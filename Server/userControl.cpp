#include "headers/userControl.h"
#include "headers/logger.h"
#include "headers/socket.h"
#include <cstdint>
#include <mutex>
#include <algorithm>
#include <chrono>

// 全局变量定义
std::map<uint8_t, std::string> g_userCredentials;
std::map<uint8_t, ClientSession*> g_userSessions;
std::map<uint8_t, std::vector<Packet>> g_offlineMessages;
std::map<std::string, std::vector<uint8_t>> g_groupChat;
std::map<uint8_t, std::string> g_userName;
std::mutex g_sessionMutex;

// ClientSession 类成员函数实现
ClientSession::ClientSession(SOCKET fd, const std::string& ip, unsigned short port)
    : socket_fd(fd), 
      client_ip(ip), 
      client_port(port), 
      userid(0),
      lastHeartbeatTime(std::chrono::steady_clock::now())  // 初始化心跳时间
{
    std::string logmessage = "客户端连接: IP = " + client_ip + ", 端口 = " + std::to_string(client_port);
    WriteLog(LogLevel::CONNECTION, logmessage);
}

void ClientSession::setID(uint8_t id) {
    this->userid = id;
}

/*
这个用户会话类的使用逻辑是这样的：我先使用accept连接客户端，然后我会为它分配一个对应这个会话类的线程。这个线程
仅仅是用于处理它与客户端之间的通信的，与他登录什么账号没有关系。验证账号密码，通过id识别会话对象，这些通过下面的map
来实现。
*/

/*
注意：现在一旦服务器关停，所有的账户信息都会被抹除，如果想要永久记录，除非手
动输入，否则可能需要通过构建数据库例如使用SQLite，然而这样工作量就太大了，因此
目前只能先这样了。当然也可以通过读取文件的方式在重启服务器的时候重新创建，然而
那样并不是一个很好的解决方案，如果用户量增多，重新创建用户对象的时间会难以估量
*/

// 检查用户是否存在
bool CheckExist(uint8_t userID) {
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    if (g_userSessions.count(userID)) {
        return true;
    }
    return false;
}
// 检查用户是否在线
bool CheckOnline(uint8_t userID) {
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    // 先检查是否存在，避免 map 自动创建条目
    if (!g_userSessions.count(userID)) {
        return false;
    }
    if (g_userSessions[userID] == nullptr) {
        return false;
    }
    return true;
}
// 先检查是否存在，再检查是否在线
int CheckUser(uint8_t userID) {
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    // 直接访问 g_userSessions，避免重复加锁
    if (!g_userSessions.count(userID)) {
        return 0;  // 用户不存在
    }
    if (g_userSessions[userID] == nullptr) {
        return 1;  // 用户存在但离线
    }
    return 2;  // 用户在线
}

// 注册函数: 1. 添加账户密码键值对 2. 给id绑定一个空的会话指针
bool Signup(uint8_t userID, const std::string &password) {
    bool success = false;
    {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        // 直接检查 g_userSessions，避免调用 CheckExist 导致重复加锁
        if (g_userSessions.count(userID)) {
            success = false;
        } else {
            g_userCredentials[userID] = password;
            g_userSessions[userID] = nullptr;
            g_userName[userID] = "Anonymous";
            success = true;
        }
    }
    
    // 在释放锁后记录日志
    if (success) {
        WriteLog(LogLevel::PROCESS, "账号创建成功: " + std::to_string(userID));
    } else {
        WriteLog(LogLevel::PROCESS, "账号创建失败 - ID已存在: " + std::to_string(userID));
    }
    
    return success;
}

// 登录函数:1. 验证登录凭证，2. 将id与这个会话线程绑定
bool LoginConnect(uint8_t userID, const std::string &Password, ClientSession* session) {
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    // 直接检查 g_userSessions，避免调用 CheckExist 导致重复加锁
    if (g_userSessions.count(userID)) {
        if (g_userSessions[userID] != nullptr) {
            return false;
        } else if (g_userCredentials[userID] == Password) {
            g_userSessions[userID] = session;
            session->setID(userID);
            return true;
        }
    }
    return false;
}

// 下线函数: 删除会话，把id绑定的会话指针改为空指针（这个函数只在会话登录了账户的情况下才要调用）
void LogOff(uint8_t userID) {
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    // 直接检查 g_userSessions，避免调用 CheckExist 导致重复加锁
    if (g_userSessions.count(userID)) {
        delete g_userSessions[userID];
        g_userSessions[userID] = nullptr;
    }
}

// 强制用户下线并断开连接
void ForceDisconnect(uint8_t userID) {
    ClientSession* session = nullptr;
    SOCKET clientSocket = INVALID_SOCKET;
    
    // 获取session并关闭socket（在锁外执行IO操作）
    {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        if (g_userSessions.count(userID) && g_userSessions[userID] != nullptr) {
            session = g_userSessions[userID];
            clientSocket = session->socket_fd;
        }
    }
    
    // 关闭socket连接（不持锁）
    if (clientSocket != INVALID_SOCKET) {
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);
    }
    
    // 清理session
    if (session) {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        // 再次检查，防止期间被其他线程删除
        if (g_userSessions.count(userID) && g_userSessions[userID] == session) {
            delete g_userSessions[userID];
            g_userSessions[userID] = nullptr;
            WriteLog(LogLevel::CONNECTION, "强制下线用户: " + std::to_string(userID));
        }
    }
}

// 彻底删除用户（包括账号、数据、连接）
void DeleteUser(uint8_t userID) {
    // 1. 先强制下线（如果在线）
    ForceDisconnect(userID);
    
    // 2. 删除所有用户数据（持锁操作）
    {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        
        // 删除账号密码
        if (g_userCredentials.count(userID)) {
            g_userCredentials.erase(userID);
        }
        
        // 删除用户名
        if (g_userName.count(userID)) {
            g_userName.erase(userID);
        }
        
        // 删除离线消息
        if (g_offlineMessages.count(userID)) {
            g_offlineMessages.erase(userID);
        }
        
        // 从所有群聊中移除该用户
        for (auto& group : g_groupChat) {
            auto& memberList = group.second;
            memberList.erase(
                std::remove(memberList.begin(), memberList.end(), userID),
                memberList.end()
            );
        }
        
        // 确保session已清理
        if (g_userSessions.count(userID)) {
            if (g_userSessions[userID] != nullptr) {
                delete g_userSessions[userID];
            }
            g_userSessions.erase(userID);
        }
    }
    
    WriteLog(LogLevel::INFO, "已彻底删除用户: " + std::to_string(userID));
}

// 创建群聊函数
bool CreateGroup(std::string &groupName, std::vector<uint8_t> &memberList) {
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    // 先检查群聊存不存在
    if (g_groupChat.count(groupName)) {
        WriteLog(LogLevel::PROCESS, "群聊已经存在");
        return false;
    }
    g_groupChat[groupName] = memberList;
    return true;
}


// 存储离线消息: 如果发现接收者不在线，则把要发送的消息暂存到离线消息队列g_offlineMessages中
void SaveOfflineMessages(uint8_t userID, Packet message) {
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    // 直接添加到离线消息队列（如果key不存在，map会自动创建空vector）
    g_offlineMessages[userID].push_back(message);
}

// 发送离线消息函数：当用户上线时，将所有离线消息推送给该用户
void SendOfflineMessages(uint8_t userID, ClientSession* session) {
    std::vector<Packet> messagesToSend;
    
    // 复制离线消息到本地（最小化持锁时间）
    {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        // 判断有没有离线消息（有key就有）
        if (!g_offlineMessages.count(userID)) {
            return;
        }
        messagesToSend = g_offlineMessages[userID];
        // 先清空，发送失败再恢复
        g_offlineMessages.erase(userID);
    }
    
    int count = messagesToSend.size();
    WriteLog(LogLevel::PASS, 
             "推送离线消息给: " + std::to_string(userID) + 
             ", 消息数量: " + std::to_string(count));
    
    // 逐条发送离线消息
    int sentCount = 0;
    for (size_t i = 0; i < messagesToSend.size(); ++i) {
        if (!SendPacket(session->socket_fd, messagesToSend[i])) {
            WriteLog(LogLevel::PASS, 
                     "离线消息发送中断, 已发送: " + std::to_string(sentCount) + 
                     " 条，剩余: " + std::to_string(messagesToSend.size() - i) + " 条");
            
            // 将未发送的消息重新保存
            std::lock_guard<std::mutex> lock(g_sessionMutex);
            for (size_t j = i; j < messagesToSend.size(); ++j) {
                g_offlineMessages[userID].push_back(messagesToSend[j]);
            }
            return;
        }
        sentCount++;
    }
    
    WriteLog(LogLevel::PASS, 
             "离线消息推送完成, 共计: " + std::to_string(sentCount) + " 条");
}

bool SetUserName(uint8_t userID, std::string& userName) {
    bool success = false;
    {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        // 直接检查 g_userSessions，避免调用 CheckExist 导致重复加锁
        if (g_userSessions.count(userID)) {
            g_userName[userID] = userName;
            
            success = true;
        }
    }
    
    // 在释放锁后记录日志
    if (success) {
        WriteLog(LogLevel::PROCESS, "用户" + std::to_string(userID) + "修改用户名为" + userName);
    } else { 
        WriteLog(LogLevel::PROCESS, "用户不存在，无法更改用户名"); 
    }

    return success;
}

std::string GetUserName(uint8_t userID) {
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    // 如果用户名不存在，返回空字符串
    if (g_userName.count(userID)) {
        return g_userName[userID];
    }
    return "";
}