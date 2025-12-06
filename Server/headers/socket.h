#pragma once
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../chatMsg_server.hpp"

// Socket初始化与清理
bool InitializeWinSock();    // 初始化WinSock
void CleanupWinSock();       // 清理WinSock

// 服务器配置
std::string GetServerIP();   // 获取本机IP地址
int SetupServerAddress(const int port, sockaddr_in& address_info); // 配置服务器地址

// 数据包收发函数
bool RecvPacket(SOCKET sock, Packet& packet);       // 接收数据包
bool SendPacket(SOCKET sock, const Packet& packet); // 发送数据包
