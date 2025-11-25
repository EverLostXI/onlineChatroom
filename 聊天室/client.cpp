#include "ChatMsg.hpp"
#include <iostream>
#include <thread>
#include <sstream>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <cctype>      // 引入 isdigit
// #pragma comment(lib, "ws2_32.lib")
static SOCKET g_sock = INVALID_SOCKET;
using namespace std;
/* 异步收包线程 */
void receiver()
{
    while(true)
    {
        Packet p;
        if(!p.recvFrom(g_sock)) { cout<<"[提示] 与服务端断开连接\n"; break; }
        switch(p.type())
        {
        case MsgType::NormalMsg:
        {
            string text   = p.readStr();
            string sender = p.readStr();
            cout<<"[消息] "<<sender<<": "<<text;
            if(p.isReply()) cout<<" (回复ID="<<p.replyMsgId()<<")";
            auto at = p.atList();
            if(!at.empty()){ cout<<" @"; for(auto id:at) cout<<id<<' '; }
            cout<<'\n';
            break;
        }
        case MsgType::PulledGroup:
        {
            uint32_t gid = p.readUint32();
            cout<<"[系统] 你被拉进了群 "<<gid<<'\n';
            break;
        }
        default:
            break;
        }
    }
}

/* 发一个完整包 */
void sendPkt(const Packet& p){ send(g_sock, p.data(), (int)p.size(), 0); }

int main()
{
    WSAData ws{};
    WSAStartup(MAKEWORD(2,2), &ws);

    g_sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{AF_INET, htons(8888)};
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if(connect(g_sock, (sockaddr*)&addr, sizeof(addr))==SOCKET_ERROR)
    { cerr<<"无法连接到服务器\n"; return 1; }

    cout<<"=== 简易聊天室 ===\n";
    cout<<"register <user> <pwd>   注册\n";
    cout<<"login    <user> <pwd>   登录\n";
    cout<<"send  <to> <text>       发普通消息\n";
    cout<<"reply <to> <id> <text>  带回复消息\n";
    cout<<"@用法: send <to> <text> @1001 @1002 …\n";
    cout<<"exit  退出\n\n";

    /* 启动收包线程 */
    thread rcv(receiver);

    string line;
    while(getline(cin, line))
    {
        if(line.empty()) continue;
        istringstream iss(line);
        string cmd; iss>>cmd;
        if(cmd=="register"){ string u,p; iss>>u>>p; sendPkt(Packet::makeCreateAcc(u,p)); }
        else if(cmd=="login"){ string u,p; iss>>u>>p; sendPkt(Packet::makeLogin(u,p)); }
        else if(cmd=="send")
        {
            string to,text; iss>>to; getline(iss,text);
            vector<uint32_t> at;
            size_t pos=0;
            while((pos=text.find('@',pos))!=string::npos)
            {
                pos++;
                uint32_t id=0;
                while(pos<text.size() && isdigit(text[pos])) id=id*10+(text[pos++]-'0');
                if(id) at.push_back(id);
            }
            sendPkt(Packet::makeChat(text, to, 0, at));
        }
        else if(cmd=="reply")
        {
            string to; uint32_t id; string text;
            iss>>to>>id; getline(iss,text);
            sendPkt(Packet::makeChat(text, to, id, {}));
        }
        else if(cmd=="exit") break;
        else cout<<"未知命令\n";
    }

    closesocket(g_sock);
    rcv.join();
    WSACleanup();
    return 0;
}