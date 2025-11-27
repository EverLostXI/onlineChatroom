#include "widget.h"
#include "logindialog.h"
#include "registerdialog.h"
#include "networkmanager.h"
#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    MainWindow w;
    LoginDialog loginDialog;
//
    w.show();
    return a.exec();

//以上为主聊天界面测试，若需测试登录注册功能可将上两行删除，启用下面代码

/*
    // 显示登录对话框。如果用户登录成功 (返回 Accepted)，则继续
    if (loginDialog.exec() == QDialog::Accepted) {
        // 登录成功，显示主窗口
        w.show();
        return a.exec();
    }

    // 如果用户关闭了登录窗口或登录失败，程序直接退出
    //return 0;
*/
}
