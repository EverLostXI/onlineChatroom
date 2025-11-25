// 客户端消息封装函数
/* 目录：
普通消息(common)
登录请求(login)
创建群聊(group)
*/

//定义消息类型
enum MsgType {
    COMMON = 1,
    LOGIN = 2,
    GROUP = 3
};

// 封装写入辅助函数(由于在每个封装函数中被多次调用，使用inline函数加速)
//写入uint32_t
inline void WriteUint32(vector<char>& buffer, size_t& offset, uint32_t value) { // 因为要使用memcpy，所以设置一个offset手动控制数据偏移量
    uint32_t netValue = htonl(value);
    memcpy(&buffer[offset], &netValue, sizeof(netValue)); // 用memcpy进行复制操作
    offset += sizeof(netValue);
}
//写入字符串
inline void WriteString(vector<char>& buffer, size_t& offset, const string& str) {
    WriteUint32(buffer, offset, static_cast<uint32_t>(str.size())); // 把数据长度转换为uint32_t的固定长度（4字节）
    memcpy(&buffer[offset], str.data(), str.size());
    offset += str.size();
}
// 使用重载函数直接在传入参数的时候判断消息类型
vector<char> MsgPackage(const string& content, uint32_t receiverID) { //普通消息，包含消息与接收人
    // 计算包体大小
    // 整条消息的结构为：消息类型（4字节）/后面所有东西的长度/接收者ID（4字节）/内容长度（4字节）/内容体
    uint32_t contentLength = static_cast<uint32_t>(content.size()); // 内容长度
    uint32_t bodyLength = 8 + contentLength;  // 消息体长度 = receiverID + contentLength + 内容体
    size_t totalSize = 8 + bodyLength;  // 消息类型 + 消息体长度
    // 初始化包体
    vector<char> package(totalSize);
    size_t offset = 0;
    // 写入数据
    WriteUint32(package, offset, static_cast<uint32_t>(MsgType::COMMON));
    WriteUint32(package, offset, bodyLength);
    WriteUint32(package, offset, receiverID);
    WriteString(package, offset, content);
    
    return package;
}

vector<char> MsgPackage(uint32_t accountID, const string& password) { // 登录请求，包含账户ID与密码
    // 计算包体大小
    // 整条消息的结构为：消息类型（4字节）/后面所有东西的长度/账户ID（4字节）/密码长度（4字节）/密码
    uint32_t passwordLength = static_cast<uint32_t>(password.size());
    uint32_t bodyLength = 8 + passwordLength; // accountID + passwordLength
    size_t totalSize = 8 + bodyLength; //与普通消息相同，不再赘述
    // 初始化
    vector<char> package(totalSize);
    size_t offset = 0;

    WriteUint32(package, offset, static_cast<uint32_t>(MsgType::LOGIN));
    WriteUint32(package, offset, bodyLength);
    WriteUint32(package, offset, accountID);
    WriteString(package, offset, password);
}

vector<char> MsgPackage(const vector<uint32_t>& memberIDs) { // 创建群聊，包含群聊成员ID（也包含自己）
    // 计算包体大小
    // 整条消息的结构为：消息类型（4字节）/消息体长度（4字节）/成员数量（4字节）/成员ID列表
    uint32_t memberCount = static_cast<uint32_t>(memberIDs.size());
    uint32_t bodyLength = sizeof(memberCount) + memberCount * sizeof(uint32_t);
    size_t totalSize = 8 + bodyLength;  // 消息类型(4) + 消息体长度(4) + 消息体
    
    // 初始化包体
    vector<char> package(totalSize);
    size_t offset = 0;
    
    // 写入数据
    WriteUint32(package, offset, static_cast<uint32_t>(MsgType::GROUP));
    WriteUint32(package, offset, bodyLength);
    WriteUint32(package, offset, memberCount);
    
    // 写入每个成员的ID
    for (uint32_t memberID : memberIDs) {
        WriteUint32(package, offset, memberID);
    }
    
    return package;
}
// 客户端消息解包函数
/* 目录：
普通消息(common)
登录请求结果(login)
被拉入群聊(group)
*/



// 服务端消息解包函数
/* 目录：
普通消息（包含转发）(common)
登录请求（包含发送结果）(login)
创建群聊（包含转发拉入群聊）(group)
*/
void UnpackProccess() {

}
