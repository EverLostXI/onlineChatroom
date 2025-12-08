// SetServerDialog.h
#ifndef SETSERVERDIALOG_H
#define SETSERVERDIALOG_H

#include <QDialog>

namespace Ui {
class SetServerDialog;
}

class SetServerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetServerDialog(QWidget *parent = nullptr);
    ~SetServerDialog();

    // 获取当前设置的方法
    QString getCurrentAddress() const;
    QString getCurrentPort() const;

private slots:
    void on_confirmButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::SetServerDialog *ui;

    // 成员变量
    QString serverAddress = "10.30.110.243";
    QString serverPortStr = "8888";

    void setupValidation();
    bool isValidIPAddress(const QString& ip);

    // 新增：保存和加载设置的方法
    void saveSettings();
    void loadSettings();
};

#endif // SETSERVERDIALOG_H
