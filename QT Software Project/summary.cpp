#include "summary.h"
#include "ui_summary.h"

Summary::Summary(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Summary)
{
    ui->setupUi(this);
}

Summary::~Summary()
{
    delete ui;
}
