// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMap>       // <-- 新增：我们需要用到 QMap
#include <QList>      // <-- 新增：我们需要用到 QList
#include <QDateTime>  // <-- 新增：用于记录消息时间
#include <QMainWindow>
#include "addfrienddialog.h"    // 包含头文件
#include "creategroupdialog.h"  // 包含头文件

// 定义一条聊天消息的结构
struct ChatMessage {
    int senderId;       // 发送者的ID
    QString text;       // 消息内容
    QDateTime timestamp; // 发送时间
};
// ...
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sendButton_clicked();
    void on_conversationListWidget_itemClicked(QListWidgetItem *item);

    // 新增的槽函数声明
    void on_addFriendButton_clicked();
    void on_createGroupButton_clicked();

    void onAutoAcceptFriendRequest(uint8_t requesterId); // [新增]
    void onNewMessageReceived(const ChatMessage &message, int conversationId);

private:
    Ui::MainWindow *ui;
    // ...
    void updateConversationList(); // <-- 声明一个刷新界面的函数

    QMap<int, QString> m_friends; // <-- 用来存储好友 (ID -> 名字)
    QMap<int, QString> m_groups;  // <-- 用来存储群聊 (ID -> 名字)
    QMap<int, QList<ChatMessage>> m_chatHistories; // 存储所有对话的聊天记录
    int m_currentConversationId = -1; // -1代表没有选中任何对话
    void updateChatHistoryView(); // 专门负责刷新右侧聊天记录的函数
};
#endif // MAINWINDOW_H
