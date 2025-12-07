#pragma once
#include <string>
#include <fstream>

// 日志级别枚举

enum class LogLevel {
    INFO, // 服务器正常运行步骤
    WARN, // 不会导致服务器无法运行的潜在错误
    FATAL, // 导致服务器无法运行的错误
    PASS, // 服务器只负责转发的消息
    PROCESS, // 需要服务器运算的消息
    CONNECTION, // 客户端连接有关
    BEAT // 心跳
};

// 全局日志文件句柄
extern std::ofstream logFile;

// 全局调试模式开关
extern bool g_debugMode;

// 函数声明
std::string TimeStamp();                          // 获取时间戳
std::string FileNameGen();                        // 生成日志文件名
std::string LevelToString(LogLevel level);        // 日志级别转字符串
void InitializeLogFile();                         // 初始化日志文件
void WriteLog(LogLevel level, const std::string& message);  // 写入日志
void DebugWriteLog(LogLevel level, const std::string& message); // 调试模式日志
void CloseLogFile();                              // 关闭日志文件
