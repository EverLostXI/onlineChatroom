/********************************************************************************
** Form generated from reading UI file 'SetNickname.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETNICKNAME_H
#define UI_SETNICKNAME_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SetNickname
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLineEdit *nickname;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *confirmButton;
    QPushButton *cancelButton;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QLabel *label;

    void setupUi(QWidget *SetNickname)
    {
        if (SetNickname->objectName().isEmpty())
            SetNickname->setObjectName("SetNickname");
        SetNickname->resize(400, 300);
        verticalLayoutWidget = new QWidget(SetNickname);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(190, 50, 171, 131));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        nickname = new QLineEdit(verticalLayoutWidget);
        nickname->setObjectName("nickname");

        verticalLayout->addWidget(nickname);

        horizontalLayoutWidget = new QWidget(SetNickname);
        horizontalLayoutWidget->setObjectName("horizontalLayoutWidget");
        horizontalLayoutWidget->setGeometry(QRect(70, 210, 271, 61));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        confirmButton = new QPushButton(horizontalLayoutWidget);
        confirmButton->setObjectName("confirmButton");

        horizontalLayout->addWidget(confirmButton);

        cancelButton = new QPushButton(horizontalLayoutWidget);
        cancelButton->setObjectName("cancelButton");

        horizontalLayout->addWidget(cancelButton);

        verticalLayoutWidget_2 = new QWidget(SetNickname);
        verticalLayoutWidget_2->setObjectName("verticalLayoutWidget_2");
        verticalLayoutWidget_2->setGeometry(QRect(69, 49, 91, 131));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(verticalLayoutWidget_2);
        label->setObjectName("label");
        QFont font;
        font.setPointSize(16);
        label->setFont(font);

        verticalLayout_2->addWidget(label);


        retranslateUi(SetNickname);

        QMetaObject::connectSlotsByName(SetNickname);
    } // setupUi

    void retranslateUi(QWidget *SetNickname)
    {
        SetNickname->setWindowTitle(QCoreApplication::translate("SetNickname", "Form", nullptr));
        nickname->setText(QCoreApplication::translate("SetNickname", "\347\224\250\346\210\2670", nullptr));
        confirmButton->setText(QCoreApplication::translate("SetNickname", "\347\241\256\350\256\244", nullptr));
        cancelButton->setText(QCoreApplication::translate("SetNickname", "\345\217\226\346\266\210", nullptr));
        label->setText(QCoreApplication::translate("SetNickname", "\350\256\276\347\275\256\346\230\265\347\247\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SetNickname: public Ui_SetNickname {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETNICKNAME_H
