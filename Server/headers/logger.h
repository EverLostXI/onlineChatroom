#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <mutex>

// 日志级别枚举

enum class LogLevel {
    INFO, // 服务器正常运行步骤
    WARN, // 不会导致服务器无法运行的潜在错误
    FATAL, // 导致服务器无法运行的错误
    PASS, // 服务器只负责转发的消息
    PROCESS, // 需要服务器运算的消息
    CONNECTION, // 客户端连接有关
};

// 全局日志文件句柄
extern std::ofstream logFile;



// UI日志缓冲区（供Monitor显示）
extern std::vector<std::string> g_forwardMsgBuffer;
extern std::vector<std::string> g_requestMsgBuffer;
extern std::vector<std::string> g_uiLogBuffer;
extern std::mutex g_forwardMsgMutex;
extern std::mutex g_requestMsgMutex;
extern std::mutex g_uiLogMutex;

// 函数声明
std::string TimeStamp();                          // 获取时间戳
std::string FileNameGen();                        // 生成日志文件名
std::string LevelToString(LogLevel level);        // 日志级别转字符串
void InitializeLogFile();                         // 初始化日志文件
void WriteLog(LogLevel level, const std::string& message);  // 写入日志
void DebugWriteLog(LogLevel level, const std::string& message); // 调试模式日志
void CloseLogFile();                              // 关闭日志文件
