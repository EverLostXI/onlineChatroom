#include <iostream>
#include <string>
#include <cstring>
#include <thread>
// 网络连接部分
#include <winsock2.h>
#include <ws2tcpip.h>
// 消息封装类（服务器专用版本，不依赖Qt）
#include "chatMsg_server.hpp"
// 引入各模块头文件
#include "headers/logger.h"
#include "headers/socket.h"
#include "headers/userControl.h"
#include "headers/handleClient.h"
#include "headers/Monitor.h"

// TODO: 添加监视窗口，包含用户列表，群聊列表，在线客户端列表，usersession跟踪，手动控制map绑定

// 服务器配置常量
extern const int PORT = 8888;
extern const int BACKLOG = 5;
extern const int HEARTBEAT_TIMEOUT = 30;


int main() {
    // 启动UI监视窗口线程
    std::thread uiThread(RunMonitorUI);
    uiThread.detach();  // 独立运行

    // 初始化日志文件
    InitializeLogFile();

    WriteLog(LogLevel::INFO, "监视窗口已启动");

    // 初始化WinSock
    if (!InitializeWinSock()) {
        CloseLogFile();
        return 1;
    }

    // 创建Socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::string errormessage = "Socket 创建失败:" + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL, errormessage);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO, "Socket 创建成功");

    // 获取本机IP地址（方便给客户端看）
    std::string serverIP = GetServerIP();
    WriteLog(LogLevel::INFO, "服务器IP: " + serverIP);

    // 调用配置地址函数
    sockaddr_in service_address;
    int iResult;
    if (SetupServerAddress(PORT, service_address) != 0) {
        WriteLog(LogLevel::FATAL, "服务器地址配置失败");
        closesocket(listenSocket);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO, "服务器地址配置成功");

    // 执行bind
    iResult = bind(listenSocket, (SOCKADDR*)&service_address, sizeof(service_address));

    if (iResult == SOCKET_ERROR) { // 错误处理
        std::string errormessage = "Bind失败: " + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL, errormessage);
        closesocket(listenSocket);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO, "Socket 成功绑定到端口");

    // 执行listen
    iResult = listen(listenSocket, BACKLOG);

    if(iResult == SOCKET_ERROR) { // 错误处理
        std::string errormessage = "Listen 失败: " + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL, errormessage);
        closesocket(listenSocket);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO, "服务器开始监听连接，端口: " + std::to_string(PORT));

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
            WriteLog(LogLevel::WARN, errormessage);
            continue; // 跳过本次循环，继续等待下一个连接
        }
        
        // 获取客户端 IP地址
        char* clientIPAddress = inet_ntoa(clientAddress.sin_addr);
        unsigned short clientPort = ntohs(clientAddress.sin_port);
        
        std::string clientIP = std::string(clientIPAddress);
        WriteLog(LogLevel::CONNECTION, 
                 "接受新连接来自IP: " + clientIP + ", 端口: " + std::to_string(clientPort));
        
        ClientSession* clientSession = nullptr;

        // 创建会话对象到栈里
        clientSession = new ClientSession(clientSocket, clientIP, clientPort);
        WriteLog(LogLevel::CONNECTION, "创建新会话对象: " + clientIP);
        
        
        // 为这个客户端创建新线程进行处理
        std::thread clientHandlerThread(HandleClient, clientSession);
        clientHandlerThread.detach(); // 分离线程，使其独立运行，主线程不等待

    }
    
    // 程序正常情况下不会执行到这里（除非手动break跳出循环）
    closesocket(listenSocket);
    CleanupWinSock();
    CloseLogFile();
    
    return 0;
}