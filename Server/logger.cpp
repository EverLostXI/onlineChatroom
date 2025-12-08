
#include "headers/logger.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <vector>

// 全局变量定义
std::ofstream logFile;


// UI日志缓冲区
std::vector<std::string> g_forwardMsgBuffer;
std::vector<std::string> g_requestMsgBuffer;
std::vector<std::string> g_uiLogBuffer;
std::mutex g_forwardMsgMutex;  // 转发消息专用锁
std::mutex g_requestMsgMutex;  // 请求消息专用锁
std::mutex g_uiLogMutex;       // 日志缓冲区专用锁

// 获取时间戳
std::string TimeStamp() {
    time_t currentTime = time(nullptr); //不是可直接阅读的日历时间
    tm* localTime = localtime(&currentTime); //转换为服务器本地时间
    std::stringstream ss; // 开始生成时间戳
    ss << "[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "]";
    return ss.str();
}

std::string FileNameGen() { // 将日志以当前日期命名，但是不包含具体时间，逻辑与获取时间戳函数一致
    time_t currentTime = time(nullptr);
    tm* localTime = localtime(&currentTime);
    std::stringstream fname;
    fname << std::put_time(localTime, "%Y%m%d");
    fname << ".log";
    return fname.str();
}

// 日志级别转字符串
std::string LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::FATAL: return "FATAL";
        case LogLevel::PASS: return "PASS";
        case LogLevel::PROCESS: return "PROCESS";
        case LogLevel::CONNECTION: return "CONNECTION";
        default: return "UNKNOWN";
    }
}
// 日志写入函数
void WriteLog(LogLevel level, const std::string& message) { //包含两个参数：重要级，消息内容
    std::string fullLog = TimeStamp() + "[" + LevelToString(level) + "]" + message;
    
    if (logFile.is_open()) {
        logFile << fullLog << std::endl;
        logFile.flush(); //立即刷新缓冲区
    }
    
    // 添加到UI日志缓冲区
    if (level == LogLevel::PASS) {
        std::lock_guard<std::mutex> lock(g_forwardMsgMutex);
        g_forwardMsgBuffer.push_back(TimeStamp() + " " + message);
        if (g_forwardMsgBuffer.size() > 500) {
        g_forwardMsgBuffer.erase(g_forwardMsgBuffer.begin());
        }
    } else if (level == LogLevel::PROCESS) {
        std::lock_guard<std::mutex> lock(g_requestMsgMutex);
        g_requestMsgBuffer.push_back(TimeStamp() + " " + message);
        if (g_requestMsgBuffer.size() > 500) {
        g_requestMsgBuffer.erase(g_requestMsgBuffer.begin());
        }
    } else {
        std::lock_guard<std::mutex> lock(g_uiLogMutex);
        g_uiLogBuffer.push_back(fullLog);
        if (g_uiLogBuffer.size() > 1000) {
        g_uiLogBuffer.erase(g_uiLogBuffer.begin());
        }
    }
}


void InitializeLogFile() {
    const std::string LOG_FOLDER = "log/";
    std::string logfilename = LOG_FOLDER + FileNameGen();
    logFile.open(logfilename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "无法打开日志文件：" << logfilename << std::endl; // 此时无法写入日志（当然）
    } else {
        std::string message = "日志文件初始化成功：" + logfilename;
        WriteLog(LogLevel::INFO, message);
    }
}

void CloseLogFile() {
    if (logFile.is_open()) {
        WriteLog(LogLevel::INFO, "关闭日志文件");
        logFile.close();
    }
}
