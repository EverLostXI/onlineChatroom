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
#include <QtWidgets/QLabel>
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
    QWidget *verticalLayoutWidget_6;
    QVBoxLayout *verticalLayout_7;
    QLabel *welcomeLabel;
    QWidget *verticalLayoutWidget_7;
    QVBoxLayout *verticalLayout_6;
    QPushButton *setNicknameButton;

    void setupUi(QWidget *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(662, 461);
        verticalLayoutWidget = new QWidget(MainWindow);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(10, 90, 171, 291));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        conversationListWidget = new QListWidget(verticalLayoutWidget);
        conversationListWidget->setObjectName("conversationListWidget");

        verticalLayout->addWidget(conversationListWidget);

        verticalLayoutWidget_2 = new QWidget(MainWindow);
        verticalLayoutWidget_2->setObjectName("verticalLayoutWidget_2");
        verticalLayoutWidget_2->setGeometry(QRect(10, 390, 171, 51));
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
        verticalLayoutWidget_3->setGeometry(QRect(199, 340, 451, 81));
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

        verticalLayoutWidget_6 = new QWidget(MainWindow);
        verticalLayoutWidget_6->setObjectName("verticalLayoutWidget_6");
        verticalLayoutWidget_6->setGeometry(QRect(19, 19, 161, 41));
        verticalLayout_7 = new QVBoxLayout(verticalLayoutWidget_6);
        verticalLayout_7->setObjectName("verticalLayout_7");
        verticalLayout_7->setContentsMargins(0, 0, 0, 0);
        welcomeLabel = new QLabel(verticalLayoutWidget_6);
        welcomeLabel->setObjectName("welcomeLabel");
        QFont font;
        font.setPointSize(16);
        welcomeLabel->setFont(font);

        verticalLayout_7->addWidget(welcomeLabel);

        verticalLayoutWidget_7 = new QWidget(MainWindow);
        verticalLayoutWidget_7->setObjectName("verticalLayoutWidget_7");
        verticalLayoutWidget_7->setGeometry(QRect(119, 59, 82, 31));
        verticalLayout_6 = new QVBoxLayout(verticalLayoutWidget_7);
        verticalLayout_6->setObjectName("verticalLayout_6");
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        setNicknameButton = new QPushButton(verticalLayoutWidget_7);
        setNicknameButton->setObjectName("setNicknameButton");
        setNicknameButton->setEnabled(true);
        QFont font1;
        font1.setPointSize(8);
        setNicknameButton->setFont(font1);

        verticalLayout_6->addWidget(setNicknameButton);


        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QWidget *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Form", nullptr));
        addFriendButton->setText(QCoreApplication::translate("MainWindow", "\346\267\273\345\212\240\345\245\275\345\217\213", nullptr));
        createGroupButton->setText(QCoreApplication::translate("MainWindow", "\345\210\233\345\273\272\347\276\244\350\201\212", nullptr));
        sendButton->setText(QCoreApplication::translate("MainWindow", "\345\217\221\351\200\201", nullptr));
        welcomeLabel->setText(QCoreApplication::translate("MainWindow", "\346\254\242\350\277\216\357\274\201\347\224\250\346\210\2670", nullptr));
        setNicknameButton->setText(QCoreApplication::translate("MainWindow", "\346\233\264\346\224\271\346\230\265\347\247\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
