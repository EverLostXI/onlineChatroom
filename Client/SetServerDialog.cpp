// SetServerDialog.cpp
#include "SetServerDialog.h"
#include "ui_SetServerDialog.h"
#include "NetworkManager.h"

#include "QDebug"
#include <QMessageBox>

#include <QRegularExpression>
#include <QRegularExpressionValidator>  // 添加这行
#include <QIntValidator>                 // 添加这行

SetServerDialog::SetServerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SetServerDialog)
{
    ui->setupUi(this);
    setWindowTitle("Server setting");

    // 设置默认值
    ui->serverAddress->setText("10.30.110.243");
    ui->serverPort->setText("8888");

    // 添加输入验证
    setupValidation();
}

SetServerDialog::~SetServerDialog()
{
    delete ui;
}

void SetServerDialog::setupValidation()
{
    // IP地址验证
    QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    ui->serverAddress->setValidator(new QRegularExpressionValidator(ipRegex, this));

    // 端口号验证（1-65535）
    ui->serverPort->setValidator(new QIntValidator(1, 65535, this));
}

void SetServerDialog::on_confirmButton_clicked()
{
    QString serverAddress = ui->serverAddress->text().trimmed();
    QString serverPortStr = ui->serverPort->text().trimmed();

    // 检查输入是否为空
    if (serverAddress.isEmpty() || serverPortStr.isEmpty()) {
        QMessageBox::warning(this, "Setting failed!", "Server address and server port cannot be empty!");
        return;
    }

    // 验证IP地址格式
    if (!isValidIPAddress(serverAddress)) {
        QMessageBox::warning(this, "Invalid IP", "Please enter a valid IP address!");
        ui->serverAddress->setFocus();
        return;
    }

    // 验证端口号 - 修复了您的bug：ok变量没有正确使用
    bool ok = false;
    quint16 serverPort = serverPortStr.toUShort(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Invalid input!", "Port must be a number between 1-65535!");
        ui->serverPort->setFocus();
        return;
    }

    // 保存设置（可选）
    //m_serverAddress = serverAddress;
    //m_serverPort = serverPort;

    // 发射信号
    //emit serverEndPoint(serverAddress, serverPort);

    // 获取NetworkManager的单例
    NetworkManager& netManager = NetworkManager::instance();
    netManager.connectToServer(serverAddress, serverPort);

    // 关闭对话框
    accept();

    qDebug() << "Server settings confirmed - Address:" << serverAddress << "Port:" << serverPort;
}

bool SetServerDialog::isValidIPAddress(const QString& ip)
{
    if (ip == "localhost") return true;

    QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return ipRegex.match(ip).hasMatch();
}

void SetServerDialog::on_cancelButton_clicked()
{
    reject();
}

/*
QString SetServerDialog::getServerAddress() const
{
    return m_serverAddress;
}

quint16 SetServerDialog::getServerPort() const
{
    return m_serverPort;
}
*/
