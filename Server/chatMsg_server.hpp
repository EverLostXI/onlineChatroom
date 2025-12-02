// 这个版本的chatmsg与客户端的工作逻辑相同，但是负责网络操作的部分不依赖qt
// 由于链接Winsock
#pragma once // 由于是头文件所以需要防止重复包含
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <winsock2.h> // 与客户端版本的chatmsg的区别（客户端用的是qt的库）

// 字节序转换函数（这里用winsock2实现）
inline uint16_t h2n16(uint16_t v) { return htons(v); }
inline uint32_t h2n32(uint32_t v) { return htonl(v); }
inline uint16_t n2h16(uint16_t v) { return ntohs(v); }
inline uint32_t n2h32(uint32_t v) { return ntohl(v); }

// 消息类型枚举
enum class MsgType : uint8_t
{
    LoginReq     = 0x01,  // 登录请求
    CreateAcc    = 0x02,  // 创建账号
    CreateGrope  = 0x03,  // 创建群聊请求
    Loginreturn  = 0x04,  // 登录反馈
    regireturn   = 0x05,  // 注册反馈
    CreateGroRe  = 0x06,  // 创建群聊反馈
    AddFriendReq = 0x07,  // 添加好友请求
    AddFriendRe  = 0x08,  // 添加好友反馈
    Heartbeat    = 0x09,  // 心跳包
    NormalMsg    = 0x10   // 普通消息
};

#pragma pack(push,1)


// 数据包头部结构
struct Header
{
    uint8_t  type;       // 消息类型
    bool  success;       // 成功/失败
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
    // 解包函数
    bool parseFrom(const char* data, size_t size) // 参数列表：接收的数据指针，数据大小
    {
        // 1. 检查数据是否足够一个包头
        if (size < sizeof(Header)) {
            return false;
        }

        // 2. 拷贝并解析包头
        memcpy(&hdr, data, sizeof(Header));
        hdr.field1Len = n2h16(hdr.field1Len);
        hdr.field2Len = n2h16(hdr.field2Len);
        hdr.field3Len = n2h16(hdr.field3Len);
        hdr.field4Len = n2h16(hdr.field4Len);

        // 3. 计算包体总大小
        size_t bodySize = hdr.field1Len + hdr.field2Len + hdr.field3Len + hdr.field4Len;
        size_t totalPacketSize = sizeof(Header) + bodySize;

        // 4. 检查数据是否足够一个完整的包
        if (size < totalPacketSize) {
            return false;
        }

        // 5. 数据完整，拷贝包体
        size_t offset = sizeof(Header);
        if (hdr.field1Len > 0) {
            field1.assign(data + offset, data + offset + hdr.field1Len);
            offset += hdr.field1Len;
        }
        if (hdr.field2Len > 0) {
            field2.assign(data + offset, data + offset + hdr.field2Len);
            offset += hdr.field2Len;
        }
        if (hdr.field3Len > 0) {
            field3.assign(data + offset, data + offset + hdr.field3Len);
            offset += hdr.field3Len;
        }
        if (hdr.field4Len > 0) {
            field4.assign(data + offset, data + offset + hdr.field4Len);
        }

        return true;
    }

    // 默认构造函数
    Packet() {
        memset(&hdr, 0, sizeof(Header));
    }
    
    // 带类型的构造函数
    explicit Packet(MsgType t) {
        memset(&hdr, 0, sizeof(Header));
        hdr.type = static_cast<uint8_t>(t);
    }

    // === 服务器端构造响应包的方法 ===
    
    /* 方法：登陆反馈消息包 */
    static Packet makeLoginRe(bool s)
    {
        Packet p(MsgType::Loginreturn);
        p.hdr.success = s;
        p.finish();
        return p;
    }
    
    /* 方法：注册反馈消息包 */
    static Packet makeRegiRe(bool s)
    {
        Packet p(MsgType::regireturn);
        p.hdr.success = s;
        p.finish();
        return p;
    }

    /* 方法：创建群聊反馈 */
    static Packet makeCreGroRe(bool s)
    {
        Packet p(MsgType::CreateGroRe);
        p.hdr.success = s;
        p.finish();
        return p;
    }

    /* 方法：添加好友反馈 */
    static Packet makeAddFriendRe(uint8_t sendId, uint8_t targetId, bool s)
    {
        Packet p(MsgType::AddFriendRe);
        p.hdr.sendid = sendId;
        p.hdr.recvid = targetId;
        p.hdr.success = s;
        p.finish();
        return p;
    }

    /* 方法：心跳包（极简空包，仅type字段有效，其余全为0） */
    static Packet makeHeartbeat()
    {
        Packet p(MsgType::Heartbeat);
        p.finish();
        return p;
    }

    /* 方法：创建聊天消息包（用于转发） */
    static Packet Message(uint8_t Sendid, 
                          uint8_t Recvid,
                          const std::string& textbody,
                          const std::string& timestamp = "")
    {
        Packet p(MsgType::NormalMsg);
        p.hdr.sendid = Sendid;
        p.hdr.recvid = Recvid;
        p.writeField1(textbody);
        if (!timestamp.empty()) p.writeField2(timestamp);
        p.finish();
        return p;
    }

    // === 访问器方法 ===
    
    uint8_t getsendid() const { return hdr.sendid; }
    uint8_t getrecvid() const { return hdr.recvid; }
    bool success() const { return hdr.success; }
    MsgType type() const { return static_cast<MsgType>(hdr.type); }
    
    /* 获取序列化后的头部指针 */
    const char* data() const { return reinterpret_cast<const char*>(&hdr); }
    
    size_t size() const { 
        return sizeof(hdr) + field1.size() + field2.size() + field3.size() + field4.size(); 
    }
    
    /* 获取变长区内容（字符串形式） */
    std::string getField1Str() const { 
        return std::string(reinterpret_cast<const char*>(field1.data()), field1.size()); 
    }
    
    std::string getField2Str() const { 
        return std::string(reinterpret_cast<const char*>(field2.data()), field2.size()); 
    }

    std::string getField3Str() const { 
        return std::string(reinterpret_cast<const char*>(field3.data()), field3.size()); 
    }

    std::string getField4Str() const { 
        return std::string(reinterpret_cast<const char*>(field4.data()), field4.size()); 
    }

    /* 获取变长区原始数据 */
    const std::vector<uint8_t>& getField1() const { return field1; }
    const std::vector<uint8_t>& getField2() const { return field2; }
    const std::vector<uint8_t>& getField3() const { return field3; }
    const std::vector<uint8_t>& getField4() const { return field4; }

private:
    Header hdr{};                    // 数据包头部
    std::vector<uint8_t> field1;     // 变长区1
    std::vector<uint8_t> field2;     // 变长区2
    std::vector<uint8_t> field3;     // 变长区3
    std::vector<uint8_t> field4;     // 变长区4

    /* 完成打包：设置长度字段 */
    void finish(){
        hdr.field1Len = static_cast<uint16_t>(field1.size());
        hdr.field2Len = static_cast<uint16_t>(field2.size());
        hdr.field3Len = static_cast<uint16_t>(field3.size());
        hdr.field4Len = static_cast<uint16_t>(field4.size());
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
    
    /* 向各变长区写入字符串 */
    void writeField1(const std::string& s){ writeField(field1, s); }
    void writeField2(const std::string& s){ writeField(field2, s); }
    void writeField3(const std::string& s){ writeField(field3, s); }
    void writeField4(const std::string& s){ writeField(field4, s); }
};
