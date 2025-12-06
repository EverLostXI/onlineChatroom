#include "SetNickname.h"
#include "ui_SetNickname.h"

SetNickname::SetNickname(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SetNickname)
{
    ui->setupUi(this);
}

SetNickname::~SetNickname()
{
    delete ui;
}
