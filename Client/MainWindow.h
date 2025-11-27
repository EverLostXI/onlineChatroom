// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "addfrienddialog.h"    // 包含头文件
#include "creategroupdialog.h"  // 包含头文件

// ...
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sendButton_clicked();
    void on_conversationListWidget_itemClicked(QListWidgetItem *item);

    // 新增的槽函数声明
    void on_addFriendButton_clicked();
    void on_createGroupButton_clicked();

private:
    Ui::MainWindow *ui;
    // ...
    void updateConversationList(); // <-- 声明一个刷新界面的函数

    QMap<int, QString> m_friends; // <-- 用来存储好友 (ID -> 名字)
    QMap<int, QString> m_groups;  // <-- 用来存储群聊 (ID -> 名字)
};
#endif // MAINWINDOW_H
