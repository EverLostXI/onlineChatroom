#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <sstream>
// 获取时间戳
#include <ctime>
// 网络连接部分
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <cstdint>
// 生成服务器日志
#include <fstream>
#include <iomanip>
// 线程库
#include <thread>
// 时间库（用于心跳检测）
#include <chrono>
// 消息封装类（服务器专用版本，不依赖Qt）
#include "chatMsg_server.hpp"
// 需要链接WinSock库
// #pragma comment(lib, "ws2_32.lib") 非MSVC编译器无法识别，必须手动链接Winsock库
// 服务端已经用CMake链接

// TODO: 注册时添加usersession = nullptr
// TODO: 添加监视窗口，包含用户列表，群聊列表，在线客户端列表，usersession跟踪，手动控制map绑定
// TODO：获取服务器ip地址的函数
// TODO：细化日志的消息类型和具体存储哪些内容
// 设置服务器端口号
const int PORT = 8888;
// 设置listen连接队列长度
const int BACKLOG = 5;
// 心跳超时时间（秒）- 如果超过此时间未收到心跳包，认为客户端已断开
const int HEARTBEAT_TIMEOUT = 30;

// 启用调试模式（是否往日志中记录DEBUG和TRACE级别的信息）（最好在运行后输入，这样不用每次更换前都重新编译（虽然也花不了几个时间））
bool g_debugMode = true;

// 为Packet添加Windows socket支持的补充函数
// chatmsg中recv和send是用的qt封装的函数，由于server不依赖qt，只能重新写一个winsock版本的

// 接收数据包函数
bool RecvPacket(SOCKET sock, Packet& packet) {
    // 先接收Header（Header的定义在chatmsg.hpp中）
    char headerBuf[sizeof(Header)]; // 取得包头长度
    int totalReceived = 0; 
    while (totalReceived < sizeof(Header)) { // 循环接收包头
        int n = recv(sock, headerBuf + totalReceived, sizeof(Header) - totalReceived, 0);
        if (n <= 0) return false;
        totalReceived += n;
    }
    
    // 解析header获取body大小
    Header* hdr = reinterpret_cast<Header*>(headerBuf);
    size_t bodySize = n2h16(hdr->field1Len) + n2h16(hdr->field2Len) + 
                      n2h16(hdr->field3Len) + n2h16(hdr->field4Len);
    
    // 接收完整数据包
    std::vector<char> fullPacket(sizeof(Header) + bodySize);
    memcpy(fullPacket.data(), headerBuf, sizeof(Header));
    
    totalReceived = 0;
    while (totalReceived < bodySize) {
        int n = recv(sock, fullPacket.data() + sizeof(Header) + totalReceived, 
                     bodySize - totalReceived, 0);
        if (n <= 0) return false;
        totalReceived += n;
    }
    
    // 使用parseFrom解析
    return packet.parseFrom(fullPacket.data(), fullPacket.size());
}

// 发送数据包函数
bool SendPacket(SOCKET sock, const Packet& packet) {
    // 准备完整数据包（header + field1 + field2 + field3 + field4）
    size_t totalSize = packet.size();
    std::vector<char> fullPacket(totalSize);
    
    // 1. 拷贝并转换header为网络字节序
    Header networkHeader;
    memcpy(&networkHeader, packet.data(), sizeof(Header));
    networkHeader.field1Len = h2n16(networkHeader.field1Len);
    networkHeader.field2Len = h2n16(networkHeader.field2Len);
    networkHeader.field3Len = h2n16(networkHeader.field3Len);
    networkHeader.field4Len = h2n16(networkHeader.field4Len);
    memcpy(fullPacket.data(), &networkHeader, sizeof(Header));
    
    // 2. 依次拷贝各个field到header后面
    size_t offset = sizeof(Header);
    
    const auto& field1 = packet.getField1();
    if (!field1.empty()) {
        memcpy(fullPacket.data() + offset, field1.data(), field1.size());
        offset += field1.size();
    }
    
    const auto& field2 = packet.getField2();
    if (!field2.empty()) {
        memcpy(fullPacket.data() + offset, field2.data(), field2.size());
        offset += field2.size();
    }
    
    const auto& field3 = packet.getField3();
    if (!field3.empty()) {
        memcpy(fullPacket.data() + offset, field3.data(), field3.size());
        offset += field3.size();
    }
    
    const auto& field4 = packet.getField4();
    if (!field4.empty()) {
        memcpy(fullPacket.data() + offset, field4.data(), field4.size());
    }
    
    // 3. 一次性发送完整数据包
    int totalSent = 0;
    while (totalSent < totalSize) {
        int n = send(sock, fullPacket.data() + totalSent, totalSize - totalSent, 0);
        if (n <= 0) {
            return false;
        }
        totalSent += n;
    }
    
    return true;
}

// 获取时间戳
std::string TimeStamp() {
    time_t currentTime = time(nullptr); //不是可直接阅读的日历时间
    tm* localTime = localtime(&currentTime); //转换为服务器本地时间
    std::stringstream ss; // 开始生成时间戳
    ss << "[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "]";
    return ss.str();
}


// 初始化服务器日志
std::ofstream logFile; // 初始化文件句柄

std::string FileNameGen() { // 将日志以当前日期命名，但是不包含具体时间，逻辑与获取时间戳函数一致
    time_t currentTime = time(nullptr);
    tm* localTime = localtime(&currentTime);
    std::stringstream fname;
    fname << std::put_time(localTime, "%Y%m%d");
    fname << ".log";
    return fname.str();
}

// 日志写入函数
enum class LogLevel { // 限定日志级别
    FATAL_LEVEL, // 系统崩溃或不可恢复的错误
    ERROR_LEVEL, // 运行时错误，功能受影响但程序仍在运行
    WARN_LEVEL, // 潜在的问题或异常情况
    INFO_LEVEL, // 程序正常运行的关键步骤
    DEBUG_LEVEL, // 详细的调试信息
    TRACE_LEVEL // 追踪细致操作
};

std::string LevelToString(LogLevel level) { // 将日志级别转为字符串
    switch (level) {
        case LogLevel::FATAL_LEVEL: return "FATAL";
        case LogLevel::ERROR_LEVEL: return "ERROR";
        case LogLevel::WARN_LEVEL: return "WARN";
        case LogLevel::INFO_LEVEL: return "INFO";
        case LogLevel::DEBUG_LEVEL: return "DEBUG";
        case LogLevel::TRACE_LEVEL: return "TRACE";
        default: return "UNKNOWN";
    }
}
// 日志写入函数
void WriteLog(LogLevel level, const std::string& message) { //包含两个参数：重要级，消息内容
    if (logFile.is_open()) {
        logFile << TimeStamp() << "[" << LevelToString(level) << "]" << message << std::endl;
        logFile.flush(); //立即刷新缓冲区
        std::cout << message << std::endl;
    }
}

// 增加内联辅助函数用于判断调试模式是否开启，如果未开启，不会打印DEBUG和TRACE级别的目录到日志中
inline void DebugWriteLog(LogLevel level, const std::string message) {
    if (level == LogLevel::DEBUG_LEVEL || level == LogLevel::TRACE_LEVEL) {
        if (!g_debugMode) return;
    }
    WriteLog(level, message);
}


void InitializeLogFile() { // 生成文件
    const std::string LOG_FOLDER = "log/";
    std::string logfilename = LOG_FOLDER + FileNameGen();
    logFile.open(logfilename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "无法打开日志文件：" << logfilename << std::endl; // 此时无法写入日志（当然）
    } else {
        std::string message = "日志文件初始化成功：" + logfilename;
        WriteLog(LogLevel::INFO_LEVEL, message);
    }
}

void CloseLogFile() {
    if (logFile.is_open()) {
        WriteLog(LogLevel::INFO_LEVEL, "关闭日志文件");
        logFile.close();
    }
}


// 初始化和清理Winsock
bool InitializeWinSock() {
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2,2), &wsadata) !=0) {
        std::string errormessage = "WSA启动失败:" + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL_LEVEL, errormessage);
        return false;
    }
    DebugWriteLog(LogLevel::DEBUG_LEVEL, "WinSock 2.2 初始化成功");
    return true;
}

void CleanupWinSock() {
    WSACleanup();
    DebugWriteLog(LogLevel::DEBUG_LEVEL, "WinSock 清理完成");
}


// 配置服务器地址
int SetupServerAddress(const int port, sockaddr_in& address_info) {
    if (port <= 0 || port > 65535) {
        WriteLog(LogLevel::FATAL_LEVEL, "配置的端口号无效");
        return -1;
    }
    memset(&address_info, 0, sizeof(address_info)); // 清零结构体
    address_info.sin_family = AF_INET; // 设置地址族为IPv4
    address_info.sin_addr.s_addr = htonl(INADDR_ANY); //设置IP地址：监听所有接口（转换为网络字节序）
    address_info.sin_port = htons(port); // 设置端口号（转换为网络字节序）
    return 0;
}


// 用户会话类（注意这个类里不包含id，密码，这个类仅仅用于表示这个用户的网络连接属性）
class ClientSession {
public:
    // 网络属性
    SOCKET socket_fd;
    std::string client_ip;
    unsigned short client_port;
    uint8_t userid;

public:
    // 构造函数：初始化用户属性
    ClientSession(SOCKET fd, const std::string& ip, unsigned short port)
        : socket_fd(fd), 
        client_ip(ip), 
        client_port(port), 
        userid(0)

    { // 向日志中记录用户连接
        std::string logmessage = "客户端连接: IP = " + client_ip + ", 端口 = " + std::to_string(client_port);
        WriteLog(LogLevel::INFO_LEVEL, logmessage);
    }

    // 更新用户id对应
    void setID(uint8_t id) {
        this->userid = id;
    }

};

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



// 存储ID与密码（ID统一为uint8长度）
std::map<uint8_t, std::string> g_userCredentials;

//将ID与用户对象对应（在线状态在这里判定：如果用户下线，它的会话会被删除以释放内存，这里的会话指针会被改为空指针。空指针=不在线）
std::map<uint8_t, ClientSession*> g_userSessions;


// 存储每个用户的离线消息队列（key: userId, value: 消息列表）
std::map<uint8_t, std::vector<Packet>> g_offlineMessages;


// 登录函数:1. 验证登录凭证，2. 将id与这个会话线程绑定
bool LoginConnect(uint8_t userID, const std::string &inputPassword, ClientSession* session) {
    if (g_userCredentials.count(userID)) { // 检查map中是否存有该用户ID
        if (g_userCredentials[userID] == inputPassword) { // 如果存在，比较密码
            g_userSessions[userID] = session;
            session->setID(userID);
            return true;
        }
    }
    return false;
}

// 下线函数: 删除会话，把id绑定的会话指针改为空指针（这个函数只在会话登录了账户的情况下才要调用）
void LogOff(uint8_t userID) {
    if (g_userCredentials.count(userID)) {
        if (g_userSessions.count(userID)) {
            delete g_userSessions[userID];
            g_userSessions[userID] = nullptr;
        }
    }
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

    DebugWriteLog(LogLevel::TRACE_LEVEL, 
             "开始推送离线消息 - 用户ID: " + std::to_string(userID) + 
             ", 消息数量: " + std::to_string(count));
    
    // 逐条发送离线消息（发送一条删除一条，支持断点续传）
    int sentCount = 0;
    while (!messages.empty()) {
        const Packet& packet = messages.front();  // 获取第一条消息的引用
        
        if (!SendPacket(session->socket_fd, packet)) {
            WriteLog(LogLevel::ERROR_LEVEL, 
                     "离线消息发送失败 - 用户ID: " + std::to_string(userID) + 
                     ", 已发送: " + std::to_string(sentCount) + " 条，剩余: " + std::to_string(messages.size()) + " 条");
            return; // 发送失败则停止，剩余消息保留
        }
        
        messages.erase(messages.begin());  // 发送成功，删除这条消息
        sentCount++;
    }
    
    // 全部发送成功，删除该用户的离线消息队列
    g_offlineMessages.erase(userID);
    WriteLog(LogLevel::INFO_LEVEL, 
             "离线消息推送完成 - 用户ID: " + std::to_string(userID) + 
             ", 已发送: " + std::to_string(sentCount) + " 条");
}


// 工作线程入口函数：为每个客户端分配独立线程处理消息
void HandleClient(ClientSession* sessionPtr) { // 这个会话指针（sessionPtr)作为一个客户端在内存中的唯一代表
    SOCKET clientSocket = sessionPtr->socket_fd;
    std::string clientInfo = sessionPtr->client_ip + ":" + std::to_string(sessionPtr->client_port); // 读取这个连接的ip和端口
    DebugWriteLog(LogLevel::DEBUG_LEVEL, "客户端处理线程启动: " + clientInfo);

    // 记录最后一次心跳时间
    auto lastHeartbeat = std::chrono::steady_clock::now();

    // 消息接收循环，持续接收并处理客户端消息
    // 逻辑是：如果socket没收到数据，我就一直检查心跳是否超时，如果有数据，我再读是什么数据，再决定要干啥
    // 判断socket中有没有数据，用的是select函数
    while (true) {
        // 检查心跳超时
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastHeartbeat); // 距离上次心跳的时间间隔
        if (duration.count() > HEARTBEAT_TIMEOUT) {
            WriteLog(LogLevel::INFO_LEVEL, "客户端心跳超时，断开连接: " + clientInfo + 
                     " (超时: " + std::to_string(HEARTBEAT_TIMEOUT) + "秒)");
            break;
        }
        
        // 使用select检测是否有数据可读（超时1秒）
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(clientSocket, &readSet);
        // 1秒超时
        timeval timeout; // 为什么不设置成全局变量方便控制：select会修改timeout的数值，所以只能每个客户端分一个分别计算
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int selectResult = select(0, &readSet, nullptr, nullptr, &timeout);
        
        if (selectResult == SOCKET_ERROR) {
            WriteLog(LogLevel::ERROR_LEVEL, "Select错误: " + clientInfo);
            break;
        }
        
        if (selectResult == 0) {
            // select超时，没有数据可读，继续下一次循环检查心跳
            continue;
        }
        
        // 有数据可读，接收数据包
        Packet receivedPacket; // 创建数据包对象
        
        if (!RecvPacket(clientSocket, receivedPacket)) {
            // 连接断开或接收失败
            WriteLog(LogLevel::INFO_LEVEL, "客户端断开连接: " + clientInfo);
            break;
        }
        
        // 根据消息类型分别处理
        switch (receivedPacket.type()) {
            // 心跳包
            case MsgType::Heartbeat: {
                // 更新心跳时间
                lastHeartbeat = std::chrono::steady_clock::now();
                DebugWriteLog(LogLevel::TRACE_LEVEL, "收到心跳包: " + clientInfo);
                break;
            }
            
            // 创建账号请求
            case MsgType::CreateAcc: {
                // 从header获取userId，从field2获取密码
                uint8_t userId = receivedPacket.getsendid();
                std::string password = receivedPacket.getField2Str();
                
                WriteLog(LogLevel::INFO_LEVEL, "创建账号请求 - 用户ID: " + std::to_string(userId));
                
                // 检查用户ID是否已存在
                bool success;
                if (g_userCredentials.count(userId)) {
                    success = false;
                    WriteLog(LogLevel::INFO_LEVEL, "账号创建失败 - 用户ID已存在: " + std::to_string(userId));
                } else {
                    g_userCredentials[userId] = password;
                    success = true;
                    WriteLog(LogLevel::INFO_LEVEL, "账号创建成功 - 用户ID: " + std::to_string(userId));
                }

                // 构造响应包并发送
                Packet response = Packet::makeRegiRe(success);
                SendPacket(clientSocket, response);
                break;
            }

            // 登录请求
            case MsgType::LoginReq: {
                // 从header获取userId，从field2获取密码
                uint8_t userId = receivedPacket.getsendid();
                std::string password = receivedPacket.getField2Str();
                
                WriteLog(LogLevel::INFO_LEVEL, "登录请求 - 用户ID: " + std::to_string(userId) + ", 来源: " + clientInfo);
                
                // 执行登录与id绑定
                bool logSuccess = LoginConnect(userId, password, sessionPtr);
                
                // 构造响应包
                Packet response = Packet::makeLoginRe(logSuccess);
                if (logSuccess) {
                    WriteLog(LogLevel::INFO_LEVEL, "用户登录成功: " + std::to_string(userId));
                } else {
                    WriteLog(LogLevel::INFO_LEVEL, "用户登录失败: " + std::to_string(userId));
                }
                
                // 发送响应给客户端
                SendPacket(clientSocket, response);
                
                // 如果登录成功，推送离线消息
                if (logSuccess) {
                    // 延迟1秒开始运行
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                    SendOfflineMessages(userId, sessionPtr);
                }
                break;
            }

            // 普通聊天消息
            case MsgType::NormalMsg: { 
                // 检查用户是否已在线
                if (g_userSessions[sessionPtr->userid] == nullptr) {
                    DebugWriteLog(LogLevel::DEBUG_LEVEL, "离线用户尝试发送消息: " + clientInfo);
                    break;
                }
                
                // 从header获取收发信息，从field1获取内容
                uint8_t senderId = receivedPacket.getsendid();
                uint8_t receiverId = receivedPacket.getrecvid();
                std::string content = receivedPacket.getField1Str();
                
                DebugWriteLog(LogLevel::DEBUG_LEVEL, 
                         "收到消息 - 发送者ID: " + std::to_string(senderId) + 
                         ", 接收者ID: " + std::to_string(receiverId) + 
                         ", 内容: " + content);
                
                // 转发消息给接收者
                if (!g_userSessions.count(receiverId)) {
                    WriteLog(LogLevel::TRACE_LEVEL, "发送的消息接收人不存在, 发送者ID: " + std::to_string(senderId));
                } else if (g_userSessions[receiverId] == nullptr) {
                    // 接收者不在线，保存为离线消息
                    g_offlineMessages[receiverId].push_back(receivedPacket);
                    WriteLog(LogLevel::TRACE_LEVEL, 
                             "保存离线消息 - 接收者ID: " + std::to_string(receiverId) + 
                             ", 发送者ID: " + std::to_string(senderId));
                } else {
                    ClientSession* passSession = g_userSessions[receiverId];
                    // 直接转发原始数据包
                    SendPacket(passSession->socket_fd, receivedPacket);
                    DebugWriteLog(LogLevel::TRACE_LEVEL, "消息已转发给用户ID: " + std::to_string(receiverId));
                }
                break;
            }
            

            // 创建群组请求
            case MsgType::CreateGrope: { // chatmsg里是grope所以就grope吧
                if (g_userSessions[sessionPtr->userid] == nullptr) {
                    DebugWriteLog(LogLevel::DEBUG_LEVEL, "未登录用户尝试创建群组");
                    break;
                }
                
                WriteLog(LogLevel::INFO_LEVEL, "创建群组请求 - 发起者: " + sessionPtr->userid);
                
                // TODO: 处理群组创建逻辑
                // 应该新建一个群组类，包含这个群组的id和成员列表，每次收到群聊消息，先检查这个群存不存在（应该不会不存在），
                // 接下来把这条消息转发给这个群组除了发送人以外的其他所有成员（发送人本地应该有这条消息记录）
                // 暂时返回成功
                Packet response = Packet::makeCreGroRe(true);
                SendPacket(clientSocket, response);
                break;
            }
            
            // 添加好友请求
            case MsgType::AddFriendReq: {
                // 检查用户是否已在线
                if (g_userSessions[sessionPtr->userid] == nullptr) {
                    DebugWriteLog(LogLevel::DEBUG_LEVEL, "离线用户尝试发送消息: " + clientInfo);
                    break;
                }
                
                //
                uint8_t senderId = receivedPacket.getsendid();
                uint8_t receiverId = receivedPacket.getrecvid();
                
                DebugWriteLog(LogLevel::DEBUG_LEVEL, 
                         "收到好友请求 - 发送者ID: " + std::to_string(senderId) + 
                         ", 接收者ID: " + std::to_string(receiverId));
                
                // 转发消息给接收者
                if (!g_userSessions.count(receiverId)) {
                    WriteLog(LogLevel::TRACE_LEVEL, "发送的消息接收人不存在, 发送者ID: " + std::to_string(senderId));
                } else if (g_userSessions[receiverId] == nullptr) {
                    // 接收者不在线，保存为离线消息
                    g_offlineMessages[receiverId].push_back(receivedPacket);
                    WriteLog(LogLevel::TRACE_LEVEL, 
                             "好友请求保存到离线列表 - 接收者ID: " + std::to_string(receiverId) + 
                             ", 发送者ID: " + std::to_string(senderId));
                } else {
                    ClientSession* passSession = g_userSessions[receiverId];
                    // 直接转发原始数据包
                    SendPacket(passSession->socket_fd, receivedPacket);
                    DebugWriteLog(LogLevel::TRACE_LEVEL, "请求已转发给用户ID: " + std::to_string(receiverId));
                }
                break;
            }
            
            case MsgType::AddFriendRe: { // 添加好友响应
                // 检查用户是否已在线
                if (g_userSessions[sessionPtr->userid] == nullptr) {
                    DebugWriteLog(LogLevel::DEBUG_LEVEL, "离线用户尝试发送消息: " + clientInfo);
                    break;
                }
                
                // 从header获取收发信息，从field1获取内容
                uint8_t senderId = receivedPacket.getsendid();
                uint8_t receiverId = receivedPacket.getrecvid();
                
                DebugWriteLog(LogLevel::DEBUG_LEVEL, 
                         "收到好友响应 - 发送者ID: " + std::to_string(senderId) + 
                         ", 接收者ID: " + std::to_string(receiverId));
                
                // 转发消息给接收者
                if (!g_userSessions.count(receiverId)) {
                    WriteLog(LogLevel::TRACE_LEVEL, "发送的响应接收人不存在, 发送者ID: " + std::to_string(senderId));
                } else if (g_userSessions[receiverId] == nullptr) {
                    // 接收者不在线，保存为离线消息
                    g_offlineMessages[receiverId].push_back(receivedPacket);
                    WriteLog(LogLevel::TRACE_LEVEL, 
                             "响应保存到离线列表 - 接收者ID: " + std::to_string(receiverId) + 
                             ", 发送者ID: " + std::to_string(senderId));
                } else {
                    ClientSession* passSession = g_userSessions[receiverId];
                    // 直接转发原始数据包
                    SendPacket(passSession->socket_fd, receivedPacket);
                    DebugWriteLog(LogLevel::TRACE_LEVEL, "消息已转发给用户ID: " + std::to_string(receiverId));
                }
                break;
            }
            default: {
                WriteLog(LogLevel::WARN_LEVEL, 
                         "未知消息类型: " + std::to_string(static_cast<int>(receivedPacket.type())));
                break;
            }
        }
    } // 这里是while循环结束的括号
    
    // 清理工作
    WriteLog(LogLevel::INFO_LEVEL, "客户端断开连接: " + clientInfo);
    if (sessionPtr->userid != 0) {
        LogOff(sessionPtr->userid);
    } else {
        delete sessionPtr;
    }
    sessionPtr = nullptr;
    closesocket(clientSocket);
}


int main() {
    // 初始化日志文件
    InitializeLogFile();

    // 标记是否启用debug模式
    if (g_debugMode) {
        WriteLog(LogLevel::INFO_LEVEL, "===debug模式已启用===");
    }
    

    // 初始化WinSock
    if (!InitializeWinSock()) {
        CloseLogFile();
        return 1;
    }

    // 创建Socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::string errormessage = "Socket 创建失败:" + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL_LEVEL, errormessage);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO_LEVEL, "Socket 创建成功");

    // 调用配置地址函数
    sockaddr_in service_address;
    int iResult;
    if (SetupServerAddress(PORT, service_address) != 0) {
        WriteLog(LogLevel::FATAL_LEVEL, "服务器地址配置失败");
        closesocket(listenSocket);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO_LEVEL, "服务器地址配置成功");

    // 执行bind
    iResult = bind(listenSocket, (SOCKADDR*)&service_address, sizeof(service_address));

    if (iResult == SOCKET_ERROR) { // 错误处理
        std::string errormessage = "Bind失败: " + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL_LEVEL, errormessage);
        closesocket(listenSocket);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO_LEVEL, "Socket 成功绑定到端口 " + std::to_string(PORT));

    // 执行listen
    iResult = listen(listenSocket, BACKLOG);

    if(iResult == SOCKET_ERROR) { // 错误处理
        std::string errormessage = "Listen 失败: " + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL_LEVEL, errormessage);
        closesocket(listenSocket);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO_LEVEL, "服务器开始监听连接，端口: " + std::to_string(PORT));

    // accept循环：持续接受客户端连接并为每个客户端分配独立线程
    while (true) {
        sockaddr_in clientAddress;
        int addressLength = sizeof(clientAddress);
        
        // 等待并接受新的客户端连接（会阻塞直到有连接到来）
        SOCKET clientSocket = accept(listenSocket, 
                                     reinterpret_cast<sockaddr*>(&clientAddress), 
                                     &addressLength);
        
        if (clientSocket == INVALID_SOCKET) {
            // accept失败，记录错误但继续监听下一个连接
            std::string errormessage = "Accept 失败: " + std::to_string(WSAGetLastError());
            WriteLog(LogLevel::ERROR_LEVEL, errormessage);
            continue; // 跳过本次循环，继续等待下一个连接
        }
        
        // 获取客户端 IP地址
        char* clientIPAddress = inet_ntoa(clientAddress.sin_addr);
        unsigned short clientPort = ntohs(clientAddress.sin_port);
        
        std::string clientIP = std::string(clientIPAddress);
        WriteLog(LogLevel::INFO_LEVEL, 
                 "接受新连接 - IP: " + clientIP + ", 端口: " + std::to_string(clientPort));
        
        ClientSession* clientSession = nullptr;

        // 创建会话对象到栈里
        clientSession = new ClientSession(clientSocket, clientIP, clientPort);
        DebugWriteLog(LogLevel::DEBUG_LEVEL, "创建新会话对象: " + clientIP);
        
        
        // 为这个客户端创建新线程进行处理
        std::thread clientHandlerThread(HandleClient, clientSession);
        clientHandlerThread.detach(); // 分离线程，使其独立运行，主线程不等待
        
        
        DebugWriteLog(LogLevel::DEBUG_LEVEL, "已为客户端分配独立处理线程");
    }
    
    // 程序正常情况下不会执行到这里（除非手动break跳出循环）
    closesocket(listenSocket);
    CleanupWinSock();
    CloseLogFile();
    
    return 0;
}