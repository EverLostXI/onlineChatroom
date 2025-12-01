#ifndef SETSERVERDIALOG_H
#define SETSERVERDIALOG_H

#include <QDialog>

namespace Ui {
class SetServerDialog;
}

class SetServerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetServerDialog(QWidget *parent = nullptr);
    ~SetServerDialog();

private slots:
    void on_confirmButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::SetServerDialog *ui;
};
#endif // SETSERVERDIALOG_H
