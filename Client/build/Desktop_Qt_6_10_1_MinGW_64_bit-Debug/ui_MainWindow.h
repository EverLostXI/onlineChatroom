/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QListWidget *conversationListWidget;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QPushButton *addFriendButton;
    QPushButton *createGroupButton;
    QWidget *verticalLayoutWidget_3;
    QVBoxLayout *verticalLayout_3;
    QTextEdit *messageInputTextEdit;
    QWidget *verticalLayoutWidget_4;
    QVBoxLayout *verticalLayout_4;
    QListWidget *chatHistoryListWidget;
    QWidget *verticalLayoutWidget_5;
    QVBoxLayout *verticalLayout_5;
    QPushButton *sendButton;

    void setupUi(QWidget *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(662, 461);
        verticalLayoutWidget = new QWidget(MainWindow);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(10, 10, 171, 361));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        conversationListWidget = new QListWidget(verticalLayoutWidget);
        conversationListWidget->setObjectName("conversationListWidget");

        verticalLayout->addWidget(conversationListWidget);

        verticalLayoutWidget_2 = new QWidget(MainWindow);
        verticalLayoutWidget_2->setObjectName("verticalLayoutWidget_2");
        verticalLayoutWidget_2->setGeometry(QRect(10, 380, 171, 61));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        addFriendButton = new QPushButton(verticalLayoutWidget_2);
        addFriendButton->setObjectName("addFriendButton");

        verticalLayout_2->addWidget(addFriendButton);

        createGroupButton = new QPushButton(verticalLayoutWidget_2);
        createGroupButton->setObjectName("createGroupButton");

        verticalLayout_2->addWidget(createGroupButton);

        verticalLayoutWidget_3 = new QWidget(MainWindow);
        verticalLayoutWidget_3->setObjectName("verticalLayoutWidget_3");
        verticalLayoutWidget_3->setGeometry(QRect(199, 340, 451, 73));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget_3);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        messageInputTextEdit = new QTextEdit(verticalLayoutWidget_3);
        messageInputTextEdit->setObjectName("messageInputTextEdit");

        verticalLayout_3->addWidget(messageInputTextEdit);

        verticalLayoutWidget_4 = new QWidget(MainWindow);
        verticalLayoutWidget_4->setObjectName("verticalLayoutWidget_4");
        verticalLayoutWidget_4->setGeometry(QRect(199, 9, 451, 321));
        verticalLayout_4 = new QVBoxLayout(verticalLayoutWidget_4);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        chatHistoryListWidget = new QListWidget(verticalLayoutWidget_4);
        chatHistoryListWidget->setObjectName("chatHistoryListWidget");

        verticalLayout_4->addWidget(chatHistoryListWidget);

        verticalLayoutWidget_5 = new QWidget(MainWindow);
        verticalLayoutWidget_5->setObjectName("verticalLayoutWidget_5");
        verticalLayoutWidget_5->setGeometry(QRect(560, 420, 91, 41));
        verticalLayout_5 = new QVBoxLayout(verticalLayoutWidget_5);
        verticalLayout_5->setObjectName("verticalLayout_5");
        verticalLayout_5->setContentsMargins(0, 0, 0, 0);
        sendButton = new QPushButton(verticalLayoutWidget_5);
        sendButton->setObjectName("sendButton");

        verticalLayout_5->addWidget(sendButton);


        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QWidget *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Form", nullptr));
        addFriendButton->setText(QCoreApplication::translate("MainWindow", "\346\267\273\345\212\240\345\245\275\345\217\213", nullptr));
        createGroupButton->setText(QCoreApplication::translate("MainWindow", "\345\210\233\345\273\272\347\276\244\350\201\212", nullptr));
        sendButton->setText(QCoreApplication::translate("MainWindow", "\345\217\221\351\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
