#pragma once
#pragma comment(lib,"ws2_32.lib")
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <winsock2.h>

// 主机序与网络序转换函数
inline uint16_t h2n16(uint16_t v){ return htons(v); }
inline uint32_t h2n32(uint32_t v){ return htonl(v); }
inline uint16_t n2h16(uint16_t v){ return ntohs(v); }
inline uint32_t n2h32(uint32_t v){ return ntohl(v); }

// 消息类型枚举
enum class MsgType : uint8_t
{
    LoginReq    = 0x01,  // 登录请求
    CreateAcc   = 0x02,  // 创建账号
    CreateGrope = 0x03,   //创建群聊
    Loginreturn = 0x04, //登录反馈
    regireturn = 0x05, //注册反馈
    NormalMsg   = 0x10   // 普通消息

};

#pragma pack(push,1)
// 数据包头部结构
struct Header
{
    uint8_t  type;       // 消息类型
    bool  success;      // 成功/失败
    uint8_t  sendid;     // 发送者id
    uint8_t  recvid;     // 接受者id
    uint16_t field1Len;  // 变长区1长度（网络序）
    uint16_t field2Len;  // 变长区2长度（网络序）  
    uint16_t field3Len;  // 变长区3长度（网络序）
    uint16_t field4Len;  // 变长区4长度（网络序）
};
#pragma pack(pop)

// 网络数据包类
class Packet
{
public:
    Packet() = default;
    explicit Packet(MsgType t){ hdr.type = static_cast<uint8_t>(t); }
    //客户端封包
    /* 方法：创建登录请求包 */
    static Packet makeLogin(uint8_t user, const std::string& pwd)//发送id直接作为用户名，大小0~255
    {
        Packet p(MsgType::LoginReq);
        p.hdr.sendid = user;
        p.writeField2(pwd);
        p.finish();
        return p;
    }
    
    /* 方法：创建注册请求包 */
    static Packet makeCreAcc(uint8_t user, const std::string& pwd)
    {
        Packet p(MsgType::CreateAcc);
        p.hdr.sendid = user;
        p.writeField2(pwd);
        p.finish();
        return p;
    }
    /* 方法：创建群聊 */
    static Packet makeCreGro(std::vector<uint8_t>& idlist)//使用getfield1（）获得数据
    {
        Packet p(MsgType::CreateGrope);
        p.writeFieldRaw(p.field1, idlist.data(),idlist.size());
        p.finish();
        return p;
    }
    //服务器
    /* 方法：登陆反馈消息包 */
    static Packet makeLoginRe(bool s)//使用getfield1（）获得数据
    {
        Packet p(MsgType::Loginreturn);
        p.hdr.success=s;
        p.finish();
        return p;
    }
    /* 方法：注册反馈消息包 */
    static Packet makeRegiRe(bool s)//使用getfield1（）获得数据
    {
        Packet p(MsgType::regireturn);
        p.hdr.success=s;
        p.finish();
        return p;
    }
    //通用
    /* 方法：创建聊天消息包 */
    static Packet Message(     uint8_t Sendid, 
                               uint8_t Recvid,
                               const std::string& textbody,
                               const std::string& timestamp)//空时间戳为“”
    {
        Packet p(MsgType::NormalMsg);
        p.hdr.sendid = Sendid;
        p.hdr.recvid = Recvid;
        p.writeField1(textbody);//正文文本
        if (!timestamp.empty()) p.writeField2(timestamp);//时间戳
        p.finish();
        return p;
    }

    uint8_t getsendid()
    {
        return hdr.sendid;
    }

    uint8_t getrecvid()
    {
        return hdr.recvid;
    }
    bool success()
    {
        return hdr.success;
    }
    /* 获取序列化后的头部指针 */
    const char* data() const { return reinterpret_cast<const char*>(&hdr); }
    
    size_t size() const { 
        return sizeof(hdr) + field1.size() + field2.size() + field3.size() + field4.size(); 
    }

    /* 从socket阻塞接收一个完整数据包 */
    bool recvFrom(SOCKET s)
    {
        // 接收头部
        if(::recv(s, reinterpret_cast<char*>(&hdr), sizeof(hdr), MSG_WAITALL) != sizeof(hdr))
            return false;
            
        // 转换长度字段为主机序
        hdr.field1Len = n2h16(hdr.field1Len);
        hdr.field2Len = n2h16(hdr.field2Len);
        hdr.field3Len = n2h16(hdr.field3Len);
        hdr.field4Len = n2h16(hdr.field4Len);
        
        // 接收变长区1数据（如果有）
        if(hdr.field1Len){ 
            field1.resize(hdr.field1Len);
            if(::recv(s, reinterpret_cast<char*>(field1.data()), hdr.field1Len, MSG_WAITALL) != hdr.field1Len) 
                return false; 
        }
        
        // 接收变长区2数据（如果有）
        if(hdr.field2Len){ 
            field2.resize(hdr.field2Len);
            if(::recv(s, reinterpret_cast<char*>(field2.data()), hdr.field2Len, MSG_WAITALL) != hdr.field2Len) 
                return false; 
        }

        // 接收变长区3数据（如果有）
        if(hdr.field3Len){ 
            field3.resize(hdr.field3Len);
            if(::recv(s, reinterpret_cast<char*>(field3.data()), hdr.field3Len, MSG_WAITALL) != hdr.field3Len) 
                return false; 
        }

        // 接收变长区4数据（如果有）
        if(hdr.field4Len){ 
            field4.resize(hdr.field4Len);
            if(::recv(s, reinterpret_cast<char*>(field4.data()), hdr.field4Len, MSG_WAITALL) != hdr.field4Len) 
                return false; 
        }
        
        return true;
    }

    /* 发送数据包到socket */
    bool sendTo(SOCKET s) const
    {
        // 发送头部
        if(::send(s, data(), sizeof(hdr), 0) != sizeof(hdr))
            return false;
            
        // 发送变长区1（如果有）
        if(!field1.empty() && ::send(s, reinterpret_cast<const char*>(field1.data()), field1.size(), 0) != field1.size())
            return false;
            
        // 发送变长区2（如果有）
        if(!field2.empty() && ::send(s, reinterpret_cast<const char*>(field2.data()), field2.size(), 0) != field2.size())
            return false;

        // 发送变长区3（如果有）
        if(!field3.empty() && ::send(s, reinterpret_cast<const char*>(field3.data()), field3.size(), 0) != field3.size())
            return false;

        // 发送变长区4（如果有）
        if(!field4.empty() && ::send(s, reinterpret_cast<const char*>(field4.data()), field4.size(), 0) != field4.size())
            return false;
            
        return true;
    }

    /* 读取消息类型 */
    MsgType type() const { return static_cast<MsgType>(hdr.type); }
    
    /* 获取变长区1内容（字符串形式） */
    std::string getField1Str() const { 
        return std::string(reinterpret_cast<const char*>(field1.data()), field1.size()); 
    }
    
    /* 获取变长区2内容（字符串形式） */
    std::string getField2Str() const { 
        return std::string(reinterpret_cast<const char*>(field2.data()), field2.size()); 
    }

    /* 获取变长区3内容（字符串形式） */
    std::string getField3Str() const { 
        return std::string(reinterpret_cast<const char*>(field3.data()), field3.size()); 
    }

    /* 获取变长区4内容（字符串形式） */
    std::string getField4Str() const { 
        return std::string(reinterpret_cast<const char*>(field4.data()), field4.size()); 
    }

    /* 获取变长区1原始数据 */
    const std::vector<uint8_t>& getField1() const { return field1; }
    
    /* 获取变长区2原始数据 */
    const std::vector<uint8_t>& getField2() const { return field2; }

    /* 获取变长区3原始数据 */
    const std::vector<uint8_t>& getField3() const { return field3; }

    /* 获取变长区4原始数据 */
    const std::vector<uint8_t>& getField4() const { return field4; }

private:
    Header hdr{};                    // 数据包头部
    std::vector<uint8_t> field1;     // 变长区1
    std::vector<uint8_t> field2;     // 变长区2
    std::vector<uint8_t> field3;     // 变长区3
    std::vector<uint8_t> field4;     // 变长区4

    /* 完成打包：设置长度字段为网络序 */
    void finish(){ 
        hdr.field1Len = h2n16(static_cast<uint16_t>(field1.size()));
        hdr.field2Len = h2n16(static_cast<uint16_t>(field2.size()));
        hdr.field3Len = h2n16(static_cast<uint16_t>(field3.size()));
        hdr.field4Len = h2n16(static_cast<uint16_t>(field4.size()));
    }
    
    /* 向变长区写入原始数据 */
    void writeFieldRaw(std::vector<uint8_t>& field, const void* p, size_t n){ 
        auto* src = static_cast<const uint8_t*>(p);
        field.insert(field.end(), src, src + n); 
    }
    
    /* 向变长区写入字符串 */
    void writeField(std::vector<uint8_t>& field, const std::string& s){ 
        writeFieldRaw(field, s.data(), s.size());
    }
    
    /* 向变长区1写入字符串 */
    void writeField1(const std::string& s){ writeField(field1, s); }
    

    /* 向变长区2写入字符串 */
    void writeField2(const std::string& s){ writeField(field2, s); }

    /* 向变长区3写入字符串 */
    void writeField3(const std::string& s){ writeField(field3, s); }

    /* 向变长区4写入字符串 */
    void writeField4(const std::string& s){ writeField(field4, s); }
};