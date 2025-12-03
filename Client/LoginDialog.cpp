#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include "NetworkManager.h" // 引入我们的网络核心
#include <QMessageBox>
#include "RegisterDialog.h"
#include "ui_registerdialog.h"
#include "SetServerDialog.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle("登录");

    // --- 核心：建立UI与NetworkManager之间的通信桥梁 ---
    // 获取NetworkManager的单例
    NetworkManager& netManager = NetworkManager::instance();

    // 1. 将NetworkManager的信号连接到本对话框的槽函数
    //    当网络管理器发出loginSuccess信号时，调用本类的onLoginSuccess方法
    connect(&netManager, &NetworkManager::loginSuccess, this, &LoginDialog::onLoginSuccess);
    //    当网络管理器发出loginFailed信号时，调用本类的onLoginFailed方法
    connect(&netManager, &NetworkManager::loginFailed, this, &LoginDialog::onLoginFailed);
    //    当网络意外断开时，也需要处理
    connect(&netManager, &NetworkManager::disconnected, this, &LoginDialog::onDisconnectedFromServer);

    // === 新增代码：连接超时信号 ===
    connect(&netManager, &NetworkManager::requestTimeout, this, &LoginDialog::onRequestTimeout);

    // 2. (可选但推荐) 启动程序时就尝试连接服务器
    //    这样用户在输入账号密码时，连接可能已经建立好了，体验更流畅

    QString host = "10.30.110.243";
    quint16 port = 8888;
    netManager.connectToServer(host, port);

}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_setServerButton_clicked(){
    SetServerDialog setServerDialog;
    setServerDialog.exec();
}

// 当用户点击“登录”按钮时
void LoginDialog::on_loginButton_clicked()
{
    // 1. 从UI获取用户输入
    QString usernameStr = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    // 2. 客户端级别的输入验证
    if (usernameStr.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名和密码不能为空！");
        return;
    }

    // 根据你的Packet设计，用户名是一个uint8_t的ID
    bool isNumeric;
    uint8_t userId = usernameStr.toUShort(&isNumeric); // toUShort可以安全地转为uint8_t
    if (!isNumeric) {
        QMessageBox::warning(this, "输入错误", "用户名必须是0-255之间的数字ID！");
        return;
    }

    // === 新增：将尝试登录的ID存入成员变量 ===
    m_attemptingUserId = userId;


    // 3. 将任务委托给NetworkManager
    NetworkManager::instance().sendLoginRequest(userId, password.toStdString());

    // 4. 更新UI，提供反馈，防止用户重复点击
    ui->loginButton->setEnabled(false);
    ui->registerButton->setEnabled(false);
    ui->loginButton->setText("登录中...");

}

// 当用户点击“注册”按钮时 (这个逻辑保持不变)
void LoginDialog::on_registerButton_clicked()
{
    // 1. 创建注册对话框
    RegisterDialog regDialog(this);

    // 2. 获取网络管理器的单例
    NetworkManager& netManager = NetworkManager::instance();

    // 3. **建立连接（核心）**
    //    连接1: 当注册窗口请求注册时 (发出信号)，让网络管理器去处理 (执行槽)
    connect(&regDialog, &RegisterDialog::registrationRequested,
            &netManager, &NetworkManager::onRegistrationRequested);

    //    连接2: 当网络管理器报告注册结果时 (发出信号)，我们在这里处理它 (执行Lambda函数)
    connect(&netManager, &NetworkManager::registrationResult,
            this, [&](bool success, const QString& msg) {
                if (success) {
                    QMessageBox::information(&regDialog, "注册成功", msg);
                    regDialog.accept();
                } else {
                    QMessageBox::warning(&regDialog, "注册失败", msg);

                    // ===================================================
                    // ===== 在这里添加新逻辑，用于恢复注册窗口的UI状态 =====
                    // ===================================================
                    regDialog.ui->registerButton->setEnabled(true);
                    regDialog.ui->backButton->setEnabled(true);
                    regDialog.ui->registerButton->setText("注册");
                }
            });

    // 4. 以模态方式显示注册对话框，程序会在这里暂停，直到对话框关闭
    regDialog.exec();

    // 5. (好习惯) 对话框关闭后，断开所有与它相关的连接，防止意外
    disconnect(&regDialog, &RegisterDialog::registrationRequested, &netManager, &NetworkManager::onRegistrationRequested);
    disconnect(&netManager, &NetworkManager::registrationResult, this, nullptr);

}

// --- 响应NetworkManager信号的槽函数实现 ---

// 当收到登录成功信号时
void LoginDialog::onLoginSuccess()
{
    // 登录成功，关闭对话框并返回Accepted
    // 使用我们刚刚在 on_loginButton_clicked 中保存的ID
    NetworkManager::instance().setCurrentUserId(m_attemptingUserId);
    // exec()的调用处会收到这个结果，从而知道可以进入主界面了
    QMessageBox::information(this, "成功", "登录成功！");
    accept();
}

// 当收到登录失败信号时
void LoginDialog::onLoginFailed()
{
    // 登录失败，显示错误信息
    QMessageBox::warning(this, "登录失败", "用户名或密码错误！");

    // 恢复UI，让用户可以重新尝试
    ui->loginButton->setEnabled(true);
    ui->registerButton->setEnabled(true);
    ui->loginButton->setText("登录");
}

// 当与服务器的连接意外断开时
void LoginDialog::onDisconnectedFromServer()
{
    QMessageBox::critical(this, "连接错误", "与服务器的连接已断开！");

    // 同样需要恢复UI
    ui->loginButton->setEnabled(true);
    ui->registerButton->setEnabled(true);
    ui->loginButton->setText("登录");
}

// === 新增代码：实现超时槽函数 ===
void LoginDialog::onRequestTimeout()
{
    // 显示超时错误信息
    QMessageBox::critical(this, "请求超时", "服务器在5秒内未响应，请检查网络或稍后再试。");

    // 恢复UI状态，让用户可以重新尝试
    ui->loginButton->setEnabled(true);
    ui->registerButton->setEnabled(true);
    ui->loginButton->setText("登录");
}
