#ifndef SUMMARY_H
#define SUMMARY_H

#include <QDialog>
#include <mainwindow.h>
#include <databaseHandler.h>

namespace Ui {
class Summary;
}

class Summary : public QDialog
{
    Q_OBJECT

public:
    databaseHandler db;

    explicit Summary(QWidget *parent = nullptr);
    ~Summary();


private:
    Ui::Summary *ui;
};

#endif // SUMMARY_H