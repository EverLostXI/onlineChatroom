#include "headers/socket.h"
#include "headers/logger.h"
#include <cstring>
#include <vector>

// 获取本机ip地址函数(要在创建了socket之后才调用)
std::string GetServerIP() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        return "127.0.0.1";
    }

    struct hostent* host = gethostbyname(hostname);
    if (host == nullptr || host->h_addr_list[0] == nullptr) {
        return "127.0.0.1";
    }

    struct in_addr addr;
    memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
    return std::string(inet_ntoa(addr));
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
