#ifndef ADDFRIENDDIALOG_H
#define ADDFRIENDDIALOG_H

#include <QDialog>

namespace Ui {
class AddFriendDialog;
}

class AddFriendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFriendDialog(QWidget *parent = nullptr);
    ~AddFriendDialog();

    int getFriendId() const; // <-- 添加这个函数声明

private:
    Ui::AddFriendDialog *ui;
};

#endif // ADDFRIENDDIALOG_H
