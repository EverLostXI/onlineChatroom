#pragma once
#include <string>
#include <fstream>

// 日志级别枚举
enum class LogLevel { 
    FATAL_LEVEL, // 系统崩溃或不可恢复的错误
    ERROR_LEVEL, // 运行时错误，功能受影响但程序仍在运行
    WARN_LEVEL,  // 潜在的问题或异常情况
    INFO_LEVEL,  // 程序正常运行的关键步骤
    DEBUG_LEVEL, // 详细的调试信息
    TRACE_LEVEL  // 追踪细致操作
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
