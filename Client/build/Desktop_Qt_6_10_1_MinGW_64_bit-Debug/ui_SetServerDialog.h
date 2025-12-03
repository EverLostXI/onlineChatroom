/********************************************************************************
** Form generated from reading UI file 'SetServerDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETSERVERDIALOG_H
#define UI_SETSERVERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SetServerDialog
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLabel *label_2;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *serverAddress;
    QLineEdit *serverPort;
    QWidget *verticalLayoutWidget_3;
    QVBoxLayout *verticalLayout_3;
    QPushButton *confirmButton;
    QPushButton *cancelButton;

    void setupUi(QWidget *SetServerDialog)
    {
        if (SetServerDialog->objectName().isEmpty())
            SetServerDialog->setObjectName("SetServerDialog");
        SetServerDialog->resize(400, 272);
        verticalLayoutWidget = new QWidget(SetServerDialog);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(69, 50, 101, 80));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(verticalLayoutWidget);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        verticalLayoutWidget_2 = new QWidget(SetServerDialog);
        verticalLayoutWidget_2->setObjectName("verticalLayoutWidget_2");
        verticalLayoutWidget_2->setGeometry(QRect(200, 50, 160, 80));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        serverAddress = new QLineEdit(verticalLayoutWidget_2);
        serverAddress->setObjectName("serverAddress");

        verticalLayout_2->addWidget(serverAddress);

        serverPort = new QLineEdit(verticalLayoutWidget_2);
        serverPort->setObjectName("serverPort");

        verticalLayout_2->addWidget(serverPort);

        verticalLayoutWidget_3 = new QWidget(SetServerDialog);
        verticalLayoutWidget_3->setObjectName("verticalLayoutWidget_3");
        verticalLayoutWidget_3->setGeometry(QRect(140, 170, 131, 61));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget_3);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        confirmButton = new QPushButton(verticalLayoutWidget_3);
        confirmButton->setObjectName("confirmButton");

        verticalLayout_3->addWidget(confirmButton);

        cancelButton = new QPushButton(verticalLayoutWidget_3);
        cancelButton->setObjectName("cancelButton");

        verticalLayout_3->addWidget(cancelButton);


        retranslateUi(SetServerDialog);

        QMetaObject::connectSlotsByName(SetServerDialog);
    } // setupUi

    void retranslateUi(QWidget *SetServerDialog)
    {
        SetServerDialog->setWindowTitle(QCoreApplication::translate("SetServerDialog", "Form", nullptr));
        label->setText(QCoreApplication::translate("SetServerDialog", "Server Address", nullptr));
        label_2->setText(QCoreApplication::translate("SetServerDialog", "Server port", nullptr));
        serverAddress->setText(QCoreApplication::translate("SetServerDialog", "10.30.110.243", nullptr));
        serverPort->setText(QCoreApplication::translate("SetServerDialog", "8888", nullptr));
        confirmButton->setText(QCoreApplication::translate("SetServerDialog", "Confirm", nullptr));
        cancelButton->setText(QCoreApplication::translate("SetServerDialog", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SetServerDialog: public Ui_SetServerDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETSERVERDIALOG_H
