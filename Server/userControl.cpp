#include "headers/userControl.h"
#include "headers/logger.h"
#include "headers/socket.h"
#include <cstdint>

// 全局变量定义
std::map<uint8_t, std::string> g_userCredentials;
std::map<uint8_t, ClientSession*> g_userSessions;
std::map<uint8_t, std::vector<Packet>> g_offlineMessages;
std::map<std::string, std::vector<uint8_t>> g_groupChat;
std::map<uint8_t, std::string> g_userName;

// ClientSession 类成员函数实现
ClientSession::ClientSession(SOCKET fd, const std::string& ip, unsigned short port)
    : socket_fd(fd), 
      client_ip(ip), 
      client_port(port), 
      userid(0)
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
    if (g_userSessions.count(userID)) {
        return true;
    }
    return false;
}
// 检查用户是否在线
bool CheckOnline(uint8_t userID) {
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
    if (!CheckExist(userID)) {
        return 0;
    } else if (!CheckOnline(userID)) {
        return 1;
    }
    return 2;
}

// 注册函数: 1. 添加账户密码键值对 2. 给id绑定一个空的会话指针
bool Signup(uint8_t userID, const std::string &password) {
    if (CheckExist(userID)) { // 检查用户是否已经存在（id唯一）
                    WriteLog(LogLevel::PROCESS, "账号创建失败 - ID已存在: " + std::to_string(userID));
                    return false;
                } else {
                    g_userCredentials[userID] = password;
                    g_userSessions[userID] = nullptr;
                    WriteLog(LogLevel::PROCESS, "账号创建成功: " + std::to_string(userID));
                    return true;
                }
}

// 登录函数:1. 验证登录凭证，2. 将id与这个会话线程绑定
bool LoginConnect(uint8_t userID, const std::string &Password, ClientSession* session) {
    if (CheckExist(userID)) { // 检查map中是否存有该用户ID
        if (g_userCredentials[userID] == Password) { // 如果存在，比较密码
            g_userSessions[userID] = session;
            session->setID(userID);
            return true;
        }
    }
    return false;
}

// 下线函数: 删除会话，把id绑定的会话指针改为空指针（这个函数只在会话登录了账户的情况下才要调用）
void LogOff(uint8_t userID) {
    if (CheckExist(userID)) {
        delete g_userSessions[userID];
        g_userSessions[userID] = nullptr;
    }
}

// 创建群聊函数
bool CreateGroup(std::string &groupName, std::vector<uint8_t> &memberList) {
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
    // 直接添加到离线消息队列（如果key不存在，map会自动创建空vector）
    g_offlineMessages[userID].push_back(message);
}

// 发送离线消息函数：当用户上线时，将所有离线消息推送给该用户
void SendOfflineMessages(uint8_t userID, ClientSession* session) {
    // 判断有没有离线消息（有key就有）
    if (!g_offlineMessages.count(userID)) {
        return;
    }
    
    std::vector<Packet>& messages = g_offlineMessages[userID];
    int count = messages.size();

    DebugWriteLog(LogLevel::PASS, 
             "推送离线消息给: " + std::to_string(userID) + 
             ", 消息数量: " + std::to_string(count));
    
    // 逐条发送离线消息（发送一条删除一条，支持断点续传）
    int sentCount = 0;
    while (!messages.empty()) {
        const Packet& packet = messages.front();  // 获取第一条消息的引用
        
        if (!SendPacket(session->socket_fd, packet)) {
            WriteLog(LogLevel::PASS, 
                     "离线消息发送中断, 已发送: " + std::to_string(sentCount) + 
                     " 条，剩余: " + std::to_string(messages.size()) + " 条");
            return; // 发送失败则停止，剩余消息保留
        }
        
        messages.erase(messages.begin());  // 发送成功，删除这条消息
        sentCount++;
    }
    
    // 全部发送成功，删除该用户的离线消息队列
    g_offlineMessages.erase(userID);
    WriteLog(LogLevel::PASS, 
             "离线消息推送完成, 共计: " + std::to_string(sentCount) + " 条");
}

void SetUserName(uint8_t userID, std::string& userName) {
    if (CheckExist(userID)) {
        g_userName[userID] = userName;
        WriteLog(LogLevel::PROCESS, "用户" + std::to_string(userID) + "修改用户名为" + userName);
    } else { WriteLog(LogLevel::PROCESS, "用户不存在，无法更改用户名"); }
}