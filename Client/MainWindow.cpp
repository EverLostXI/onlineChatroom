// MainWindow.cpp

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "AddFriendDialog.h"
#include <QListWidgetItem> // 如果槽函数参数用到了，需要包含头文件

// ... 其他代码 ...

// 构造函数可能已经有了
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // ...
    // --- 添加初始的假数据 ---
    m_friends[1001] = "张三";
    m_friends[1002] = "李四";
    m_groups[2001] = "家庭群";

    // 启动时刷新一次列表
    updateConversationList();
}

// ！！！确保析构函数的实现存在 ！！！
MainWindow::~MainWindow()
{
    delete ui;
}

// ！！！为所有槽函数添加实现 ！！！

void MainWindow::on_sendButton_clicked()
{
    // 这里暂时可以为空，但函数体必须存在
}

void MainWindow::on_conversationListWidget_itemClicked(QListWidgetItem *item)
{
    (void)item;
}

void MainWindow::on_addFriendButton_clicked()
{
    AddFriendDialog dialog(this);
    // dialog.exec() 会阻塞程序，直到对话框关闭
    // 如果用户点击 OK，exec() 返回 QDialog::Accepted
    if (dialog.exec() == QDialog::Accepted) {
        int friendId = dialog.getFriendId();

        // 为了测试，我们给新好友一个默认名字
        QString friendName = QString("新好友%1").arg(friendId);

        // 更新我们的“本地数据库”
        m_friends[friendId] = friendName;

        // 刷新主界面列表，立即看到变化！
        updateConversationList();
    }
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
