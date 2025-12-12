// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMap>       // <-- 新增：我们需要用到 QMap
#include <QList>      // <-- 新增：我们需要用到 QList
#include <QDateTime>  // <-- 新增：用于记录消息时间
#include <QVector>
#include <QMainWindow>
#include "addfrienddialog.h"    // 包含头文件
#include "creategroupdialog.h"  // 包含头文件

#include <QMap>        // [新增]
#include <QByteArray>  // [新增]
#include <QFileDialog> // [新增]
#include <QFileInfo>   // [新增]

// [新增] 定义一个结构体来存储群聊的完整信息
struct GroupInfo {
    QString groupName;
    QVector<uint8_t> memberIds;
};
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
    void on_setNicknameButton_clicked();
    void onNicknameChanged(const QString& newNickname);

    void onAutoAcceptFriendRequest(uint8_t requesterId); // [新增]
    void onNewMessageReceived(const ChatMessage &message, const QString& conversationId);
    // [新增] 响应 NetworkManager 信号的槽函数
    void onCreateGroupResult(bool success, const QString& message);
    void onAddedToNewGroup(const QString& groupName, uint8_t creatorId, const QVector<uint8_t>& memberIds);
    void on_selectImageButton_clicked(); // [新增]
    // [新增] 响应 NetworkManager 图片信号的槽函数
    void onNewImageReceived(uint8_t senderId,
                            const QString& conversationId,
                            const QByteArray& imageData,
                            const QString& fileName);

    void onSetNicknameResult(bool success);
    void onCheckUserStatusResult(uint8_t userId, const QString& nickname, bool isOnline);

private:
    Ui::MainWindow *ui;
    // ...
    void updateConversationList(); // <-- 声明一个刷新界面的函数

    QMap<int, QString> m_friends; // <-- 用来存储好友 (ID -> 名字)
    // [修改] 群聊列表，Key从 int 改为 QString (群名)
    QMap<QString, GroupInfo> m_groups;

    // [修改] 聊天记录，Key从 int 改为 QString
    QMap<QString, QList<ChatMessage>> m_chatHistories;

    // 新增数据结构 - 需要声明
    QHash<QString, int> m_unreadCounts;  // 未读消息计数
    QHash<QString, QListWidgetItem*> m_conversationItems; // 对话项指针映射

    // [修改] 当前会话ID，从 int 改为 QString
    QString m_currentConversationId = "-1";

    // [新增] 用于暂存正在创建的群聊的名称
    QString m_pendingGroupName;
    // [新增] 用于暂存正在创建的群聊的成员列表
    QVector<uint8_t> m_pendingGroupMembers;
    // [新增] 用于图片发送功能
    QString m_selectedImagePath;

    // [新增] 用于存储聊天记录中的图片数据
    // Key:   消息的时间戳 (作为唯一标识)
    // Value: 图片的二进制数据
    QMap<QDateTime, QByteArray> m_imageHistory;
    void updateChatHistoryView(); // 专门负责刷新右侧聊天记录的函数

    void updateConversationItem(const QString& conversationId);
    QString formatConversationDisplay(const QString& conversationId,
                                      const QString& name,
                                      bool isGroup);
    void setItemUnreadStyle(QListWidgetItem* item, int unreadCount);

    void updateWelcomeMessage();
};
#endif // MAINWINDOW_H
