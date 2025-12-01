#include "AddFriendDialog.h"
#include "ui_AddFriendDialog.h"

#include "networkmanager.h" // 引入NetworkManager
#include <QMessageBox>
#include <QIntValidator> // 用于输入验证



AddFriendDialog::AddFriendDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddFriendDialog)
    , m_addedFriendId(-1) // 初始化为无效值
{
    ui->setupUi(this);

    // 1. 设置输入框只接受0-255的整数
    // 确保你的输入框 objectName 是 friendIdLineEdit
    ui->friendIdLineEdit->setValidator(new QIntValidator(0, 255, this));

    // 2. 连接"OK"按钮的点击事件到我们的处理函数
    // 假设你的按钮是标准 QDialogButtonBox 的一部分
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddFriendDialog::onOkButtonClicked);

    // 3. 连接 NetworkManager 的信号到我们的槽
    connect(&NetworkManager::instance(), &NetworkManager::addFriendResult, this, &AddFriendDialog::onAddFriendResponse);
}

AddFriendDialog::~AddFriendDialog()
{
    // [新增] 断开连接，防止对话框销毁后槽函数依然被调用
    disconnect(&NetworkManager::instance(), &NetworkManager::addFriendResult, this, &AddFriendDialog::onAddFriendResponse);
    delete ui;
}

// 当点击"OK"时触发
void AddFriendDialog::onOkButtonClicked()
{
    // 1. 获取并验证输入
    QString idText = ui->friendIdLineEdit->text();
    if (idText.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "好友ID不能为空！");
        return;
    }

    bool ok;
    int friendId = idText.toInt(&ok);
    // 范围验证其实已经被QIntValidator处理了，但双重检查更安全
    if (!ok || friendId < 0 || friendId > 255) {
        QMessageBox::warning(this, "输入错误", "请输入一个0到255之间的有效ID。");
        return;
    }

    // 2. 禁用按钮，防止重复点击
    ui->buttonBox->setEnabled(false);

    // 3. 调用NetworkManager发送请求
    // [注意] 自己的ID暂时硬编码为1，后续需要从登录信息中获取
    uint8_t selfId = NetworkManager::instance().selfId();
    NetworkManager::instance().sendAddFriendRequest(selfId, static_cast<uint8_t>(friendId));

    // 注意：我们在这里不关闭对话框，而是等待服务器的响应
}

// 当收到服务器响应时，NetworkManager会发射信号，触发这个槽函数
void AddFriendDialog::onAddFriendResponse(bool success, uint8_t friendId)
{
    // 重新启用按钮
    ui->buttonBox->setEnabled(true);

     qDebug() << "[AddFriendDialog] Slot onAddFriendResponse triggered! Success:" << success << "Friend ID:" << friendId;

    if (success) {
        QMessageBox::information(this, "成功", QString("成功添加好友 %1！").arg(friendId));
        m_addedFriendId = friendId; // 记录下成功添加的ID
        accept(); // 关闭对话框，并返回 QDialog::Accepted
    } else {
        QMessageBox::critical(this, "失败", QString("添加好友 %1 失败，请检查ID是否正确或对方是否已是好友。").arg(friendId));
        // 不关闭对话框，用户可以修改ID后重试
    }
}

// 实现新的getter函数
int AddFriendDialog::getAddedFriendId() const
{
    return m_addedFriendId;
}
