/********************************************************************************
** Form generated from reading UI file 'AddFriendDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDFRIENDDIALOG_H
#define UI_ADDFRIENDDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_AddFriendDialog
{
public:
    QDialogButtonBox *buttonBox;
    QLabel *label;
    QLineEdit *friendIdLineEdit;

    void setupUi(QDialog *AddFriendDialog)
    {
        if (AddFriendDialog->objectName().isEmpty())
            AddFriendDialog->setObjectName("AddFriendDialog");
        AddFriendDialog->resize(191, 269);
        buttonBox = new QDialogButtonBox(AddFriendDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setGeometry(QRect(20, 210, 151, 32));
        buttonBox->setOrientation(Qt::Orientation::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok);
        label = new QLabel(AddFriendDialog);
        label->setObjectName("label");
        label->setGeometry(QRect(60, 50, 71, 41));
        friendIdLineEdit = new QLineEdit(AddFriendDialog);
        friendIdLineEdit->setObjectName("friendIdLineEdit");
        friendIdLineEdit->setGeometry(QRect(30, 100, 131, 21));

        retranslateUi(AddFriendDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, AddFriendDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(AddFriendDialog);
    } // setupUi

    void retranslateUi(QDialog *AddFriendDialog)
    {
        AddFriendDialog->setWindowTitle(QCoreApplication::translate("AddFriendDialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("AddFriendDialog", "<html><head/><body><p align=\"center\"><span style=\" font-size:14pt;\">\345\245\275\345\217\213ID</span></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AddFriendDialog: public Ui_AddFriendDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDFRIENDDIALOG_H
