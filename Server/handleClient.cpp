#include "headers/handleClient.h"
#include "headers/socket.h"
#include "headers/logger.h"
#include "headers/userControl.h"
#include <chrono>
#include <thread>

// 外部常量声明（定义在 main.cpp）
extern const int HEARTBEAT_TIMEOUT;

// 工作线程入口函数：为每个客户端分配独立线程处理消息
void HandleClient(ClientSession* sessionPtr) { // 这个会话指针（sessionPtr)作为一个客户端在内存中的唯一代表
    SOCKET clientSocket = sessionPtr->socket_fd;
    std::string clientInfo = sessionPtr->client_ip + ":" + std::to_string(sessionPtr->client_port); // 读取这个连接的ip和端口
    DebugWriteLog(LogLevel::DEBUG_LEVEL, "客户端处理线程启动: " + clientInfo);

    // 记录最后一次心跳时间
    auto lastHeartbeat = std::chrono::steady_clock::now();

    // 消息接收循环，持续接收并处理客户端消息
    // 逻辑是：如果socket没收到数据，我就一直检查心跳是否超时，如果有数据，我再读是什么数据，再决定要干啥
    // 判断socket中有没有数据，用的是select函数
    while (true) {
        // 检查心跳超时
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastHeartbeat); // 距离上次心跳的时间间隔
        if (duration.count() > HEARTBEAT_TIMEOUT) {
            WriteLog(LogLevel::INFO_LEVEL, "客户端心跳超时，断开连接: " + clientInfo + 
                     " (超时: " + std::to_string(HEARTBEAT_TIMEOUT) + "秒)");
            break;
        }
        
        // 使用select检测是否有数据可读（超时1秒）
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(clientSocket, &readSet);
        // 1秒超时
        timeval timeout; // 为什么不设置成全局变量方便控制：select会修改timeout的数值，所以只能每个客户端分一个分别计算
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int selectResult = select(0, &readSet, nullptr, nullptr, &timeout);
        
        if (selectResult == SOCKET_ERROR) {
            WriteLog(LogLevel::ERROR_LEVEL, "Select错误: " + clientInfo);
            break;
        }
        
        if (selectResult == 0) {
            // select超时，没有数据可读，继续下一次循环检查心跳
            continue;
        }
        
        // 有数据可读，接收数据包
        Packet receivedPacket; // 创建数据包对象
        
        if (!RecvPacket(clientSocket, receivedPacket)) {
            // 连接断开或接收失败
            WriteLog(LogLevel::INFO_LEVEL, "客户端断开连接: " + clientInfo);
            break;
        }
        
        // 根据消息类型分别处理
        switch (receivedPacket.type()) {
            // 心跳包
            case MsgType::Heartbeat: {
                // 更新心跳时间
                lastHeartbeat = std::chrono::steady_clock::now();
                DebugWriteLog(LogLevel::TRACE_LEVEL, "收到心跳包: " + clientInfo);
                break;
            }
            
            // 注册请求
            case MsgType::CreateAcc: {
                // 从header获取userID，从field2获取密码
                uint8_t userID = receivedPacket.getsendid();
                std::string password = receivedPacket.getField2Str();
                
                WriteLog(LogLevel::INFO_LEVEL, "创建账号请求 - 用户ID: " + std::to_string(userID));
                
                // 调用注册函数
                bool success = Signup(userID, password);
                
                // 构造响应包并发送
                Packet response = Packet::makeRegiRe(success);
                SendPacket(clientSocket, response);
                break;
            }

            // 登录请求
            case MsgType::LoginReq: {
                // 从header获取userID，从field2获取密码
                uint8_t userID = receivedPacket.getsendid();
                std::string password = receivedPacket.getField2Str();
                
                WriteLog(LogLevel::INFO_LEVEL, "登录请求 - 用户ID: " + std::to_string(userID) + ", 来源: " + clientInfo);
                
                // 调用登录函数
                bool logSuccess = LoginConnect(userID, password, sessionPtr);
                
                // 构造响应包
                Packet response = Packet::makeLoginRe(logSuccess);
                if (logSuccess) {
                    WriteLog(LogLevel::INFO_LEVEL, "用户登录成功: " + std::to_string(userID));
                } else {
                    WriteLog(LogLevel::INFO_LEVEL, "用户登录失败: " + std::to_string(userID));
                }
                
                // 发送响应给客户端
                SendPacket(clientSocket, response);
                
                // 如果登录成功，推送离线消息
                if (logSuccess) {
                    // 延迟3秒开始运行
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                    SendOfflineMessages(userID, sessionPtr);
                }
                break;
            }

            // 转发添加好友请求
            case MsgType::AddFriendReq: {
                // 检查本会话是否已在线
                if (!CheckOnline(sessionPtr->userid)) {
                    DebugWriteLog(LogLevel::DEBUG_LEVEL, "离线用户尝试发送消息: " + clientInfo);
                    break;
                }
                
                //
                uint8_t senderID = receivedPacket.getsendid();
                uint8_t receiverID = receivedPacket.getrecvid();
                
                DebugWriteLog(LogLevel::DEBUG_LEVEL, 
                         "收到好友请求 - 发送者ID: " + std::to_string(senderID) + 
                         ", 接收者ID: " + std::to_string(receiverID));
                
                // 转发消息给接收者
                if (!g_userSessions.count(receiverID)) {
                    WriteLog(LogLevel::TRACE_LEVEL, "发送的消息接收人不存在, 发送者ID: " + std::to_string(senderID));
                } else if (g_userSessions[receiverID] == nullptr) {
                    // 接收者不在线，保存为离线消息
                    g_offlineMessages[receiverID].push_back(receivedPacket);
                    WriteLog(LogLevel::TRACE_LEVEL, 
                             "好友请求保存到离线列表 - 接收者ID: " + std::to_string(receiverID) + 
                             ", 发送者ID: " + std::to_string(senderID));
                } else {
                    ClientSession* passSession = g_userSessions[receiverID];
                    // 直接转发原始数据包
                    SendPacket(passSession->socket_fd, receivedPacket);
                    DebugWriteLog(LogLevel::TRACE_LEVEL, "请求已转发给用户ID: " + std::to_string(receiverID));
                }
                break;
            }

            // 转发好友响应
            case MsgType::AddFriendRe: { 
                // 检查本会话是否是否已在线
                if (!CheckOnline(sessionPtr->userid)) {
                    DebugWriteLog(LogLevel::DEBUG_LEVEL, "离线用户尝试发送消息: " + clientInfo);
                    break;
                }
                
                // 从header获取收发信息，从field1获取内容
                uint8_t senderID = receivedPacket.getsendid();
                uint8_t receiverID = receivedPacket.getrecvid();
                
                DebugWriteLog(LogLevel::DEBUG_LEVEL, 
                         "收到好友响应 - 发送者ID: " + std::to_string(senderID) + 
                         ", 接收者ID: " + std::to_string(receiverID));
                
                // 转发消息给接收者
                if (!g_userSessions.count(receiverID)) {
                    WriteLog(LogLevel::TRACE_LEVEL, "发送的响应接收人不存在, 发送者ID: " + std::to_string(senderID));
                } else if (g_userSessions[receiverID] == nullptr) {
                    // 接收者不在线，保存为离线消息
                    g_offlineMessages[receiverID].push_back(receivedPacket);
                    WriteLog(LogLevel::TRACE_LEVEL, 
                             "响应保存到离线列表 - 接收者ID: " + std::to_string(receiverID) + 
                             ", 发送者ID: " + std::to_string(senderID));
                } else {
                    ClientSession* passSession = g_userSessions[receiverID];
                    // 直接转发原始数据包
                    SendPacket(passSession->socket_fd, receivedPacket);
                    DebugWriteLog(LogLevel::TRACE_LEVEL, "消息已转发给用户ID: " + std::to_string(receiverID));
                }
                break;
            }

            // 转发私聊聊天消息
            case MsgType::NormalMsg: { 
                // 检查本会话是否已在线
                if (!CheckOnline(sessionPtr->userid)) {
                    DebugWriteLog(LogLevel::DEBUG_LEVEL, "离线用户尝试发送消息: " + clientInfo);
                    break;
                }
                
                // 从header获取收发信息，从field1获取内容
                uint8_t senderID = receivedPacket.getsendid();
                uint8_t receiverID = receivedPacket.getrecvid();
                std::string content = receivedPacket.getField1Str();
                
                DebugWriteLog(LogLevel::DEBUG_LEVEL, 
                         "收到消息 - 发送者ID: " + std::to_string(senderID) + 
                         ", 接收者ID: " + std::to_string(receiverID) + 
                         ", 内容: " + content);
                
                // 转发消息给接收者
                if (!g_userSessions.count(receiverID)) {
                    WriteLog(LogLevel::TRACE_LEVEL, "发送的消息接收人不存在, 发送者ID: " + std::to_string(senderID));
                } else if (g_userSessions[receiverID] == nullptr) {
                    // 接收者不在线，保存为离线消息
                    g_offlineMessages[receiverID].push_back(receivedPacket);
                    WriteLog(LogLevel::TRACE_LEVEL, 
                             "保存离线消息 - 接收者ID: " + std::to_string(receiverID) + 
                             ", 发送者ID: " + std::to_string(senderID));
                } else {
                    ClientSession* passSession = g_userSessions[receiverID];
                    // 直接转发原始数据包
                    SendPacket(passSession->socket_fd, receivedPacket);
                    DebugWriteLog(LogLevel::TRACE_LEVEL, "消息已转发给用户ID: " + std::to_string(receiverID));
                }
                break;
            }
            

            // 创建群组请求
            case MsgType::CreateGrope: { // chatmsg里是grope所以就grope吧
                // 检测本会话是否在线
                if (!CheckOnline(sessionPtr->userid)) {
                    DebugWriteLog(LogLevel::DEBUG_LEVEL, "未登录用户尝试创建群组");
                    break;
                }
                
                WriteLog(LogLevel::INFO_LEVEL, "创建群组请求 - 发起者: " + sessionPtr->userid);
                
                uint8_t creatorID = receivedPacket.getsendid();
                std::vector<uint8_t> memberList = receivedPacket.getField1();
                std::string groupName = receivedPacket.getField2Str();
                // 将创建者添加到成员列表(消息中的成员列表不包含创建者)
                memberList.push_back(creatorID);

                // 调用创建群聊函数
                bool success = CreateGroup(groupName, memberList);

                // 向群聊成员转发创建群聊消息（除了创建者自己）
                if (success) {
                    int forwardCount = 0;
                    int offlineCount = 0;
                    int notExistCount = 0;
                    
                    for (uint8_t memberID : memberList) {
                        // 跳过创建者自己
                        if (memberID == creatorID) {
                            continue;
                        }
                        
                        // 使用 CheckUser 检查成员状态
                        int userStatus = CheckUser(memberID);
                        
                        switch (userStatus) {
                            case 0:  // 用户不存在
                                WriteLog(LogLevel::WARN_LEVEL, 
                                         "群聊成员不存在 - 成员ID: " + std::to_string(memberID));
                                notExistCount++;
                                break;
                                
                            case 1:  // 用户存在但不在线
                                // 保存为离线消息
                                g_offlineMessages[memberID].push_back(receivedPacket);
                                offlineCount++;
                                DebugWriteLog(LogLevel::TRACE_LEVEL,
                                             "群聊创建消息保存到离线队列 - 成员ID: " + std::to_string(memberID));
                                break;
                                
                            case 2:  // 用户在线
                                // 直接转发
                                ClientSession* memberSession = g_userSessions[memberID];
                                SendPacket(memberSession->socket_fd, receivedPacket);
                                forwardCount++;
                                DebugWriteLog(LogLevel::TRACE_LEVEL,
                                             "群聊创建消息已转发 - 成员ID: " + std::to_string(memberID));
                                break;
                        }
                    }
                    
                    WriteLog(LogLevel::INFO_LEVEL, 
                             "群聊创建消息转发完成 - 在线转发: " + std::to_string(forwardCount) + 
                             " 条，离线保存: " + std::to_string(offlineCount) + 
                             " 条，不存在: " + std::to_string(notExistCount) + " 个");
                }

                // 向创建者返回成功
                Packet response = Packet::makeCreGroRe(success);
                SendPacket(clientSocket, response);
                break;
            }
            
            case MsgType::GroupMsg: {
                // 检测本会话是否在线
                if (!CheckOnline(sessionPtr->userid)) {
                    DebugWriteLog(LogLevel::DEBUG_LEVEL, "未登录用户尝试发送群聊消息");
                    break;
                }

                uint8_t senderID = receivedPacket.getsendid();
                std::string groupName = receivedPacket.getField2Str();
                std::string messageContent = receivedPacket.getField1Str();

                DebugWriteLog(LogLevel::DEBUG_LEVEL,
                             "收到群聊消息 - 发送者ID: " + std::to_string(senderID) +
                             ", 群聊名称: " + groupName +
                             ", 内容: " + messageContent);

                // 检查群聊是否存在
                if (!g_groupChat.count(groupName)) {
                    WriteLog(LogLevel::WARN_LEVEL, 
                             "群聊不存在 - 群聊名称: " + groupName + 
                             ", 发送者ID: " + std::to_string(senderID));
                    break;
                }

                // 获取群成员列表
                std::vector<uint8_t>& memberList = g_groupChat[groupName];
                
                // 转发消息给除发送者外的所有群成员
                int forwardCount = 0;
                int offlineCount = 0;
                int notExistCount = 0;
                
                for (uint8_t memberID : memberList) {
                    // 跳过发送者自己
                    if (memberID == senderID) {
                        continue;
                    }
                    
                    // 使用 CheckUser 检查成员状态
                    int userStatus = CheckUser(memberID);
                    
                    switch (userStatus) {
                        case 0:  // 用户不存在
                            WriteLog(LogLevel::WARN_LEVEL, 
                                     "群成员不存在 - 成员ID: " + std::to_string(memberID) +
                                     ", 群聊: " + groupName);
                            notExistCount++;
                            break;
                            
                        case 1:  // 用户存在但不在线
                            // 保存为离线消息
                            g_offlineMessages[memberID].push_back(receivedPacket);
                            offlineCount++;
                            DebugWriteLog(LogLevel::TRACE_LEVEL,
                                         "群聊消息保存到离线队列 - 成员ID: " + std::to_string(memberID));
                            break;
                            
                        case 2:  // 用户在线
                            // 直接转发
                            ClientSession* memberSession = g_userSessions[memberID];
                            SendPacket(memberSession->socket_fd, receivedPacket);
                            forwardCount++;
                            DebugWriteLog(LogLevel::TRACE_LEVEL,
                                         "群聊消息已转发 - 成员ID: " + std::to_string(memberID));
                            break;
                    }
                }
                
                WriteLog(LogLevel::INFO_LEVEL, 
                         "群聊消息转发完成 - 群聊: " + groupName + 
                         ", 在线转发: " + std::to_string(forwardCount) + 
                         " 条，离线保存: " + std::to_string(offlineCount) + 
                         " 条，不存在: " + std::to_string(notExistCount) + " 个");
                break;
            }
            
            default: {
                WriteLog(LogLevel::WARN_LEVEL, 
                         "未知消息类型: " + std::to_string(static_cast<int>(receivedPacket.type())));
                break;
            }
        }
    } // 这里是while循环结束的括号
    
    // 清理工作
    WriteLog(LogLevel::INFO_LEVEL, "客户端断开连接: " + clientInfo);
    if (sessionPtr->userid != 0) {
        LogOff(sessionPtr->userid);
    } else {
        delete sessionPtr;
    }
    sessionPtr = nullptr;
    closesocket(clientSocket);
}
