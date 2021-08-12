#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>
#include <mainwindow.h>
#include <databaseHandler.h>

namespace Ui {
class Register;
}

class Register : public QDialog
{
    Q_OBJECT

public:
    databaseHandler db;// QSqlDatabase handler object

    explicit Register(QWidget *parent = nullptr);
    ~Register();

signals:
    void signal_emit_item(QString);

private slots:
    void on_pushButton_clicked();

private:
    Ui::Register *ui;
};

#endif // REGISTER_H
