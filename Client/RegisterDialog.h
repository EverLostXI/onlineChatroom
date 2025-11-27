#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

signals: // <--- 添加 signals 区域
    // 定义一个信号，当用户点击注册时，就发射这个信号
    // 它会把用户名和密码作为参数“喊”出去
    void registrationRequested(const QString& username, const QString& password);

private slots:
    // "注册" 按钮的槽函数
    void on_registerButton_clicked();

    // "返回" 按钮的槽函数
    void on_backButton_clicked();

private:
    Ui::RegisterDialog *ui;
};

#endif // REGISTERDIALOG_H
