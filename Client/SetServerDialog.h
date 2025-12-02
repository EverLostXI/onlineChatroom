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
    static SetServerDialog& instance();

    explicit SetServerDialog(QWidget *parent = nullptr);
    ~SetServerDialog();

signals:
    void serverEndPoint(QString serverAddress,quint16 serverPort);

private slots:
    void on_confirmButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::SetServerDialog *ui;
};
#endif // SETSERVERDIALOG_H
