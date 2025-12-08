// MainWindow.cpp

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "CreateGroupDialog.h" // 确保包含了CreateGroupDialog
#include "AddFriendDialog.h"
#include <QListWidgetItem> // 如果槽函数参数用到了，需要包含头文件
#include <QMessageBox>
#include <QFont>
#include <QApplication>
#include <QSettings>
#include "networkmanager.h"
#include "SetNickname.h"

// ... 其他代码 ...

// 构造函数可能已经有了
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("开始聊天");

    // 初始化欢迎信息
    updateWelcomeMessage();

    // ...
    connect(&NetworkManager::instance(), &NetworkManager::autoAcceptFriendRequest, this, &MainWindow::onAutoAcceptFriendRequest);
    connect(&NetworkManager::instance(), &NetworkManager::newMessageReceived, this, &MainWindow::onNewMessageReceived);

    // --- 添加初始的假数据 ---////////////////////////
    /////////////////////////////////////////////////
    m_friends[123] = "张三";
    m_friends[124] = "李四";
    GroupInfo friendGroup;
    friendGroup.groupName = "朋友群";
    friendGroup.memberIds.append(123); // 把“张三”加进去
    friendGroup.memberIds.append(124); // 把“李四”加进去
    friendGroup.memberIds.append(1);   // 把“我”自己也加进去
    m_groups.insert("朋友群", friendGroup); // 把这个完整的群信息对象存入 m_groups
    int myUserId = NetworkManager::instance().selfId();

    // --- 1. 创建和张三(1001)的聊天记录 ---
    QList<ChatMessage> zhangsanHistory;

    ChatMessage msg1;
    msg1.senderId = 123; // 张三发的
    msg1.text = "你好，在忙吗？";
    msg1.timestamp = QDateTime::currentDateTime().addSecs(-120); // 假设是120秒前
    zhangsanHistory.append(msg1);

    ChatMessage msg2;
    msg2.senderId = myUserId; // 我发的
    msg2.text = "你好啊，刚忙完。";
    msg2.timestamp = QDateTime::currentDateTime().addSecs(-60); // 假设是60秒前
    zhangsanHistory.append(msg2);

    // 把和张三的完整聊天记录列表，存入“大柜子”m_chatHistories
    m_chatHistories.insert("123", zhangsanHistory);


    // --- 2. 创建和李四(124)的聊天记录 ---
    QList<ChatMessage> lisiHistory;

    ChatMessage msg3;
    msg3.senderId = 124; // 李四发的
    msg3.text = "那个项目文档你发我一下。";
    msg3.timestamp = QDateTime::currentDateTime(); // 假设是刚刚
    lisiHistory.append(msg3);

    // 把和李四的完整聊天记录列表，存入“大柜子”m_chatHistories
    m_chatHistories.insert("124", lisiHistory);

    // --- 3. 创建朋友群(125)的聊天记录 ---
    QList<ChatMessage> groupHistory;

    ChatMessage msg4;
    msg4.senderId = 123; // 张三在群里发的
    msg4.text = "你在哪？";
    msg4.timestamp = QDateTime::currentDateTime().addSecs(-180);
    groupHistory.append(msg4);

    ChatMessage msg5;
    msg5.senderId = myUserId; // 我在群里发的
    msg5.text = "我在学校。";
    msg5.timestamp = QDateTime::currentDateTime().addSecs(-10);
    groupHistory.append(msg5);

    // 把群的聊天记录存入“大柜子”
    m_chatHistories.insert("朋友群", groupHistory);

    // =======================================================
    // === 测试数据添加完毕 ====================================
    // =======================================================
    // 启动时刷新一次列表
    // [新增] 连接创建群聊相关的信号
    connect(&NetworkManager::instance(), &NetworkManager::createGroupResult, this, &MainWindow::onCreateGroupResult);
    connect(&NetworkManager::instance(), &NetworkManager::addedToNewGroup, this, &MainWindow::onAddedToNewGroup);
    updateConversationList();
}

// ！！！确保析构函数的实现存在 ！！！
MainWindow::~MainWindow()
{
    delete ui;
}

// ！！！为所有槽函数添加实现 ！！！

void MainWindow::on_setNicknameButton_clicked(){
    qDebug() <<"click set nickname";
    // 使用指针创建对话框，以便连接信号
    SetNickname *setNickname = new SetNickname(this);
    setNickname->setAttribute(Qt::WA_DeleteOnClose);

    // 连接昵称更改信号
    connect(setNickname, &SetNickname::nicknameChanged,
            this, &MainWindow::onNicknameChanged);

    // 连接对话框关闭信号
    connect(setNickname, &SetNickname::finished,
            this, [this, setNickname]() {
                // 对话框关闭后更新欢迎信息
                updateWelcomeMessage();
            });

    setNickname->show();
}

// 更新欢迎信息函数
void MainWindow::updateWelcomeMessage()
{
    // 从设置文件读取昵称
    QSettings settings("CSC3002", "Chatroom");
    QString nickname = settings.value("Client/Nickname", "用户0").toString();

    // 假设您有一个 welcomeLabel 标签
    if (ui->welcomeLabel) {
        ui->welcomeLabel->setText(QString("欢迎！%1").arg(nickname));
    }

    qDebug() << "更新欢迎信息，昵称:" << nickname;
}

void MainWindow::onNicknameChanged(const QString& newNickname)
{
    qDebug() << "昵称已更改为:" << newNickname;

    // 立即更新欢迎信息
    if (ui->welcomeLabel) {
        ui->welcomeLabel->setText(QString("欢迎！%1").arg(newNickname));
    }

    // 这里还可以添加其他逻辑，比如更新聊天中显示的"我"的名字
    // 或者通知服务器昵称更改等
}

// 这个函数在用户点击“发送”按钮时被自动调用
// [修改] on_sendButton_clicked
void MainWindow::on_sendButton_clicked()
{
    if (m_currentConversationId == "-1") { return; }
    QString text = ui->messageInputTextEdit->toPlainText().trimmed();
    if (text.isEmpty()) { return; }

    uint8_t myUserId = NetworkManager::instance().selfId();

    // 本地更新UI
    ChatMessage newMessage = { (int)myUserId, text, QDateTime::currentDateTime() };
    m_chatHistories[m_currentConversationId].append(newMessage);
    updateChatHistoryView();
    ui->messageInputTextEdit->clear();

    // 网络发送逻辑
    bool isGroup = m_groups.contains(m_currentConversationId);
    if (isGroup) {
        // [修改] 调用新的群聊发送函数
        qDebug() << "发送群聊消息到:" << m_currentConversationId;
        NetworkManager::instance().sendGroupMessage(myUserId, m_currentConversationId, text);
    } else {
        // 私聊逻辑保持不变
        qDebug() << "发送私聊消息到:" << m_currentConversationId;
        NetworkManager::instance().sendMessage(myUserId, m_currentConversationId.toUInt(), text);
    }
}

// 这个函数在用户点击左侧列表时被自动调用
// [修改] on_conversationListWidget_itemClicked
void MainWindow::on_conversationListWidget_itemClicked(QListWidgetItem *item)
{
    QString conversationId = item->data(Qt::UserRole).toString();

    // 清除该对话的未读计数
    if (m_unreadCounts.contains(conversationId) && m_unreadCounts[conversationId] > 0) {
        m_unreadCounts.remove(conversationId);
        updateConversationItem(conversationId);
    }

    // 切换到对话
    m_currentConversationId = conversationId;
    qDebug() << "切换到对话:" << m_currentConversationId;
    updateChatHistoryView();
}

// 这个函数专门根据 m_currentConversationId 来刷新右侧的聊天窗口
void MainWindow::updateChatHistoryView()
{
    ui->chatHistoryListWidget->clear();
    ui->chatHistoryListWidget->setSpacing(5);

    if (m_currentConversationId == "-1" || !m_chatHistories.contains(m_currentConversationId)) {
        return;
    }

    uint8_t myUserId = NetworkManager::instance().selfId();
    const QList<ChatMessage>& messages = m_chatHistories[m_currentConversationId];

    for (const ChatMessage& msg : messages)
    {
        QListWidgetItem* item = new QListWidgetItem();
        QString senderName;

        QSettings settings("CSC3002", "Chatroom");
        QString myNickname = settings.value("Client/Nickname", "用户0").toString();

        if (msg.senderId == myUserId) {
            senderName = QString("%1(%2)").arg(myNickname).arg(myUserId);
            item->setTextAlignment(Qt::AlignRight);
        } else {
            // 好友的消息：显示为"好友昵称(好友ID)"格式
            // 获取好友的昵称
            QString friendNickname = m_friends.value(msg.senderId, QString("用户%1").arg(msg.senderId));
            // 格式化为"昵称(ID)"的格式
            senderName = QString("%1(%2)").arg(friendNickname).arg(msg.senderId);
            item->setTextAlignment(Qt::AlignLeft);
        }

        item->setText(senderName + ": " + msg.text);
        ui->chatHistoryListWidget->addItem(item);
    }
    ui->chatHistoryListWidget->scrollToBottom();
}


void MainWindow::on_addFriendButton_clicked() // 假设你的按钮槽函数是这个名字
{
    AddFriendDialog dialog(this);

    // dialog.exec() 会显示对话框并等待它关闭
    // 如果我们在对话框内部调用了 accept(), exec() 会返回 QDialog::Accepted
    if (dialog.exec() == QDialog::Accepted) {
        // 1. 如果添加成功，就从对话框获取新好友的ID
        int newFriendId = dialog.getAddedFriendId();

        // 2. 检查ID是否有效，并更新本地好友列表
        if (newFriendId != -1 && !m_friends.contains(newFriendId)) {
            // 为了显示，我们先给一个默认名字
            QString newFriendName = QString("好友 %1").arg(newFriendId);
            m_friends[newFriendId] = newFriendName;

            // 3. 刷新界面上的好友列表
            updateConversationList();

        }
    }
    // 如果用户点击了 "Cancel" 或者添加失败后关闭了窗口，exec() 会返回 Rejected，我们什么都不做
}

// [修改] on_createGroupButton_clicked
void MainWindow::on_createGroupButton_clicked()
{
    CreateGroupDialog dialog(this);
    dialog.setFriendsList(m_friends);

    if (dialog.exec() == QDialog::Accepted) {
        QString groupName = dialog.getGroupName().trimmed();
        if (groupName.isEmpty()) {
            qDebug() << "群名不能为空";
            return;
        }

        QVector<uint8_t> memberIds = dialog.getSelectedMemberIDs();

        // [修改] 暂存群名和成员列表
        m_pendingGroupName = groupName;
        m_pendingGroupMembers = memberIds; // 保存其他成员
        // [修改] 暂存群名，并调用网络接口
        m_pendingGroupName = groupName;
        NetworkManager::instance().sendCreateGroupRequest(groupName, memberIds);
    }
}


// [修改] updateConversationList，将ID存入data
void MainWindow::updateConversationList()
{
    ui->conversationListWidget->clear();
    m_conversationItems.clear(); // 清空指针映射

    // 添加好友
    for(auto it = m_friends.constBegin(); it != m_friends.constEnd(); ++it) {
        QString conversationId = QString::number(it.key());
        QString displayText = formatConversationDisplay(conversationId, it.value(), false);

        QListWidgetItem* item = new QListWidgetItem(displayText, ui->conversationListWidget);
        item->setData(Qt::UserRole, conversationId);

        // 设置未读样式
        int unreadCount = m_unreadCounts.value(conversationId, 0);
        if (unreadCount > 0) {
            setItemUnreadStyle(item, unreadCount);
        }

        m_conversationItems[conversationId] = item;
    }

    // 添加群聊
    for(auto it = m_groups.constBegin(); it != m_groups.constEnd(); ++it) {
        QString conversationId = it.key();
        QString displayText = formatConversationDisplay(conversationId, it.key(), true);

        QListWidgetItem* item = new QListWidgetItem(displayText, ui->conversationListWidget);
        item->setData(Qt::UserRole, conversationId);

        // 设置未读样式
        int unreadCount = m_unreadCounts.value(conversationId, 0);
        if (unreadCount > 0) {
            setItemUnreadStyle(item, unreadCount);
        }

        m_conversationItems[conversationId] = item;
    }
}

void MainWindow::onAutoAcceptFriendRequest(uint8_t requesterId)
{
    qDebug() << "[MainWindow] Auto-accepting and adding friend:" << requesterId;

    // 1. 检查是否已经是好友了，防止重复添加
    if (m_friends.contains(requesterId)) {
        qDebug() << "[MainWindow] Friend" << requesterId << "already exists.";
        // 即使已经是好友，也应该回复一个成功的消息，让对方能完成添加流程
    } else {
        // 2. 添加到你的好友数据中 (m_friends)
        // 我们暂时不知道新好友的名字，所以先用ID作为临时名字
        QString temporaryName = QString("用户 %1").arg(requesterId);
        m_friends.insert(requesterId, temporaryName);

        QMessageBox::information(this, "已自动接受好友添加请求", QString("已添加好友%1").arg(requesterId));
        // 3. 调用你已有的函数刷新UI
        updateConversationList();
    }

    // 4. [关键] 无论对方是否已经是好友，都回复服务器，告诉它你已经“同意”了
    // 这样可以确保发起请求的A端能够收到成功的响应
    uint8_t selfId = NetworkManager::instance().selfId(); // [注意] 这里需要获取当前用户的真实ID
    NetworkManager::instance().sendAddFriendResponse(requesterId, selfId, true);
}

// [修改] onNewMessageReceived
void MainWindow::onNewMessageReceived(const ChatMessage &message, const QString& conversationId)
{
    qDebug() << "[MainWindow] 收到新消息，对话ID:" << conversationId;

    // 临时：为群聊消息添加前缀，避免与私聊ID冲突
    QString actualConversationId = conversationId;

    // 检查是否是群聊（通过m_groups）
    if (m_groups.contains(conversationId)) {
        actualConversationId = "group_" + conversationId;
    }

    // 使用actualConversationId存储和显示
    m_chatHistories[actualConversationId].append(message);

    // 如果当前正在看这个对话，就刷新界面
    if (actualConversationId == m_currentConversationId) {
        updateChatHistoryView();
    } else {
        // 否则，可以在左侧列表项上显示未读标记
        int currentUnread = m_unreadCounts.value(actualConversationId, 0);
        m_unreadCounts[actualConversationId] = currentUnread + 1;
        // 更新对应的列表项显示
        updateConversationItem(conversationId);
        QApplication::beep();
        qDebug() << "收到非当前窗口消息，来自:" << conversationId;
    }
}

void MainWindow::updateConversationItem(const QString& conversationId)
{
    if (!m_conversationItems.contains(conversationId)) {
        return;
    }

    QListWidgetItem* item = m_conversationItems[conversationId];
    int unreadCount = m_unreadCounts.value(conversationId, 0);

    // 根据对话类型格式化文本
    QString displayText;
    bool isGroup = m_groups.contains(conversationId);

    if (isGroup) {
        QString groupName = conversationId;
        displayText = formatConversationDisplay(conversationId, groupName, true);
    } else {
        int friendId = conversationId.toInt();
        QString friendName = m_friends.value(friendId, QString("用户 %1").arg(friendId));
        displayText = formatConversationDisplay(conversationId, friendName, false);
    }

    item->setText(displayText);

    // 设置样式
    if (unreadCount > 0) {
        setItemUnreadStyle(item, unreadCount);
    } else {
        // 清除未读样式
        QFont font = item->font();
        font.setBold(false);
        item->setFont(font);
        item->setForeground(Qt::white); // 恢复默认颜色
    }
}

QString MainWindow::formatConversationDisplay(const QString& conversationId,
                                              const QString& name,
                                              bool isGroup)
{
    QString prefix = isGroup ? "群聊: " : "好友: ";
    int unreadCount = m_unreadCounts.value(conversationId, 0);

    if (unreadCount > 0) {
        return QString("(%1) %2%3").arg(unreadCount).arg(prefix).arg(name);
    }
    return prefix + name;
}
// 辅助函数：设置未读样式
void MainWindow::setItemUnreadStyle(QListWidgetItem* item, int unreadCount)
{
    // 设置粗体
    QFont font = item->font();
    font.setBold(true);
    item->setFont(font);

    // 设置文本颜色为红色
    item->setForeground(Qt::red);
}

// [新增] 实现槽函数
// [修改] onCreateGroupResult (创建者的处理逻辑)
void MainWindow::onCreateGroupResult(bool success, const QString& message)
{
    if (success) {
        qDebug() << "UI收到创建成功信号，群名:" << m_pendingGroupName;
        if (!m_pendingGroupName.isEmpty()) {

            // [核心修改] 创建 GroupInfo 对象并存储
            GroupInfo newGroup;
            newGroup.groupName = m_pendingGroupName;
            newGroup.memberIds = m_pendingGroupMembers; // 其他成员
            newGroup.memberIds.append(NetworkManager::instance().selfId()); // 把自己也加进去！

            m_groups[m_pendingGroupName] = newGroup;

            updateConversationList();
        }
    } else {
        qDebug() << "UI收到创建失败信号:" << message;
    }
    // 清理临时变量
    m_pendingGroupName.clear();
    m_pendingGroupMembers.clear();
}

// [修改] onAddedToNewGroup (被邀请者的处理逻辑)
void MainWindow::onAddedToNewGroup(const QString& groupName, uint8_t creatorId, const QVector<uint8_t>& memberIds)
{
    qDebug() << "UI收到被动加群信号，群名:" << groupName;

    if (!m_groups.contains(groupName)) {

        // [核心修改] 创建 GroupInfo 对象并存储
        GroupInfo newGroup;
        newGroup.groupName = groupName;
        newGroup.memberIds = memberIds; // 这是服务器发来的其他成员列表
        newGroup.memberIds.append(creatorId); // 把创建者也加进去！

        m_groups[groupName] = newGroup;

        updateConversationList();
        QMessageBox::information(this, "已被拉入群聊", QString("已被好友%1拉入群聊%2").arg(creatorId).arg(groupName));
    }
}
