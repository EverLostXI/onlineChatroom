// MainWindow.cpp

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "AddFriendDialog.h"
#include <QListWidgetItem> // 如果槽函数参数用到了，需要包含头文件
#include "networkmanager.h"

// ... 其他代码 ...

// 构造函数可能已经有了
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // ...
    connect(&NetworkManager::instance(), &NetworkManager::autoAcceptFriendRequest, this, &MainWindow::onAutoAcceptFriendRequest);
    connect(&NetworkManager::instance(), &NetworkManager::newMessageReceived, this, &MainWindow::onNewMessageReceived);

    // --- 添加初始的假数据 ---////////////////////////
    /////////////////////////////////////////////////
    m_friends[123] = "张三";
    m_friends[124] = "李四";
    m_groups[125] = "朋友群";
    // 假设我们自己的ID是 50
    int myUserId = 50;

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
    m_chatHistories.insert(123, zhangsanHistory);


    // --- 2. 创建和李四(124)的聊天记录 ---
    QList<ChatMessage> lisiHistory;

    ChatMessage msg3;
    msg3.senderId = 124; // 李四发的
    msg3.text = "那个项目文档你发我一下。";
    msg3.timestamp = QDateTime::currentDateTime(); // 假设是刚刚
    lisiHistory.append(msg3);

    // 把和李四的完整聊天记录列表，存入“大柜子”m_chatHistories
    m_chatHistories.insert(124, lisiHistory);

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
    m_chatHistories.insert(125, groupHistory);

    // =======================================================
    // === 测试数据添加完毕 ====================================
    // =======================================================
    // 启动时刷新一次列表
    updateConversationList();
}

// ！！！确保析构函数的实现存在 ！！！
MainWindow::~MainWindow()
{
    delete ui;
}

// ！！！为所有槽函数添加实现 ！！！

// 这个函数在用户点击“发送”按钮时被自动调用
void MainWindow::on_sendButton_clicked()
{
    // --- 1. 前置检查 (这里没有变化) ---
    if (m_currentConversationId == -1) {
        return;
    }
    QString text = ui->messageInputTextEdit->toPlainText();
    if (text.trimmed().isEmpty()) {
        return;
    }

    // --- 2. 本地更新 (这里有小修改) ---
    // 注意：我们需要正确获取自己的ID。暂时，我们使用 NetworkManager 里的那个。
    // 登录成功后，这里必须更新为真实登录用户的ID。
    uint8_t myUserId = NetworkManager::instance().selfId();

    ChatMessage newMessage;
    newMessage.senderId = myUserId; // 使用一致的ID
    newMessage.text = text;
    newMessage.timestamp = QDateTime::currentDateTime();

    m_chatHistories[m_currentConversationId].append(newMessage);
    updateChatHistoryView();
    ui->messageInputTextEdit->clear();

    // --- 3. 网络发送 (这是更新的部分) ---
    NetworkManager::instance().sendMessage(myUserId, m_currentConversationId, text);
}

// 这个函数在用户点击左侧列表时被自动调用
void MainWindow::on_conversationListWidget_itemClicked(QListWidgetItem *item)
{
    // 1. 从 item 的文本中解析出 ID
    // 例如，从 "好友: 张三 (1001)" 中提取出 1001
    QString text = item->text();
    int id_start = text.lastIndexOf('(');
    int id_end = text.lastIndexOf(')');

    if (id_start != -1 && id_end != -1) {
        QString idStr = text.mid(id_start + 1, id_end - id_start - 1);
        bool ok;
        int id = idStr.toInt(&ok);

        if (ok) {
            // 2. 更新当前选中的对话ID
            m_currentConversationId = id;

            // 3. 调用刷新函数，让右边显示对应的聊天记录
            updateChatHistoryView();
        }
    }
}

// 这个函数专门根据 m_currentConversationId 来刷新右侧的聊天窗口
void MainWindow::updateChatHistoryView()
{
    // 1. 先清空当前的聊天记录显示
    ui->chatHistoryListWidget->clear();

    // 2. 检查当前是否选中了一个有效的对话
    if (m_currentConversationId == -1 || !m_chatHistories.contains(m_currentConversationId)) {
        return; // 如果没选中或没有这个人的聊天记录，就直接返回
    }

    // 3. 获取我们自己的ID
    uint8_t myUserId = NetworkManager::instance().selfId(); // 确保ID来源统一

    // 4. 从“大柜子”里根据ID拿出对应的聊天记录列表
    const QList<ChatMessage>& messages = m_chatHistories[m_currentConversationId];

    // 5. 遍历这个列表里的每一条消息，并把它添加到界面上
    for (const ChatMessage& msg : messages)
    {
        // 创建一个新的列表项
        QListWidgetItem* item = new QListWidgetItem();

        // 格式化要显示的文本
        QString displayText;
        if (msg.senderId == myUserId) {
            // 如果是我发的消息
            displayText = "我: " + msg.text;
            // 设置文本靠右对齐
            item->setTextAlignment(Qt::AlignRight);
        } else {
            // 如果是别人发的消息
            displayText = QString::number(msg.senderId) + ": " + msg.text;
            // 设置文本靠左对齐
            item->setTextAlignment(Qt::AlignLeft);
        }

        item->setText(displayText);
        ui->chatHistoryListWidget->addItem(item);
        ui->chatHistoryListWidget->scrollToBottom();
    }
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

void MainWindow::on_createGroupButton_clicked()
{
    CreateGroupDialog dialog(this);

    // 1. 把当前的好友列表传给对话框
    dialog.setFriendsList(m_friends);

    // 2. 显示对话框并等待用户操作
    if (dialog.exec() == QDialog::Accepted) {
        // 3. 从对话框获取结果
        QString groupName = dialog.getGroupName();
        QVector<uint8_t> memberIds = dialog.getSelectedMemberIDs(); // 假设返回的是ID

        // 4. 为了测试，我们创建一个新的群ID
        // 简单的逻辑：取当前最大群ID+1，或者一个随机数
        int newGroupId = 2001;
        if (!m_groups.isEmpty()) {
            newGroupId = m_groups.lastKey() + 1;
        }

        // 5. 更新我们的“本地数据库”
        m_groups[newGroupId] = groupName;

        // 6. 刷新主界面
        updateConversationList();

        // (可选) 在控制台打印出群成员，确认选择正确
        qDebug() << "创建了新群聊:" << groupName << "ID:" << newGroupId;
        qDebug() << "群成员ID:";
        for(uint8_t id : memberIds) {
            qDebug() << id;
        }
    }
}

void MainWindow::updateConversationList()
{
    // 1. 先清空当前的列表
    ui->conversationListWidget->clear();

    // 2. 添加所有好友到列表
    for(auto it = m_friends.constBegin(); it != m_friends.constEnd(); ++it) {
        QString itemText = QString("好友: %1 (%2)").arg(it.value()).arg(it.key());
        ui->conversationListWidget->addItem(itemText);
    }

    // 3. 添加所有群聊到列表
    for(auto it = m_groups.constBegin(); it != m_groups.constEnd(); ++it) {
        QString itemText = QString("群聊: %1 (%2)").arg(it.value()).arg(it.key());
        ui->conversationListWidget->addItem(itemText);
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

        // 3. 调用你已有的函数刷新UI
        updateConversationList();
    }

    // 4. [关键] 无论对方是否已经是好友，都回复服务器，告诉它你已经“同意”了
    // 这样可以确保发起请求的A端能够收到成功的响应
    uint8_t selfId = NetworkManager::selfId(); // [注意] 这里需要获取当前用户的真实ID
    NetworkManager::instance().sendAddFriendResponse(requesterId, selfId, true);
}

void MainWindow::onNewMessageReceived(const ChatMessage &message, int conversationId)
{
    // 1. 把收到的消息存入我们的“大柜子”
    m_chatHistories[conversationId].append(message);

    // 2. 如果我们当前正在查看这个对话，就刷新UI
    if (conversationId == m_currentConversationId) {
        updateChatHistoryView();
    } else {
        // 未来可以增强的功能：显示一个通知或未读红点
        qDebug() << "收到一条来自对话 " << conversationId << " 的消息，但当前未查看该窗口。";
    }
}
