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
    CreateGroup = 0x03,  // 创建群组
    PulledGroup = 0x04,   // 服务器下行推送群组
    Beat = 0xFF,//心跳
    NormalMsg   = 0x10   // 普通消息
};

#pragma pack(push,1)
// 数据包头部结构
struct Header
{
    uint8_t  type;      // 消息类型
    uint8_t  flags;     // 标志位：bit0-是否回复消息 bit1-是否有@列表
    uint16_t extLen;    // 扩展区长度
    uint32_t bodyLen;   // 业务体长度（网络序）
};
#pragma pack(pop)

// 网络数据包类
class Packet
{
public:
    Packet() = default;
    explicit Packet(MsgType t){ hdr.type = static_cast<uint8_t>(t); }

    /* 方法：创建登录请求包 */
    static Packet makeLogin(const std::string& user, const std::string& pwd)
    {
        Packet p(MsgType::LoginReq);
        p.writeStr(user); p.writeStr(pwd); p.finish(); return p;
    }
    
    /* 方法：创建注册请求包 */
    static Packet makeCreateAcc(const std::string& user, const std::string& pwd)
    {
        Packet p(MsgType::CreateAcc);
        p.writeStr(user); p.writeStr(pwd); p.finish(); return p;
    }
    
    /* 方法：创建聊天消息包（支持回复、@功能） */
    static Packet makeChat(const std::string& text,
                           const std::string& receiver,
                           uint32_t replyId = 0,
                           const std::vector<uint32_t>& at = {})
    {
        Packet p(MsgType::NormalMsg);
        if(replyId) p.setReply(replyId);        // 设置回复消息ID
        if(!at.empty()) p.setAtList(at);        // 设置@列表
        p.writeStr(text);                       // 写入消息内容
        p.writeStr(receiver);                   // 写入接收者
        p.finish();                             // 完成打包
        return p;
    }
    static Packet Heartbeat()
    {
        Packet p(MsgType::NormalMsg);
        p.finish();
        return p;
    }
    /* 获取序列化后的头部指针 */
    const char* data() const { return reinterpret_cast<const char*>(&hdr); }
    
    /* 获取序列化后的数据总长度 */
    size_t      size() const { return sizeof(hdr) + hdr.extLen + hdr.bodyLen; }

    /* 从socket阻塞接收一个完整数据包 */
    bool recvFrom(SOCKET s)
    {
        // 接收头部
        if(::recv(s, reinterpret_cast<char*>(&hdr), sizeof(hdr), MSG_WAITALL)!=sizeof(hdr))
            return false;
            
        // 转换长度字段为主机序
        hdr.bodyLen = n2h32(hdr.bodyLen);
        hdr.extLen  = n2h16(hdr.extLen);
        
        // 接收扩展区数据（如果有）
        if(hdr.extLen){ 
            ext.resize(hdr.extLen);
            if(::recv(s, reinterpret_cast<char*>(ext.data()), hdr.extLen, MSG_WAITALL)!=hdr.extLen) 
                return false; 
        }
        
        // 接收业务体数据（如果有）
        if(hdr.bodyLen){ 
            body.resize(hdr.bodyLen);
            if(::recv(s, reinterpret_cast<char*>(body.data()), hdr.bodyLen, MSG_WAITALL)!=hdr.bodyLen) 
                return false; 
        }
        return true;
    }

    /* 读取消息类型 */
    MsgType type() const { return static_cast<MsgType>(hdr.type); }
    
    /* 检查是否为回复消息 */
    bool isReply() const { return hdr.flags & 1; }
    
    /* 获取被回复的消息ID */
    uint32_t replyMsgId() const
    {
        if(!isReply() || ext.size()<4) return 0;
        return n2h32(*reinterpret_cast<const uint32_t*>(ext.data()));
    }
    
    /* 获取@列表 */
    std::vector<uint32_t> atList() const
    {
        std::vector<uint32_t> v;
        if(!(hdr.flags&2)) return v;  // 无@列表
        
        const uint8_t* p = ext.data();
        if(isReply()) p += 4;  // 跳过回复消息ID
        
        // 读取@用户数量
        uint16_t cnt = n2h16(*reinterpret_cast<const uint16_t*>(p)); p+=2;
        
        // 读取每个@用户的ID
        for(uint16_t i=0;i<cnt;++i){ 
            v.push_back(n2h32(*reinterpret_cast<const uint32_t*>(p))); 
            p+=4; 
        }
        return v;
    }
    
    /* 从包体中读取字符串 */
    std::string readStr()
    {
        uint16_t len = readCnt<uint16_t>();  // 先读长度
        std::string s(reinterpret_cast<const char*>(body.data() + rdPos), len); 
        rdPos+=len; 
        return s;
    }
    
    /* 从包体中读取32位无符号整数 */
    uint32_t readUint32(){ return readCnt<uint32_t>(); }

private:
    Header hdr{};                   // 数据包头部
    std::vector<uint8_t> ext;       // 扩展数据区
    std::vector<uint8_t> body;      // 业务数据区
    size_t wrPos = 0, rdPos = 0;    // 读写位置指针

    /* 完成打包：设置长度字段为网络序 */
    void finish(){ 
        hdr.bodyLen=h2n32(static_cast<uint32_t>(body.size()));
        hdr.extLen =h2n16(static_cast<uint16_t>(ext.size())); 
    }
    
    /* 向包体写入原始数据 */
    void writeRaw(const void* p, size_t n){ 
        auto* src=static_cast<const uint8_t*>(p);
        body.insert(body.end(), src, src+n); 
    }
    
    /* 向包体写入数值（自动转换为网络序） */
    template<typename T>
    void writeVal(T v){ 
        if constexpr(sizeof(T)==2) v=h2n16(v);
        if constexpr(sizeof(T)==4) v=h2n32(v);
        writeRaw(&v,sizeof(v)); 
    }
    
    /* 向包体写入字符串（长度+内容） */
    void writeStr(const std::string& s){ 
        writeVal<uint16_t>(static_cast<uint16_t>(s.size()));  // 先写长度
        writeRaw(s.data(),s.size());                          // 再写内容
    }
    
    /* 设置回复消息ID */
    void setReply(uint32_t id){ 
        hdr.flags|=1; 
        uint32_t net=h2n32(id);
        ext.insert(ext.end(), reinterpret_cast<uint8_t*>(&net),
                   reinterpret_cast<uint8_t*>(&net)+4); 
    }
    
    /* 设置@用户列表 */
    void setAtList(const std::vector<uint32_t>& v){ 
        hdr.flags|=2;
        // 写入@用户数量
        uint16_t cnt=h2n16(static_cast<uint16_t>(v.size()));
        ext.insert(ext.end(), reinterpret_cast<uint8_t*>(&cnt),
                   reinterpret_cast<uint8_t*>(&cnt)+2);
        // 写入每个@用户的ID
        for(uint32_t id:v){ 
            uint32_t net=h2n32(id);
            ext.insert(ext.end(), reinterpret_cast<uint8_t*>(&net),
                       reinterpret_cast<uint8_t*>(&net)+4); 
        } 
    }
    
    /* 从包体读取数值（自动转换为主机序） */
    template<typename T>
    T readCnt(){ 
        T v{}; 
        std::memcpy(&v, body.data()+rdPos, sizeof(T)); 
        rdPos+=sizeof(T);
        if constexpr(sizeof(T)==2) v=n2h16(v);
        if constexpr(sizeof(T)==4) v=n2h32(v); 
        return v; 
    }
};