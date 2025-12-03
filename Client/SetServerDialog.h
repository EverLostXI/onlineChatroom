#ifndef SETSERVERDIALOG_H
#define SETSERVERDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class SetServerDialog;
}

class SetServerDialog : public QDialog
{
    Q_OBJECT

public:
    // 获取设置的方法
    QString getServerAddress() const;
    quint16 getServerPort() const;

    explicit SetServerDialog(QWidget *parent = nullptr);
    ~SetServerDialog();

signals:
    // 声明信号
    void serverEndPoint(const QString& address, quint16 port);

private slots:
    void on_confirmButton_clicked();
    void on_cancelButton_clicked();

private:
    // 添加缺失的私有方法声明
    void setupValidation();
    bool isValidIPAddress(const QString& ip);

    QString m_serverAddress;
    quint16 m_serverPort;

    Ui::SetServerDialog *ui;
};
#endif // SETSERVERDIALOG_H
