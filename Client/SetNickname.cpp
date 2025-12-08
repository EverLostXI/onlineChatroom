#include "SetNickname.h"
#include "ui_SetNickname.h"
#include <QSettings>
#include <QMessageBox>

SetNickname::SetNickname(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SetNickname)
{
    ui->setupUi(this);
    setWindowTitle("更改昵称");

    loadSettings();
}

SetNickname::~SetNickname()
{
    delete ui;
}

void SetNickname::on_confirmButton_clicked()
{
    nickname = ui->nickname->text().trimmed();

    // 检查输入是否为空
    if (nickname.isEmpty()) {
        QMessageBox::warning(this, "设置失败!", "昵称不能为空!");
        return;
    }

    ui->nickname->setFocus();


    // 保存设置到文件
    saveSettings();

    // 发射信号通知昵称已更改
    emit nicknameChanged(nickname);
    // 关闭对话框
    accept();
}

void SetNickname::on_cancelButton_clicked()
{
    loadSettings();
    reject();
}

void SetNickname::saveSettings(){
    QSettings settings("CSC3002", "Chatroom");
    settings.setValue("Client/Nickname", nickname);
}

void SetNickname::loadSettings(){
    QSettings settings("CSC3002", "Chatroom");

    // 从设置文件读取，如果没有则使用默认值
    QString savedNickname = settings.value("Client/Nickname", "用户0").toString();
    nickname = savedNickname;
    // 更新UI显示
    ui->nickname->setText(nickname);
}
