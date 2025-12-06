#ifndef SETNICKNAME_H
#define SETNICKNAME_H

#include <QWidget>

namespace Ui {
class SetNickname;
}

class SetNickname : public QWidget
{
    Q_OBJECT

public:
    explicit SetNickname(QWidget *parent = nullptr);
    ~SetNickname();

private:
    Ui::SetNickname *ui;
};

#endif // SETNICKNAME_H
