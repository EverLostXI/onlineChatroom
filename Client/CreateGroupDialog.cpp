#include "creategroupdialog.h"
#include "ui_creategroupdialog.h"
#include <QListWidgetItem>

CreateGroupDialog::CreateGroupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateGroupDialog)
{
    ui->setupUi(this);
    setWindowTitle("创建群聊");
    // 设置为多选模式，虽然在UI里设置了，但在代码里再设置一次更保险
    ui->friendsListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
}

CreateGroupDialog::~CreateGroupDialog()
{
    delete ui;
}

// 实现公共接口：接收好友列表并显示
void CreateGroupDialog::setFriendsList(const QMap<int, QString>& friends)
{
    ui->friendsListWidget->clear();
    // QMap的迭代器很方便
    for (auto it = friends.constBegin(); it != friends.constEnd(); ++it) {
        // it.key() 是 ID, it.value() 是名字
        QListWidgetItem* item = new QListWidgetItem(it.value(), ui->friendsListWidget);
        item->setData(Qt::UserRole, it.key()); // 将好友ID存入item
    }
}

// 实现公共接口：获取群名
QString CreateGroupDialog::getGroupName() const
{
    return ui->groupNameLineEdit->text();
}

// 实现公共接口：获取被选中的好友ID
QVector<uint8_t> CreateGroupDialog::getSelectedMemberIDs() const
{
    QVector<uint8_t> selectedIDs;
    // 获取所有被选中的项
    QList<QListWidgetItem*> selectedItems = ui->friendsListWidget->selectedItems();

    for (QListWidgetItem* item : selectedItems) {
        // 取出我们之前存进去的ID
        selectedIDs.append(static_cast<uint8_t>(item->data(Qt::UserRole).toInt()));
    }
    return selectedIDs;
}

QListWidget* CreateGroupDialog::getFriendsListWidget() const
{
    return ui->friendsListWidget;
}
