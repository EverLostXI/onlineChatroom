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
    QPushButton *confirmButton;
    QPushButton *cancelButton;
    QWidget *verticalLayoutWidget_3;
    QVBoxLayout *verticalLayout_3;
    QLineEdit *serverAddress;
    QLineEdit *serverPort;

    void setupUi(QWidget *SetServerDialog)
    {
        if (SetServerDialog->objectName().isEmpty())
            SetServerDialog->setObjectName("SetServerDialog");
        SetServerDialog->resize(400, 300);
        verticalLayoutWidget = new QWidget(SetServerDialog);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(69, 50, 91, 81));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(verticalLayoutWidget);
        label->setObjectName("label");
        label->setMinimumSize(QSize(69, 40));
        label->setScaledContents(false);
        label->setIndent(-6);

        verticalLayout->addWidget(label);

        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        verticalLayoutWidget_2 = new QWidget(SetServerDialog);
        verticalLayoutWidget_2->setObjectName("verticalLayoutWidget_2");
        verticalLayoutWidget_2->setGeometry(QRect(200, 190, 160, 80));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        confirmButton = new QPushButton(verticalLayoutWidget_2);
        confirmButton->setObjectName("confirmButton");

        verticalLayout_2->addWidget(confirmButton);

        cancelButton = new QPushButton(verticalLayoutWidget_2);
        cancelButton->setObjectName("cancelButton");

        verticalLayout_2->addWidget(cancelButton);

        verticalLayoutWidget_3 = new QWidget(SetServerDialog);
        verticalLayoutWidget_3->setObjectName("verticalLayoutWidget_3");
        verticalLayoutWidget_3->setGeometry(QRect(169, 49, 171, 91));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget_3);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        serverAddress = new QLineEdit(verticalLayoutWidget_3);
        serverAddress->setObjectName("serverAddress");

        verticalLayout_3->addWidget(serverAddress);

        serverPort = new QLineEdit(verticalLayoutWidget_3);
        serverPort->setObjectName("serverPort");

        verticalLayout_3->addWidget(serverPort);


        retranslateUi(SetServerDialog);

        QMetaObject::connectSlotsByName(SetServerDialog);
    } // setupUi

    void retranslateUi(QWidget *SetServerDialog)
    {
        SetServerDialog->setWindowTitle(QCoreApplication::translate("SetServerDialog", "Form", nullptr));
        label->setText(QCoreApplication::translate("SetServerDialog", "Server Address", nullptr));
        label_2->setText(QCoreApplication::translate("SetServerDialog", "Server port", nullptr));
        confirmButton->setText(QCoreApplication::translate("SetServerDialog", "Confirm", nullptr));
        cancelButton->setText(QCoreApplication::translate("SetServerDialog", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SetServerDialog: public Ui_SetServerDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETSERVERDIALOG_H
