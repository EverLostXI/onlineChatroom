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
// 消息封装类
#include "../chatMsg.hpp"
// 需要链接WinSock库
// #pragma comment(lib, "ws2_32.lib") 非MSVC编译器无法识别，必须手动链接Winsock库
// 服务端已经用CMake链接


using namespace std;


// 设置服务器端口号
const int PORT = 8888;
// 设置listen连接队列长度
const int BACKLOG = 5;

// 获取时间戳
string TimeStamp() {
    time_t currentTime = time(nullptr); //不是可直接阅读的日历时间
    tm* localTime = localtime(&currentTime); //转换为服务器本地时间
    stringstream ss; // 开始生成时间戳
    ss << "[" << put_time(localTime, "%Y-%m-%d %H:%M:%S") << "]";
    return ss.str();
}


// 初始化服务器日志
ofstream logFile; // 初始化文件句柄

string FileNameGen() { // 将日志以当前日期命名,逻辑与获取时间戳函数一致
    time_t currentTime = time(nullptr);
    tm* localTime = localtime(&currentTime);
    stringstream fname;
    fname << put_time(localTime, "%Y%m%d");
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

string LevelToString(LogLevel level) { // 将日志级别转为字符串
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

void WriteLog(LogLevel level, const string& message) { //包含两个参数：重要级，消息内容
    if (logFile.is_open()) {
        logFile << TimeStamp() << "[" << LevelToString(level) << "]" << message << endl;
        logFile.flush(); //立即刷新缓冲区
    }
}

void InitializeLogFile() { // 生成文件
    const string LOG_FOLDER = "log/";
    string logfilename = LOG_FOLDER + FileNameGen();
    logFile.open(logfilename, ios::out | ios::app);
    if (!logFile.is_open()) {
        cerr << "无法打开日志文件：" << logfilename << endl; // 此时无法写入日志，因为日志都没打开
    } else {
        string message = "日志文件初始化成功：" + logfilename;
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
        string errormessage = "WSA启动失败:" + to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL_LEVEL, errormessage);
        return false;
    }
    WriteLog(LogLevel::DEBUG_LEVEL, "WinSock 2.2 初始化成功");
    return true;
}

void CleanupWinSock() {
    WSACleanup();
    WriteLog(LogLevel::DEBUG_LEVEL, "WinSock 清理完成");
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


// 用户类
class ClientSession {
public:
    // 网络属性
    SOCKET socket_fd;
    string client_ip;
    unsigned short client_port;
    // 认证状态
    string username;
    bool is_authenticated;

public:
    // 构造函数：初始化用户属性
    ClientSession(SOCKET fd, const string& ip, unsigned short port)
        : socket_fd(fd), 
        client_ip(ip), 
        client_port(port), 
        username(""), // 初始为空
        is_authenticated(false) // 初始未认证
    { // 向日志中记录用户连接
        string logmessage = "客户端连接: IP = " + client_ip + ", 端口 = " + to_string(client_port);
        WriteLog(LogLevel::INFO_LEVEL, logmessage);
    }
    // 认证方法
    void authenticate(const string& user) {
        this->username = user;
        this->is_authenticated = true;
    }

};

// 创建账户：要求用户输入账号和密码，将账号密码作为键值对存入g_userCredentials，再创建一个用户类并认证
/* 账户登录：用户输出账号密码，比对g_userCredentials中的键值对，如果成功，将会话状态标记为已认证，
后续服务器通过检查会话状态来判断该客户端是否有权发送消息*/
// 然后将输入的账号与新创建的用户对象绑定，这样其他人发消息过来的时候可以使用id指明接收人

/*注意：现在一旦服务器关停，所有的账户信息都会被抹除，如果想要永久记录，除非手
动输入，否则可能需要通过构建数据库例如使用SQLite，然而这样工作量就太大了，因此
目前只能先这样了。当然也可以通过读取文件的方式在重启服务器的时候重新创建，然而
那样并不是一个很好的解决方案，如果用户量增多，重新创建用户对象的时间会难以估量
*/

// 存储ID与密码 （可以在客户端部署对密码的不可逆加密）
map<string, string> g_userCredentials = {

};

//将ID与用户对象对应
map<string, ClientSession> g_userSessions = {

};


// 验证登录凭证函数
bool AuthenticateCredential(const string &inputUsername, const string &inputPassword) {
    if (g_userCredentials.count(inputUsername)) { // 检查map中是否存有该用户名
        if (g_userCredentials[inputUsername] == inputPassword) { // 如果存在，比较密码
            return true;
        }
    }
    return false;
}


// 工作线程入口函数：为每个客户端分配独立线程处理消息
void HandleClient(ClientSession* sessionPtr) { // 这个会话指针（sessionPtr)作为一个客户端在内存中的唯一代表
    SOCKET clientSocket = sessionPtr->socket_fd;
    string clientInfo = sessionPtr->client_ip + ":" + to_string(sessionPtr->client_port); // 读取这个连接的ip和端口
    
    WriteLog(LogLevel::DEBUG_LEVEL, "客户端处理线程启动: " + clientInfo);
    
    // 消息接收循环：持续接收并处理客户端消息
    while (true) {
        Packet receivedPacket; // 创建数据包对象
        
        // 接收完整数据包（会阻塞直到收到完整消息或连接断开）
        if (!receivedPacket.recvFrom(clientSocket)) {
            // 连接断开或接收失败
            WriteLog(LogLevel::INFO_LEVEL, "客户端断开连接: " + clientInfo);
            break;
        }
        
        
        // 根据消息类型分别处理
        switch (receivedPacket.type()) {
            case MsgType::LoginReq: { // 登录请求
                // 读取用户名和密码
                string username = receivedPacket.readStr();
                string password = receivedPacket.readStr();
                
                WriteLog(LogLevel::INFO_LEVEL, "登录请求 - 用户名: " + username + ", 来源: " + clientInfo);
                
                // 验证登录凭证
                bool authSuccess = AuthenticateCredential(username, password); // TODO: 调用 AuthenticateCredential 验证
                
                // 构造响应包
                Packet response(MsgType::LoginReq);
                if (authSuccess) {
                    response.writeStr("success");
                    sessionPtr->authenticate(username); // 标记会话为已认证
                    WriteLog(LogLevel::INFO_LEVEL, "用户认证成功: " + username);
                } else {
                    response.writeStr("failed");
                    WriteLog(LogLevel::WARN_LEVEL, "用户认证失败: " + username);
                }
                response.finish();
                
                // 发送响应给客户端
                send(clientSocket, response.data(), response.size(), 0);
                break;
            }
            
            case MsgType::CreateAcc: { // 创建账号请求
                string username = receivedPacket.readStr();
                string password = receivedPacket.readStr();
                
                WriteLog(LogLevel::INFO_LEVEL, "创建账号请求 - 用户名: " + username);
                
                // 检查用户名是否已存在
                Packet response(MsgType::CreateAcc);
                if (g_userCredentials.count(username)) {
                    response.writeStr("failed");
                    response.writeStr("用户名已存在");
                    WriteLog(LogLevel::WARN_LEVEL, "账号创建失败：用户名已存在 - " + username);
                } else {
                    g_userCredentials[username] = password;
                    response.writeStr("success");
                    response.writeStr("账号创建成功");
                    WriteLog(LogLevel::INFO_LEVEL, "账号创建成功 - 用户名: " + username);
                }

                // 发送响应包
                response.finish();
                send(clientSocket, response.data(), response.size(), 0);
                break;
            }
            
            case MsgType::NormalMsg: { // 普通聊天消息
                // 检查用户是否已认证
                if (!sessionPtr->is_authenticated) {
                    WriteLog(LogLevel::WARN_LEVEL, "未认证用户尝试发送消息: " + clientInfo);
                    break;
                }
                
                string content = receivedPacket.readStr();
                string receiver = receivedPacket.readStr();
                
                WriteLog(LogLevel::INFO_LEVEL, 
                         "收到消息 - 发送者: " + sessionPtr->username + 
                         ", 接收者: " + receiver + 
                         ", 内容: " + content);
                
                // 检查是否有回复ID
                if (receivedPacket.isReply()) {
                    uint32_t replyId = receivedPacket.replyMsgId();
                    WriteLog(LogLevel::DEBUG_LEVEL, "这是对消息ID " + to_string(replyId) + " 的回复");
                }
                
                // 检查是否有@列表
                vector<uint32_t> mentions = receivedPacket.atList();
                if (!mentions.empty()) {
                    WriteLog(LogLevel::DEBUG_LEVEL, "消息中提到了 " + to_string(mentions.size()) + " 个用户");
                }
                
                // TODO: 转发消息给接收者
                // 需要在g_userSessions中查找接收者的socket并转发
                break;
            }
            
            case MsgType::CreateGroup: { // 创建群组请求
                if (!sessionPtr->is_authenticated) {
                    WriteLog(LogLevel::WARN_LEVEL, "未认证用户尝试创建群组");
                    break;
                }
                
                WriteLog(LogLevel::INFO_LEVEL, "创建群组请求 - 发起者: " + sessionPtr->username);
                
                // TODO: 处理群组创建逻辑
                break;
            }
            
            case MsgType::Beat: { // 心跳包
                // 心跳包已更新last_heartbeat_time，无需其他处理
                WriteLog(LogLevel::TRACE_LEVEL, "收到心跳包: " + clientInfo);
                break;
            }
            
            default: {
                WriteLog(LogLevel::WARN_LEVEL, 
                         "未知消息类型: " + to_string(static_cast<int>(receivedPacket.type())));
                break;
            }
        }
    }
    
    // 清理工作：关闭socket并释放会话对象
    WriteLog(LogLevel::INFO_LEVEL, "客户端线程结束: " + clientInfo);
    closesocket(clientSocket);
    delete sessionPtr; // 释放动态分配的会话对象
}
// 注：消息封装和解包功能已由 chatMsg.hpp 中的 Packet 类实现
// Packet 类提供了更完善的消息封装、网络字节序转换、以及接收功能


int main() {
    // 初始化日志文件
    InitializeLogFile();

    // 初始化WinSock
    if (!InitializeWinSock()) {
        CloseLogFile();
        return 1;
    }

    // 创建Socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        string errormessage = "Socket 创建失败:" + to_string(WSAGetLastError());
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
        string errormessage = "Bind失败: " + to_string(WSAGetLastError());
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
        string errormessage = "Listen 失败: " + to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL_LEVEL, errormessage);
        closesocket(listenSocket);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO_LEVEL, "服务器开始监听连接，端口: " + to_string(PORT));

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
            string errormessage = "Accept 失败: " + to_string(WSAGetLastError());
            WriteLog(LogLevel::ERROR_LEVEL, errormessage);
            continue; // 跳过本次循环，继续等待下一个连接
        }
        
        // 获取客户端 IP地址
        char* clientIPAddress = inet_ntoa(clientAddress.sin_addr);
        unsigned short clientPort = ntohs(clientAddress.sin_port);
        
        WriteLog(LogLevel::INFO_LEVEL, 
                 "接受新连接 - IP: " + string(clientIPAddress) + ", 端口: " + to_string(clientPort));
        
        // 创建客户端会话对象（动态分配，将在线程结束时释放）
        ClientSession* clientSession = new ClientSession(clientSocket, 
                                                         string(clientIPAddress), 
                                                         clientPort);
        
        // 为这个客户端创建新线程进行处理
        thread clientHandlerThread(HandleClient, clientSession);
        clientHandlerThread.detach(); // 分离线程，使其独立运行，主线程不等待
        
        WriteLog(LogLevel::DEBUG_LEVEL, "已为客户端分配独立处理线程");
    }
    
    // 程序正常情况下不会执行到这里（除非手动break跳出循环）
    closesocket(listenSocket);
    CleanupWinSock();
    CloseLogFile();
    
    return 0;
}