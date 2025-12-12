#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include <QtEndian> // <-- 正确，这个头文件专门用于字节序转换
#include <QTcpSocket> // <-- 添加，用于网络操作



// 使用Qt的函数
inline uint16_t h2n16(uint16_t v){ return qToBigEndian(v); }
inline uint32_t h2n32(uint32_t v){ return qToBigEndian(v); }
inline uint16_t n2h16(uint16_t v){ return qFromBigEndian(v); }
inline uint32_t n2h32(uint32_t v){ return qFromBigEndian(v); }

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
    NormalMsg    = 0x10,  // 普通消息
    GroupMsg     = 0x11,  // 群聊消息
    ImageMsg     = 0x12,  // 图片消息
    SetName      = 0x13,  //[新增]设置用户名
    CheckUser    = 0x14   //[新增]查询用户状态
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
    const Header& internal_get_header_for_debug() const { return hdr; }

    // === 新增的公共解析函数 ===
    // 从给定的数据指针和大小中解析出一个完整的数据包
    // 如果数据足够解析一个完整的包，则返回 true，否则返回 false
    bool parseFrom(const char* data, size_t size)
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
            // 数据不完整，重置头部长度为主机序（因为n2h16已经转换过）
            // 这一步其实不是必须的，但保持对象状态一致性是好习惯
            hdr.field1Len = h2n16(hdr.field1Len);
            // ... 其他len也一样
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



    // 修改后的默认构造函数：确保hdr被清零
    Packet() {
        memset(&hdr, 0, sizeof(Header));
    }
    // 修改后的带类型的构造函数：先清零，再赋值
    explicit Packet(MsgType t) {
        memset(&hdr, 0, sizeof(Header));
        hdr.type = static_cast<uint8_t>(t);
    }
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
    /* 方法：创建群聊 (修改版) */
    // 参数：idlist - 成员ID列表, groupName - 群聊名称
    static Packet makeCreGro(uint8_t creatorId, const std::vector<uint8_t>& idlist, const std::string& groupName)
    {
        Packet p(MsgType::CreateGrope);

        // [修改] 将创建者ID放入 sendid 字段
        p.hdr.sendid = creatorId;

        // Field 1: 存放其他成员ID列表 (二进制数据)
        // 注意：这里的 const_cast 是安全的，因为 writeFieldRaw 不会修改数据
        p.writeFieldRaw(p.field1, idlist.data(), idlist.size());

        // Field 2: 存放群聊名称 (字符串)
        p.writeField2(groupName);

        p.finish();
        return p;
    }

    /* 方法：添加好友请求 (新增) */
    static Packet makeAddFriend(uint8_t sendId, uint8_t targetId)
    {
        Packet p(MsgType::AddFriendReq);
        p.hdr.sendid = sendId;   // 发起请求的人
        p.hdr.recvid = targetId; // 被请求的人
        // 如果需要附带验证消息，可以用 writeField1("我是xxx");
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

    /* 方法：创建群聊反馈 (新增) */
    static Packet makeCreGroRe(bool s)
    {
        Packet p(MsgType::CreateGroRe);
        p.hdr.success = s;
        p.finish();
        return p;
    }

    /* 方法：添加好友反馈 (新增) */
    static Packet makeAddFriendRe(uint8_t sendId, uint8_t targetId, bool s)
    {
        Packet p(MsgType::AddFriendRe);
        p.hdr.sendid = targetId;   // 这里sendId是最初发起请求的人
        p.hdr.recvid = sendId; // 这里recvid通常是做出响应的人（被添加者）
        p.hdr.success = s;
        p.finish();
        return p;
    }

    //通用
    /* 方法：心跳包（极简空包，仅type字段有效，其余全为0） */
    static Packet makeHeartbeat()
    {
        Packet p(MsgType::Heartbeat);
        p.finish();
        return p;
    }
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
    /* [新增] 方法：创建群聊消息包 */
    static Packet makeGroupMessage(uint8_t senderId, const std::string& groupId, const std::string& textbody, const std::string& timestamp)
    {
        Packet p(MsgType::GroupMsg);
        p.hdr.sendid = senderId;
        // recvid 在群聊消息中无意义，保持为0

        p.writeField1(textbody);      // Field 1: 消息正文
        p.writeField2(groupId);       // Field 2: 目标群ID (字符串)
        if (!timestamp.empty()) {
            p.writeField3(timestamp); // Field 3: 时间戳 (如果需要)
        }
        p.finish();
        return p;
    }

    /* [新增] 方法：创建图片消息包 (支持私聊和群聊) */
    static Packet makeImageMessage(uint8_t senderId,
                                   const std::string& targetId,
                                   bool isGroup,
                                   const std::vector<uint8_t>& imageData,
                                   const std::string& imageName)
    {
        Packet p(MsgType::ImageMsg);
        p.hdr.sendid = senderId;
        p.hdr.success = isGroup;

        // 核心数据：图片二进制内容
        p.writeFieldRaw(p.field1, imageData.data(), imageData.size());

        // 元数据：图片文件名
        p.writeField2(imageName);

        // =============================================================
        // ================ 关键修改：在这里先调用 finish() ================
        // =============================================================
        // finish() 会根据 field1 和 field2 当前的 size() 来设置 hdr 里的长度
        // 此时 field1.size() 是 8696, field2.size() 是 9
        // 所以 hdr.field1Len 会被正确设置为 8696
        p.finish();
        // =============================================================


        if (isGroup) {
            // 如果是群聊，recvid无意义，我们将群ID存放在field3
            p.writeField3(targetId);
            // 注意：因为我们提前调用了 finish()，所以这里对 field3 的修改
            // 不会更新到 hdr.field3Len。我们需要手动更新。
            p.hdr.field3Len = static_cast<uint16_t>(p.field3.size());
        } else {
            // 如果是私聊，将字符串形式的接收者ID转为uint8_t
            try {
                p.hdr.recvid = static_cast<uint8_t>(std::stoul(targetId));
            } catch (const std::exception& e) {
                p.hdr.recvid = 0;
            }
        }

        // 因为 finish() 已经被调用过了，所以函数末尾不需要再调用
        return p;
    }

    static Packet SetUserName(uint8_t senderId, const std::string& username) {
        Packet p(MsgType::SetName);
        p.hdr.sendid = senderId;
        p.writeField1(username);
        p.finish();
        return p;
    }

    static Packet CheckUserStatus(uint8_t senderId, uint8_t targetId) {
        Packet p(MsgType::CheckUser);
        p.hdr.sendid = senderId;
        p.hdr.recvid = targetId;  // 注意：这里设置 recvid 为目标用户
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



    /* 发送数据包到socket */
    bool sendTo(QTcpSocket* socket) const
    {
        if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
            return false;
        }

        // 1. 准备一个临时的、网络字节序的头部
        Header networkHeader = hdr;
        networkHeader.field1Len = h2n16(hdr.field1Len);
        networkHeader.field2Len = h2n16(hdr.field2Len);
        networkHeader.field3Len = h2n16(hdr.field3Len);
        networkHeader.field4Len = h2n16(hdr.field4Len);

        // 2. 发送数据
        socket->write(reinterpret_cast<const char*>(&networkHeader), sizeof(Header));
        if (!field1.empty()) socket->write(reinterpret_cast<const char*>(field1.data()), field1.size());
        if (!field2.empty()) socket->write(reinterpret_cast<const char*>(field2.data()), field2.size());
        if (!field3.empty()) socket->write(reinterpret_cast<const char*>(field3.data()), field3.size());
        if (!field4.empty()) socket->write(reinterpret_cast<const char*>(field4.data()), field4.size());

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
        // 直接赋值长度
        hdr.field1Len = static_cast<uint16_t>(field1.size());
        hdr.field2Len = static_cast<uint16_t>(field2.size());
        hdr.field3Len = static_cast<uint16_t>(field3.size());
        hdr.field4Len = static_cast<uint16_t>(field4.size());
    }
    
    /* 向变长区写入原始数据 */
    void writeFieldRaw(std::vector<uint8_t>& field, const void* p, size_t n){ 
        auto* src = static_cast<const uint8_t*>(p);
        field.assign(src, src + n);
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

