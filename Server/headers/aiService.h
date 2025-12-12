#pragma once
#include <string>

// AI服务配置
struct AIConfig {
    std::string apiKey;        // API密钥
    std::string apiEndpoint;   // API端点URL
    std::string model;         // 使用的模型名称
    int maxTokens;             // 最大token数
    float temperature;         // 温度参数(0.0-1.0)
};

// AI服务类
class AIService {
public:
    // 构造函数
    AIService();
    explicit AIService(const AIConfig& config);
    
    // 配置AI服务
    void Configure(const AIConfig& config);
    
    // 发送消息给AI并获取回复
    // 参数：userMessage - 用户发送的消息
    // 返回：AI的回复字符串，如果失败返回空字符串
    std::string GetAIResponse(const std::string& userMessage);
    
    // 检查服务是否已配置
    bool IsConfigured() const;
    
private:
    AIConfig config_;
    bool configured_;
    
    // 发送HTTP请求的辅助函数
    std::string SendHttpRequest(const std::string& url, const std::string& postData);
    
    // 构造AI API请求的JSON
    std::string BuildRequestJSON(const std::string& userMessage);
    
    // 解析AI API响应的JSON
    std::string ParseResponseJSON(const std::string& responseJson);
};

// 全局AI服务实例
extern AIService g_aiService;

// 初始化AI服务（在程序启动时调用）
void InitializeAIService();
