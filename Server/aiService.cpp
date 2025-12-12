#include "headers/aiService.h"
#include "headers/logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <curl/curl.h>

// 全局AI服务实例
AIService g_aiService;

// 构造函数
AIService::AIService() : configured_(false) {
    config_.apiKey = "";
    config_.apiEndpoint = "https://api.openai.com/v1/chat/completions";
    config_.model = "gpt-3.5-turbo";
    config_.maxTokens = 500;
    config_.temperature = 0.7f;
}

AIService::AIService(const AIConfig& config) 
    : config_(config), configured_(!config.apiKey.empty()) {
}

void AIService::Configure(const AIConfig& config) {
    config_ = config;
    configured_ = !config_.apiKey.empty();
    
    if (configured_) {
        WriteLog(LogLevel::INFO, "AI服务已配置 - 模型: " + config_.model);
    }
}

bool AIService::IsConfigured() const {
    return configured_;
}

std::string AIService::GetAIResponse(const std::string& userMessage) {
    if (!configured_) {
        WriteLog(LogLevel::WARN, "AI服务未配置，无法获取回复");
        return "AI服务未配置，请联系管理员。";
    }
    
    if (userMessage.empty()) {
        return "消息不能为空。";
    }
    
    WriteLog(LogLevel::INFO, "发送AI请求: " + userMessage);
    
    try {
        // 构造请求JSON
        std::string requestJson = BuildRequestJSON(userMessage);
        
        // 发送HTTP请求
        std::string response = SendHttpRequest(config_.apiEndpoint, requestJson);
        
        if (response.empty()) {
            WriteLog(LogLevel::WARN, "AI API响应为空");
            return "抱歉，AI服务暂时不可用。";
        }
        
        // 解析响应JSON
        std::string aiReply = ParseResponseJSON(response);
        
        if (aiReply.empty()) {
            WriteLog(LogLevel::WARN, "无法解析AI响应");
            return "抱歉，无法理解AI的回复。";
        }
        
        WriteLog(LogLevel::INFO, "AI回复成功");
        return aiReply;
        
    } catch (const std::exception& e) {
        WriteLog(LogLevel::FATAL, std::string("AI服务异常: ") + e.what());
        return "抱歉，处理您的请求时出现错误。";
    }
}

std::string AIService::BuildRequestJSON(const std::string& userMessage) {
    // 转义JSON特殊字符
    std::string escapedMessage = userMessage;
    size_t pos = 0;
    while ((pos = escapedMessage.find("\"", pos)) != std::string::npos) {
        escapedMessage.replace(pos, 1, "\\\"");
        pos += 2;
    }
    pos = 0;
    while ((pos = escapedMessage.find("\n", pos)) != std::string::npos) {
        escapedMessage.replace(pos, 1, "\\n");
        pos += 2;
    }
    
    // 构造JSON请求
    std::ostringstream json;
    json << "{"
         << "\"model\":\"" << config_.model << "\","
         << "\"messages\":[{\"role\":\"user\",\"content\":\"" << escapedMessage << "\"}],"
         << "\"max_tokens\":" << config_.maxTokens << ","
         << "\"temperature\":" << config_.temperature
         << "}";
    
    return json.str();
}

std::string AIService::ParseResponseJSON(const std::string& responseJson) {
    // 简单的JSON解析 - 查找 "content": "..."
    size_t contentPos = responseJson.find("\"content\"");
    if (contentPos == std::string::npos) {
        return "";
    }
    
    // 找到内容开始的引号
    size_t startQuote = responseJson.find("\"", contentPos + 9);
    if (startQuote == std::string::npos) {
        return "";
    }
    
    // 找到内容结束的引号（需要跳过转义的引号）
    size_t endQuote = startQuote + 1;
    while (endQuote < responseJson.length()) {
        if (responseJson[endQuote] == '\"' && responseJson[endQuote - 1] != '\\') {
            break;
        }
        endQuote++;
    }
    
    if (endQuote >= responseJson.length()) {
        return "";
    }
    
    // 提取内容
    std::string content = responseJson.substr(startQuote + 1, endQuote - startQuote - 1);
    
    // 反转义特殊字符
    size_t pos = 0;
    while ((pos = content.find("\\n", pos)) != std::string::npos) {
        content.replace(pos, 2, "\n");
        pos += 1;
    }
    pos = 0;
    while ((pos = content.find("\\\"", pos)) != std::string::npos) {
        content.replace(pos, 2, "\"");
        pos += 1;
    }
    
    return content;
}

std::string AIService::SendHttpRequest(const std::string& url, const std::string& postData) {
    CURL* curl;
    CURLcode res;
    std::string responseBuffer;
    
    // 初始化curl
    curl = curl_easy_init();
    if (!curl) {
        WriteLog(LogLevel::FATAL, "无法初始化CURL");
        return "";
    }
    
    // 设置回调函数用于接收响应数据
    auto writeCallback = [](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
        size_t totalSize = size * nmemb;
        std::string* buffer = static_cast<std::string*>(userdata);
        buffer->append(ptr, totalSize);
        return totalSize;
    };
    
    // 设置请求选项
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
    
    // 设置HTTP头
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string authHeader = "Authorization: Bearer " + config_.apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // 设置SSL验证（生产环境应该启用）
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // 设置超时
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    
    // 执行请求
    res = curl_easy_perform(curl);
    
    // 清理
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) {
        WriteLog(LogLevel::FATAL, std::string("CURL请求失败: ") + curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return "";
    }
    
    // 获取HTTP响应码
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    
    curl_easy_cleanup(curl);
    
    if (httpCode != 200) {
        WriteLog(LogLevel::WARN, "AI API返回非200状态码: " + std::to_string(httpCode));
        WriteLog(LogLevel::WARN, "响应内容: " + responseBuffer);
        return "";
    }
    
    return responseBuffer;
}

void InitializeAIService() {
    // 初始化CURL库（全局初始化，只需要一次）
    curl_global_init(CURL_GLOBAL_ALL);
    
    // 从配置文件读取配置
    AIConfig config;
    
    std::ifstream configFile("ai_config.txt");
    if (!configFile.is_open()) {
        WriteLog(LogLevel::FATAL, "未找到ai_config.txt配置文件，AI服务无法启动");
        WriteLog(LogLevel::FATAL, "请创建ai_config.txt并填写API密钥等配置信息");
        return;
    }
    
    // 读取配置文件
    std::string line;
    config.apiKey = "";
    config.apiEndpoint = "";
    config.model = "";
    config.maxTokens = 0;
    config.temperature = 0.0f;
    
    while (std::getline(configFile, line)) {
        // 跳过注释和空行
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // 解析键值对
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // 去除首尾空格
            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            
            if (key == "API_KEY") {
                config.apiKey = value;
            } else if (key == "API_ENDPOINT") {
                config.apiEndpoint = value;
            } else if (key == "MODEL") {
                config.model = value;
            } else if (key == "MAX_TOKENS") {
                try {
                    config.maxTokens = std::stoi(value);
                } catch (...) {
                    WriteLog(LogLevel::WARN, "MAX_TOKENS配置无效，使用默认值500");
                    config.maxTokens = 500;
                }
            } else if (key == "TEMPERATURE") {
                try {
                    config.temperature = std::stof(value);
                } catch (...) {
                    WriteLog(LogLevel::WARN, "TEMPERATURE配置无效，使用默认值0.7");
                    config.temperature = 0.7f;
                }
            }
        }
    }
    configFile.close();
    
    // 验证必需配置
    if (config.apiKey.empty() || config.apiKey == "your-api-key-here") {
        WriteLog(LogLevel::FATAL, "配置文件错误：API_KEY未配置或无效");
        return;
    }
    if (config.apiEndpoint.empty()) {
        WriteLog(LogLevel::FATAL, "配置文件错误：API_ENDPOINT未配置");
        return;
    }
    if (config.model.empty()) {
        WriteLog(LogLevel::FATAL, "配置文件错误：MODEL未配置");
        return;
    }
    if (config.maxTokens <= 0) {
        WriteLog(LogLevel::WARN, "配置文件错误：MAX_TOKENS无效，使用默认值500");
        config.maxTokens = 500;
    }
    if (config.temperature < 0.0f || config.temperature > 1.0f) {
        WriteLog(LogLevel::WARN, "配置文件错误：TEMPERATURE无效，使用默认值0.7");
        config.temperature = 0.7f;
    }
    
    g_aiService.Configure(config);
    WriteLog(LogLevel::INFO, "AI服务初始化完成");
}
