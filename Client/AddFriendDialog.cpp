#include "AddFriendDialog.h"
#include "ui_AddFriendDialog.h"

AddFriendDialog::AddFriendDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddFriendDialog)
{
    ui->setupUi(this);
}

AddFriendDialog::~AddFriendDialog()
{
    delete ui;
}

int AddFriendDialog::getFriendId() const
{
    // ui->lineEdit 是你在 Designer 里那个输入框的 objectName
    // 如果你的输入框叫别的名字，请修改这里
    return ui->friendIdLineEdit->text().toInt();
}
