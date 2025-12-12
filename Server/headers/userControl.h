#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include <winsock2.h>
#include "../chatMsg_server.hpp"

// 用户会话类
class ClientSession {
public:
    SOCKET socket_fd;          // 客户端socket
    std::string client_ip;     // 客户端IP
    unsigned short client_port; // 客户端端口
    uint8_t userid;            // 用户ID
    std::chrono::steady_clock::time_point lastHeartbeatTime;  // 最后一次心跳时间

    // 构造函数
    ClientSession(SOCKET fd, const std::string& ip, unsigned short port);
    
    // 更新用户ID
    void setID(uint8_t id);
};

// 全局变量声明
extern std::map<uint8_t, std::string> g_userCredentials;       // 用户凭证(ID->密码)
extern std::map<uint8_t, ClientSession*> g_userSessions;       // 用户会话(ID->会话指针)
extern std::map<uint8_t, std::vector<Packet>> g_offlineMessages; // 离线消息队列
extern std::map<std::string, std::vector<uint8_t>> g_groupChat;  // 群聊(群名->成员列表)
extern std::map<uint8_t, std::string> g_userName;              // 用户名(ID->用户名)
extern std::mutex g_sessionMutex;  // 保护用户会话数据的互斥锁

// 用户检查函数
bool CheckExist(uint8_t userID);   // 检查用户是否存在
bool CheckOnline(uint8_t userID);  // 检查用户是否在线
int CheckUser(uint8_t userID);     // 综合检查(0=不存在, 1=离线, 2=在线)

// 用户管理函数
bool Signup(uint8_t userID, const std::string& password);  // 注册账户
bool LoginConnect(uint8_t userID, const std::string& password, ClientSession* session); // 登录
void LogOff(uint8_t userID);  // 下线
void ForceDisconnect(uint8_t userID);  // 强制用户下线并断开连接
void DeleteUser(uint8_t userID);  // 彻底删除用户（包括账号、数据、连接）

// 群聊管理函数
bool CreateGroup(std::string& groupName, std::vector<uint8_t>& memberList); // 创建群聊

// 离线消息函数
void SaveOfflineMessages(uint8_t userID, Packet message);  // 保存离线消息
void SendOfflineMessages(uint8_t userID, ClientSession* session); // 发送离线消息

// 用户名函数
std::string GetUserName(uint8_t userID);  // 查询用户名

// 用户名管理函数
bool SetUserName(uint8_t userID, std::string& userName);  // 设置用户名