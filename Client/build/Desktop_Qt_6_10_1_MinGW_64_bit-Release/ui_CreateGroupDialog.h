/********************************************************************************
** Form generated from reading UI file 'CreateGroupDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATEGROUPDIALOG_H
#define UI_CREATEGROUPDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CreateGroupDialog
{
public:
    QDialogButtonBox *buttonBox;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *groupNameLineEdit;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *label_2;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QListWidget *friendsListWidget;

    void setupUi(QDialog *CreateGroupDialog)
    {
        if (CreateGroupDialog->objectName().isEmpty())
            CreateGroupDialog->setObjectName("CreateGroupDialog");
        CreateGroupDialog->resize(372, 461);
        buttonBox = new QDialogButtonBox(CreateGroupDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setGeometry(QRect(110, 410, 151, 32));
        buttonBox->setOrientation(Qt::Orientation::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok);
        horizontalLayoutWidget = new QWidget(CreateGroupDialog);
        horizontalLayoutWidget->setObjectName("horizontalLayoutWidget");
        horizontalLayoutWidget->setGeometry(QRect(20, 20, 331, 71));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName("label");

        horizontalLayout->addWidget(label);

        groupNameLineEdit = new QLineEdit(horizontalLayoutWidget);
        groupNameLineEdit->setObjectName("groupNameLineEdit");

        horizontalLayout->addWidget(groupNameLineEdit);

        verticalLayoutWidget = new QWidget(CreateGroupDialog);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(20, 110, 81, 41));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        verticalLayoutWidget_2 = new QWidget(CreateGroupDialog);
        verticalLayoutWidget_2->setObjectName("verticalLayoutWidget_2");
        verticalLayoutWidget_2->setGeometry(QRect(109, 100, 231, 291));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        friendsListWidget = new QListWidget(verticalLayoutWidget_2);
        friendsListWidget->setObjectName("friendsListWidget");
        friendsListWidget->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);

        verticalLayout_2->addWidget(friendsListWidget);


        retranslateUi(CreateGroupDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, CreateGroupDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, CreateGroupDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(CreateGroupDialog);
    } // setupUi

    void retranslateUi(QDialog *CreateGroupDialog)
    {
        CreateGroupDialog->setWindowTitle(QCoreApplication::translate("CreateGroupDialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("CreateGroupDialog", "<html><head/><body><p><span style=\" font-size:14pt;\">\347\276\244\350\201\212\345\220\215\347\247\260</span></p></body></html>", nullptr));
        label_2->setText(QCoreApplication::translate("CreateGroupDialog", "<html><head/><body><p><span style=\" font-size:14pt;\">\345\245\275\345\217\213\345\210\227\350\241\250</span></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CreateGroupDialog: public Ui_CreateGroupDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATEGROUPDIALOG_H
