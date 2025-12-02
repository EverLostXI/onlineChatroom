/********************************************************************************
** Form generated from reading UI file 'LoginDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINDIALOG_H
#define UI_LOGINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginDialog
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLabel *label_2;
    QWidget *layoutWidget1;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QWidget *layoutWidget2;
    QVBoxLayout *verticalLayout_3;
    QPushButton *registerButton;
    QPushButton *loginButton;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout_4;
    QPushButton *connectButton;
    QPushButton *setServerButton;

    void setupUi(QDialog *LoginDialog)
    {
        if (LoginDialog->objectName().isEmpty())
            LoginDialog->setObjectName("LoginDialog");
        LoginDialog->resize(439, 300);
        layoutWidget = new QWidget(LoginDialog);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(40, 50, 101, 111));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(layoutWidget);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        layoutWidget1 = new QWidget(LoginDialog);
        layoutWidget1->setObjectName("layoutWidget1");
        layoutWidget1->setGeometry(QRect(170, 40, 221, 141));
        verticalLayout_2 = new QVBoxLayout(layoutWidget1);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        usernameLineEdit = new QLineEdit(layoutWidget1);
        usernameLineEdit->setObjectName("usernameLineEdit");

        verticalLayout_2->addWidget(usernameLineEdit);

        passwordLineEdit = new QLineEdit(layoutWidget1);
        passwordLineEdit->setObjectName("passwordLineEdit");

        verticalLayout_2->addWidget(passwordLineEdit);

        layoutWidget2 = new QWidget(LoginDialog);
        layoutWidget2->setObjectName("layoutWidget2");
        layoutWidget2->setGeometry(QRect(160, 200, 231, 54));
        verticalLayout_3 = new QVBoxLayout(layoutWidget2);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        registerButton = new QPushButton(layoutWidget2);
        registerButton->setObjectName("registerButton");

        verticalLayout_3->addWidget(registerButton);

        loginButton = new QPushButton(layoutWidget2);
        loginButton->setObjectName("loginButton");

        verticalLayout_3->addWidget(loginButton);

        verticalLayoutWidget = new QWidget(LoginDialog);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(30, 189, 121, 71));
        verticalLayout_4 = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        connectButton = new QPushButton(verticalLayoutWidget);
        connectButton->setObjectName("connectButton");

        verticalLayout_4->addWidget(connectButton);

        setServerButton = new QPushButton(verticalLayoutWidget);
        setServerButton->setObjectName("setServerButton");

        verticalLayout_4->addWidget(setServerButton);


        retranslateUi(LoginDialog);

        QMetaObject::connectSlotsByName(LoginDialog);
    } // setupUi

    void retranslateUi(QDialog *LoginDialog)
    {
        LoginDialog->setWindowTitle(QCoreApplication::translate("LoginDialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("LoginDialog", "<html><head/><body><p><span style=\" font-size:14pt;\">Username</span></p></body></html>", nullptr));
        label_2->setText(QCoreApplication::translate("LoginDialog", "<html><head/><body><p><span style=\" font-size:14pt;\">Password</span></p></body></html>", nullptr));
        usernameLineEdit->setText(QString());
        registerButton->setText(QCoreApplication::translate("LoginDialog", "Register", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginDialog", "Login", nullptr));
        connectButton->setText(QCoreApplication::translate("LoginDialog", "Connect to server", nullptr));
        setServerButton->setText(QCoreApplication::translate("LoginDialog", "Set Server", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginDialog: public Ui_LoginDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
