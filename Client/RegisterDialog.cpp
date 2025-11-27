#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <QMessageBox>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    setWindowTitle("注册新账号");
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_registerButton_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString confirm = ui->confirmPasswordLineEdit_2->text();

    // 1. 检查输入是否为空
    if (username.isEmpty() || password.isEmpty() || confirm.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "用户名和密码不能为空！");
        return;
    }

    // 2. 检查两次输入的密码是否一致
    if (password != confirm) {
        QMessageBox::warning(this, "注册失败", "两次输入的密码不一致！");
        return;
    }

    // 3. 在这里添加实际的注册逻辑
    // 例如，将用户名和密码保存到数据库或文件中。
    // 为了演示，我们只显示一个成功的消息。
    // TODOTODOTODO: 实现真正的账号存储逻辑
    // 3. 发射信号，把注册任务交给 NetworkManager
    emit registrationRequested(username, password);

    // 注册成功后，关闭对话框并返回 Accepted 状态
    //accept();
}

void RegisterDialog::on_backButton_clicked()
{
    // 点击返回，关闭对话框并返回 Rejected 状态
    reject();
}
