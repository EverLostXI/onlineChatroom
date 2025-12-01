#include "SetServerDialog.h"
#include "ui_SetServerDialog.h"
#include <QMessageBox>

SetServerDialog::SetServerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SetServerDialog)
{
    ui->setupUi(this);
}

SetServerDialog::~SetServerDialog()
{
    delete ui;
}

void SetServerDialog::on_confirmButton_clicked()
{
    QString serverAddress = ui->serverAddress->text();
    QString serverPort = ui->serverPort->text();

    //检查输入是否为空
    if (serverAddress.isEmpty() || serverPort.isEmpty()) {
        QMessageBox::warning(this, "Setting failed!", "Server address and server port cannot be empty！");
        return;
    }
    //未接信号
}

void SetServerDialog::on_cancelButton_clicked()
{
    reject();
}
