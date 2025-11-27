#include "widget.h"
#include "logindialog.h"
#include "registerdialog.h"
#include "networkmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    Widget w;
    LoginDialog loginDialog;

    // 显示登录对话框。如果用户登录成功 (返回 Accepted)，则继续
    if (loginDialog.exec() == QDialog::Accepted) {
        // 登录成功，显示主窗口
        w.show();
        return a.exec();
    }

    // 如果用户关闭了登录窗口或登录失败，程序直接退出
    return 0;
}
