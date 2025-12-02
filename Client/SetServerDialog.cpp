#include "SetServerDialog.h"
#include "ui_SetServerDialog.h"
#include <QMessageBox>

SetServerDialog::SetServerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SetServerDialog)
{
    ui->setupUi(this);
    setWindowTitle("Set Server");
}

SetServerDialog::~SetServerDialog()
{
    delete ui;
}

void SetServerDialog::on_confirmButton_clicked()
{
    QString serverAddress = ui->serverAddress->text();
    QString serverPort1 = ui->serverPort->text();

    //检查输入是否为空
    if (serverAddress.isEmpty() || serverPort1.isEmpty()) {
        QMessageBox::warning(this, "Setting failed!", "Server address and server port cannot be empty！");
        return;
    }
    bool ok = false;
    quint16 serverPort = serverPort1.toUShort();
    if(ok){
        emit serverEndPoint(serverAddress,serverPort);
    }else{
        QMessageBox::warning(this,"Invalid input!","Invalid port input!");
    }
}

void SetServerDialog::on_cancelButton_clicked()
{
    reject();
}
