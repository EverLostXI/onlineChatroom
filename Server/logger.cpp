
#include "headers/logger.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>

// 全局变量定义
std::ofstream logFile;
bool g_debugMode = true;

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
        case LogLevel::BEAT: return "BEAT";
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

// 调试模式日志函数, 只有打开的时候才会记录PASS和BEAT类型的日志
void DebugWriteLog(LogLevel level, const std::string& message) {
    if (level == LogLevel::BEAT || level == LogLevel::PASS) {
        if (!g_debugMode) return;
    }
    WriteLog(level, message);
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
