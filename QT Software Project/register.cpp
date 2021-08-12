#include "register.h"
#include "ui_register.h"
#include <QMessageBox>
#include <mainwindow.h>
Register::Register(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);

}

Register::~Register()
{
    delete ui;
}

void Register::on_pushButton_clicked()
{
    QString project_name = ui->lineEdit_project_name->text();
    QString project_description = ui->plainTextEdit_project_description->toPlainText();
    QString IC_name = ui->lineEdit_IC_name->text();
    QString package_name = ui->lineEdit_package_name->text();
    QString pins = ui->lineEdit_pins->text();
    int pin_count = pins.toInt();

//    for (int i =0; i<IC_name.length();i++){
//        if (IC_name[i] == " "){
//            QMessageBox::warning(this,"Error","No spaces allowed in IC name field! ");
//            return;
//        }
//    }
    // MainWindow v;

    if (!db.connopen()){
        qDebug() << "Database is not open at register.cpp";
        return;
    }
//    db.connopen();
    QSqlQuery query1;
    query1.prepare("insert into ICregistry (ProjectName, Description, ICName,PackageName,PinCount) "
                   "values('"+project_name+"','"+project_description+"', '"+IC_name+"','"+package_name+"','"+pins+"')");

    if(query1.exec()){
        emit signal_emit_item(project_name);
        qDebug()<<"signal emmitted ";

        QMessageBox::information(this,"Save","Entry saved!");

//        db.connclose(); //dont forget to close an opened connection once the work is done.
    }
    else{
        QMessageBox::warning(this,"Error - ReEnter data correctly ",query1.lastError().text());
    }
    this->close();

}