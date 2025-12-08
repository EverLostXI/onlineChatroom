// SetServerDialog.cpp
#include "SetServerDialog.h"
#include "ui_SetServerDialog.h"
#include "NetworkManager.h"

#include "QDebug"
#include <QMessageBox>
#include <QSettings>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QIntValidator>

SetServerDialog::SetServerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SetServerDialog)
{
    ui->setupUi(this);
    setWindowTitle("服务器设置");

    // 从设置文件中加载保存的值
    loadSettings();

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
    serverAddress = ui->serverAddress->text().trimmed();
    serverPortStr = ui->serverPort->text().trimmed();

    // 检查输入是否为空
    if (serverAddress.isEmpty() || serverPortStr.isEmpty()) {
        QMessageBox::warning(this, "设置失败!", "服务器IP或端口不能为空!");
        return;
    }

    // 验证IP地址格式
    if (!isValidIPAddress(serverAddress)) {
        QMessageBox::warning(this, "输入无效！", "请输入有效的IP地址!");
        ui->serverAddress->setFocus();
        return;
    }

    // 验证端口号
    bool ok = false;
    quint16 serverPort = serverPortStr.toUShort(&ok);
    if (!ok) {
        QMessageBox::warning(this, "输入无效!", "端口号应为1-65535的整数!");
        ui->serverPort->setFocus();
        return;
    }

    // 保存设置到文件
    saveSettings();

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
    loadSettings();
    reject();
}

// 新添加的函数：保存设置到配置文件
void SetServerDialog::saveSettings()
{
    QSettings settings("CSC3002", "Chatroom"); // 可以改为您的公司和应用名称
    settings.setValue("Server/Address", serverAddress);
    settings.setValue("Server/Port", serverPortStr);
}

// 新添加的函数：从配置文件加载设置
void SetServerDialog::loadSettings()
{
    QSettings settings("CSC3002", "Chatroom");

    // 从设置文件读取，如果没有则使用默认值
    QString savedAddress = settings.value("Server/Address", "10.30.110.243").toString();
    QString savedPort = settings.value("Server/Port", "8888").toString();

    // 更新成员变量
    serverAddress = savedAddress;
    serverPortStr = savedPort;

    // 更新UI显示
    ui->serverAddress->setText(serverAddress);
    ui->serverPort->setText(serverPortStr);
}

// 可选：添加获取当前设置的方法（如果需要外部访问）
QString SetServerDialog::getCurrentAddress() const
{
    return serverAddress;
}

QString SetServerDialog::getCurrentPort() const
{
    return serverPortStr;
}
