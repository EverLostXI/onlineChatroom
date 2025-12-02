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
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLabel *label_2;
    QWidget *widget1;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QWidget *widget2;
    QVBoxLayout *verticalLayout_3;
    QPushButton *registerButton;
    QPushButton *loginButton;

    void setupUi(QDialog *LoginDialog)
    {
        if (LoginDialog->objectName().isEmpty())
            LoginDialog->setObjectName("LoginDialog");
        LoginDialog->resize(400, 300);
        widget = new QWidget(LoginDialog);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(40, 50, 93, 111));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(widget);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        label_2 = new QLabel(widget);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        widget1 = new QWidget(LoginDialog);
        widget1->setObjectName("widget1");
        widget1->setGeometry(QRect(140, 40, 231, 141));
        verticalLayout_2 = new QVBoxLayout(widget1);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        usernameLineEdit = new QLineEdit(widget1);
        usernameLineEdit->setObjectName("usernameLineEdit");

        verticalLayout_2->addWidget(usernameLineEdit);

        passwordLineEdit = new QLineEdit(widget1);
        passwordLineEdit->setObjectName("passwordLineEdit");

        verticalLayout_2->addWidget(passwordLineEdit);

        widget2 = new QWidget(LoginDialog);
        widget2->setObjectName("widget2");
        widget2->setGeometry(QRect(150, 200, 211, 54));
        verticalLayout_3 = new QVBoxLayout(widget2);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        registerButton = new QPushButton(widget2);
        registerButton->setObjectName("registerButton");

        verticalLayout_3->addWidget(registerButton);

        loginButton = new QPushButton(widget2);
        loginButton->setObjectName("loginButton");

        verticalLayout_3->addWidget(loginButton);

        usernameLineEdit->raise();
        passwordLineEdit->raise();
        passwordLineEdit->raise();
        label_2->raise();
        loginButton->raise();
        label->raise();
        registerButton->raise();

        retranslateUi(LoginDialog);

        QMetaObject::connectSlotsByName(LoginDialog);
    } // setupUi

    void retranslateUi(QDialog *LoginDialog)
    {
        LoginDialog->setWindowTitle(QCoreApplication::translate("LoginDialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("LoginDialog", "<html><head/><body><p><span style=\" font-size:14pt;\">\347\224\250\346\210\267\345\220\215</span></p></body></html>", nullptr));
        label_2->setText(QCoreApplication::translate("LoginDialog", "<html><head/><body><p><span style=\" font-size:14pt;\">\345\257\206\347\240\201</span></p></body></html>", nullptr));
        usernameLineEdit->setText(QString());
        registerButton->setText(QCoreApplication::translate("LoginDialog", "\346\263\250\345\206\214", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginDialog", "\347\231\273\345\275\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginDialog: public Ui_LoginDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
