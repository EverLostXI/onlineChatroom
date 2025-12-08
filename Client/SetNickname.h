#ifndef SETNICKNAME_H
#define SETNICKNAME_H

#include <QDialog>

namespace Ui {
class SetNickname;
}

class SetNickname : public QDialog
{
    Q_OBJECT

public:
    explicit SetNickname(QWidget *parent = nullptr);
    ~SetNickname();

private slots:
    void on_confirmButton_clicked();
    void on_cancelButton_clicked();

signals:
    void nicknameChanged(const QString& newNickname);  // 新增信号

private:
    Ui::SetNickname *ui;

    QString nickname = "用户0";

    void saveSettings();
    void loadSettings();
};

#endif // SETNICKNAME_H
