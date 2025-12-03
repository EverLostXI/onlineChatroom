#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    // 这个枚举或常量最好定义在一个公共的头文件中，但放在这里也可以工作
    enum DialogCode {
        RegisterRequest = 100 // 自定义一个用于请求注册的返回码
    };


private slots:

    void onRequestTimeout();

    void on_loginButton_clicked();
    void on_registerButton_clicked();
    void on_setServerButton_clicked();

    // === 新增的槽函数，用于响应NetworkManager的信号 ===
    void onLoginSuccess();
    void onLoginFailed();
    void onDisconnectedFromServer(); // 处理意外断开连接的情况

private:
    Ui::LoginDialog *ui;
    // === 新增：临时存储尝试登录的ID ===
    uint8_t m_attemptingUserId;

};

#endif // LOGINDIALOG_H
