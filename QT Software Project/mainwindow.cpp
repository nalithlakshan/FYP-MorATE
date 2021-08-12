#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QListWidget>
#include<QIcon>
#include<QMessageBox>
#include <register.h>
#include <summary.h>
#include <string>
#include <QCloseEvent>
#include <QMovie>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include<QDateTime>
#include <QFileDialog>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    db.connopen();
    ui->setupUi(this);
    setWindowTitle("MorATE IC Tester");

    if (ui->stackedWidget->currentIndex()==0){
        ui->pushButton_back->hide();
    }
    ui->label_address_bar->setText("");

    if( db.query_execution
            (
                "CREATE TABLE if not exists ICregistry"
                   "("
                        "ID INTEGER PRIMARY KEY AUTOINCREMENT ,"
                        "ProjectName VARCHAR(25) UNIQUE CHECK (length(ProjectName)>0),"
                        "Description TEXT ,"
                        "ICName VARCHAR(25) ,"
                        "PackageName VARCHAR(25), "
                        "PinCount INTEGER "
                   ")"
                   ,   "Error Creating Table"
            )
       )
    {
        Load_table_listwidget("select ProjectName from 'ICregistry' ", ui->listWidget , ":/prefix1/img/IC4.png");
    }
    else
    {
      QMessageBox::information(this,"Error","Error creating the main table.");
    }
    for (int j=0 ; j< 2; j++)
    {
        for (int i = 1; i < 92; i=i+6) {
            QString radio_button_name_template = tab_list[j] + "_radio_button_" + QString::number(i);
            QRadioButton *radiobutton = ui->centralwidget->findChild<QRadioButton *>(radio_button_name_template);
            connect(radiobutton, SIGNAL(toggled(bool)), this, SLOT(vs1_FB_validator()));
        }

        for (int i = 2; i < 93; i=i+6) {
            QString radio_button_name_template =tab_list[j] + "_radio_button_" + QString::number(i);
            QRadioButton *radiobutton = ui->centralwidget->findChild<QRadioButton *>(radio_button_name_template);
            connect(radiobutton, SIGNAL(toggled(bool)), this, SLOT(vs2_FB_validator()));
        }

        for (int i = 3; i < 94; i=i+6) {
            QString radio_button_name_template =tab_list[j] + "_radio_button_" + QString::number(i);
            QRadioButton *radiobutton = ui->centralwidget->findChild<QRadioButton *>(radio_button_name_template);
            connect(radiobutton, SIGNAL(toggled(bool)), this, SLOT(vs3_FB_validator()));
        }
    }

    qDebug()<<"mainwindow thread: "<<QThread::currentThread();

        //##################################### BEGIN USB Inferface Connections ####################################
    qRegisterMetaType<QVariant>("QVariant");

    usb = new usb_interface();
    usb->moveToThread(new QThread(this));

        connect(usb, SIGNAL(sendError(QString)),this, SLOT(usbErrorHandler(QString)));
        connect(usb, SIGNAL(sendWarning(QString)),this, SLOT(usbWarningHandler(QString)));
        connect(usb, SIGNAL(sendInfo(QString)),this, SLOT(usbInfoHandler(QString)));
        connect(usb, SIGNAL(send_current_iteration(QString)), this, SLOT(receive_current_iteration(QString)));
        connect(usb, SIGNAL(send_status_icon_update_row(int,int)), this, SLOT(receive_status_icon_update_row(int,int)));
        connect(usb, SIGNAL(send_summary_activation_signal(QStringList,QStringList,QStringList)), this, SLOT(receive_summary_activation_signal(QStringList,QStringList,QStringList)));

        connect(usb,SIGNAL(send_print_to_review_text(QString)),this, SLOT(print_to_review(QString)));

        connect(this,
                SIGNAL(sendStart(
                    QString ,
                    int ,
                    int ,
                    QString ,
                    QString,
                    QString ,
                    QString,
                    int,
                    QVariant,
                    QVariant
                    )) ,
                usb,
                SLOT(pg3_start_clicked(
                         QString ,
                         int ,
                         int ,
                         QString ,
                         QString,
                         QString ,
                         QString,
                         int,
                         QVariant,
                         QVariant
                 )),
                Qt::QueuedConnection
                );

        connect(this,SIGNAL(sendStop()),usb,SLOT(pg3_stop_clicked()));

        usb->thread()->start();

        //##################################### END USB Interface Connections #####################################



}


MainWindow::~MainWindow()
{
    usb->thread()->terminate();
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::Load_table_listwidget( QString query_string , QListWidget* list_widget, QString icon_path)
{
//    db.query_execution("BEGIN TRANSACTION", "transaction was not begun-loadtble");
    if(!db.connopen())
    {
      QMessageBox::warning(this, "DB Error", "Not connected to the DB!");
    }
    QSqlQuery query;
    bool prepRet = query.prepare(query_string );
    if (!prepRet) {
         return;
    }
    if (!query.exec()) {
         qDebug() <<"Query didnt execute"<< query.lastError().text();
        return;
    }
    QStringList list;
    while (query.next()) {
        list << query.value(0).toString();
    }
    qDebug()<<"IC list created: "<<list;

    for (int i =0; i<list.length();i++){
        add_item_to_listwidget(list[i] , list_widget , icon_path);
    }
}


void MainWindow::add_item_to_listwidget(QString entry , QListWidget* list_widget , QString icon_path)
{
    QListWidgetItem* item = new QListWidgetItem(QIcon(icon_path),entry);
    item->setSizeHint(QSize(0,40));
    item->setFont(QFont("Helvetica", 14));
    list_widget->addItem(item);
}


void MainWindow::slot_add_item(QString project_name)
{
    add_item_to_listwidget(project_name, ui->listWidget , ":/prefix1/img/IC4.png");
}

void MainWindow::delete_handler(QListWidget *list_widget,QString table_name, QString column_name, QString drop_table_name)
{
    db.query_execution("BEGIN TRANSACTION", "transaction was not begun");
    if(list_widget->selectedItems().size()!=0)
    {
        QListWidgetItem* item = list_widget->currentItem();
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,"Delete Item","Are you sure you want to delete item '"+item->text()+"' ?",QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes){
            if (db.query_execution("DELETE FROM '"+table_name+"' WHERE "+column_name+" = '"+item->text()+"'  " , "Error deleting the item" ))
            {
                if (db.drop_tables(drop_table_name))
                {
                    if (item->text()== current_task)
                    {
                        ui->frame2_pg2->setDisabled(1);
                        ui->label_address_bar->setText(current_project);
                        //call a function to load an empty canvas
                    }
                    list_widget->removeItemWidget(item);
                    delete item;
                    QMessageBox::information(this,"Deleted","Selected Entry deleted!");
                }
            }
        }
    }
    else
        QMessageBox::warning(this, "Warning", "Item not selected !");
    db.query_execution("COMMIT", "not commited 3");
    db.query_execution(" VACUUM " , "Error vacuuming the table ");
}


void MainWindow::add_item_handler(QString entry)
{
    db.query_execution("BEGIN TRANSACTION", "transaction was not begun");
    if (db.query_execution("insert into '"+current_project+"' (test_seq_name, duplicate_number) values('"+entry+"' , 0 )",
                    "Error - ReEnter data correctly"))
    {
        add_item_to_listwidget(entry, ui->listWidget_pg2_tests , ":/prefix1/img/img_gear2.png");
        ui->lineEdit_pg2_add_test->clear();
        qDebug()<<"test inserted to the test table";

//        QStringList tab_list={"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P"};
        //create table for tabs A to P
        for (int i =0; i<tab_list.length();i++)
        {
            QString table_name_for_tab = current_project +"/" + entry +"/"+ tab_list[i];
            qDebug()<<"tab db name: " + table_name_for_tab ;
            if (db.query_execution(
                        "CREATE TABLE if not exists '"+table_name_for_tab+"' "
                            "("
                                "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                                "row_name TEXT UNIQUE,"
                                "radio_button_column INTEGER,"
                                " measure_voltage INTEGER,"
                                "measure_current INTEGER,"
                                " min_voltage REAL,"
                                " max_voltage REAL,"
                                " min_current REAL, "
                                " max_current REAL,"
                                " V_A_start_timer_dynamic_load_value REAL,"
                                " FB INTEGER,"
                                " sink_source INTEGER"
                            ")"
                        ,"Error creating tab table"))
            {
                qDebug()<<"tab table created";
                QStringList row_list = {"A1","A2","A3","A4","A5","A6","A7","A8","A9","A10","A11","A12","A13",
                                        "A14","A15","A16",
                                        "voltage_source_1_V","voltage_source_2_V","voltage_source_3_V",
                                        "voltage_source_1_A","voltage_source_2_A","voltage_source_3_A",
                                        "voltage_source_1_start_timer","voltage_source_2_start_timer","voltage_source_3_start_timer",
                                        "voltage_source_1_FB","voltage_source_2_FB","voltage_source_3_FB",
                                        "I_OL", "I_OH", "sink", "source","current_range"};

                for (int j=0; j<row_list.length(); j++)
                {
                    db.query_execution("insert into '"+table_name_for_tab+"' (row_name) values('"+row_list[j]+"' )",
                                       "Error entering the 32 rows to tab tables");
                }
            }
        }

        //create table for tab Digital IO
        QString table_name_for_tab_digital_IO = current_project +"/" + entry +"/Digital IO";
        qDebug()<<"table_name_for_tab_digital_IO: " + table_name_for_tab_digital_IO ;
        if (db.query_execution(
                    "CREATE TABLE if not exists '"+table_name_for_tab_digital_IO+"' "
                        "("
                            "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "digital_testing_enabled INTEGER,"
                            "code TEXT"
                        ")"
                    ,"Error creating tab table"))
        {
            qDebug()<<"table for the tab, digital IO created";
            db.query_execution("insert into '"+table_name_for_tab_digital_IO+"' (code) values( '""' )",
                                   "Error entering the row to tab table digital IO");

        }



    }
    else
    {
        QMessageBox::warning(this,"Error - ReEnter data correctly ","Enter a unique, not null name");
    }
    db.query_execution("COMMIT", "transaction not commited 4");
}






void MainWindow::duplicate_item_handler(QString entry)
{
    db.query_execution("BEGIN TRANSACTION", "transaction was not begun");
    if (db.query_execution("insert into '"+current_project+"' (test_seq_name, duplicate_number) values('"+entry+"' , 0 )",
                    "Error - ReEnter data correctly"))
    {
        add_item_to_listwidget(entry, ui->listWidget_pg2_tests , ":/prefix1/img/img_gear2.png");
        ui->lineEdit_pg2_add_test->clear();
        qDebug()<<"test inserted to the test table";

//        QStringList tab_list={"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P"};
        for (int i =0; i<tab_list.length();i++)
        {
            QString table_name_for_tab = current_project +"/" + entry +"/"+ tab_list[i];
            qDebug()<<"tab db name: " + table_name_for_tab ;
            db.query_execution(
                        "CREATE TABLE if not exists '"+table_name_for_tab+"' "
                            "("
                                "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                                "row_name TEXT UNIQUE,"
                                "radio_button_column INTEGER,"
                                " measure_voltage INTEGER,"
                                "measure_current INTEGER,"
                                " min_voltage REAL,"
                                " max_voltage REAL,"
                                " min_current REAL, "
                                " max_current REAL,"
                                " V_A_start_timer_dynamic_load_value REAL,"
                                " FB INTEGER,"
                                " sink_source INTEGER"
                            ")"
                        ,"Error creating tab table");


            QString first_table = current_project +"/" +ui->listWidget_pg2_tests->currentItem()->text() +"/"+ tab_list[i];

            db.query_execution("insert into '"+table_name_for_tab+"' select* from  '"+first_table+"'  ","table copy unsuccessfull");
        }

        //duplicate the tab. digital IO
        QString table_name_for_tab_digital_IO = current_project +"/" + entry +"/Digital IO";
        if (db.query_execution(
                    "CREATE TABLE if not exists '"+table_name_for_tab_digital_IO+"' "
                        "("
                            "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "digital_testing_enabled INTEGER,"
                            "code TEXT"
                        ")"
                    ,"Error creating tab table"))
        {
            QString first_table = current_project +"/" +ui->listWidget_pg2_tests->currentItem()->text() +"/Digital IO";
            db.query_execution("insert into '"+table_name_for_tab_digital_IO+"' select* from  '"+first_table+"'  ","table copy unsuccessfull");

        }

    }
    else
    {
        QMessageBox::warning(this,"Error - ReEnter data correctly ","Enter a unique, not null name");
    }
    db.query_execution("COMMIT", "transaction not commited 4");


}


void MainWindow::update_task_order_number()
{
    for(int i = 0; i < ui->listWidget_pg2_tests->count(); ++i)
    {
        if (db.query_execution("update '"+current_project+"' set order_number= '"+QString::number(i)+"' where test_seq_name ='"+ui->listWidget_pg2_tests->item(i)->text()+"' ",
                               "Error - ReEnter data correctly1"))
        {
            qDebug()<<"Order number updated: " << i;
        }
    }
}


void MainWindow::load_line_edit_V_A_start_timer_dynamic_load_value_columns_data_from_DB()
{
    QString table_name = current_project +"/"+ current_task +"/"+ current_tab_text;

    QSqlQuery query_le;
    bool prepRet_le = query_le.prepare("select V_A_start_timer_dynamic_load_value, FB, sink_source from '"+table_name+"' " );
    if (!prepRet_le) {
         qDebug() << "prepRet error"<<query_le.lastError().text();
         return;
    }
    if (!query_le.exec()) {
         qDebug() <<"Query didnt execute"<< query_le.lastError().text();
        return;
    }
     QStringList list1 ;
     QStringList list2 ;
     QList<int> list3 ;

     while (query_le.next()) {
        list1<< query_le.value(0).toString();
        list2<< query_le.value(1).toString();
        list3<< query_le.value(2).toInt();
    }


    for (int i =16; i<19;i++)
    {
        QString line_edit_name_template =  current_tab_text + "_line_edit_VS" + QString::number(i-15)+ "_V" ;
        QLineEdit *load_line_edit = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template);
        load_line_edit->setText(list1[i]);
    }
    for (int i =19; i<22;i++)
    {
        QString line_edit_name_template =  current_tab_text + "_line_edit_VS" + QString::number(i-18)+ "_A" ;
        QLineEdit *load_line_edit = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template);
        load_line_edit->setText(list1[i]);
    }
    for (int i =22; i<25;i++)
    {
        QString line_edit_name_template =  current_tab_text + "_line_edit_VS" + QString::number(i-21)+ "_start_timer" ;
        QLineEdit *load_line_edit = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template);
        load_line_edit->setText(list1[i]);
    }
    //A_line_edit_I_OL
    QString line_edit_name_template_OL =  current_tab_text + "_line_edit_I_OL" ;
    QLineEdit *line_edit_OL = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template_OL);
    line_edit_OL->setText(list1[28]);


    //A_line_edit_I_OH
    QString line_edit_name_template_OH =  current_tab_text + "_line_edit_I_OH" ;
    QLineEdit *line_edit_OH = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template_OH);
    line_edit_OH->setText(list1[29]);

    //A_radio_button_sink
    QString sink_radio_button_name_template =  current_tab_text + "_radio_button_sink" ;
    QRadioButton *sink_radio_button = ui->tabWidget->findChild<QRadioButton *>(sink_radio_button_name_template);
    sink_radio_button->setChecked(list3[30]);

    //A_radio_button_source
    QString source_radio_button_name_template =  current_tab_text + "_radio_button_source" ;
    QRadioButton *source_radio_button = ui->tabWidget->findChild<QRadioButton *>(source_radio_button_name_template);
    source_radio_button->setChecked(list3[31]);

    for (int i =25; i<28;i++)
    {
        QString combo_box_name_template =  current_tab_text + "_comboBox_" + QString::number(i-24) ;
        QComboBox *combo_box_FB = ui->tabWidget->findChild<QComboBox *>(combo_box_name_template);
        combo_box_FB->setCurrentText(list2[i]);
        qDebug()<<"initial FB items for a new task:"<<list2[i];
    }
    //ammeter range combobox
    QString combo_box_name_template =  current_tab_text + "_comboBox_4";
    QComboBox *combo_box_FB = ui->tabWidget->findChild<QComboBox *>(combo_box_name_template);
    qDebug()<<"initial ammeter range for a new task:"<<list2[32];
    if(list2[32] ==NULL){
        combo_box_FB->setCurrentText("A");
    }
    else{
        combo_box_FB->setCurrentText(list2[32]);
    }
}


void MainWindow::load_canvas_line_edit_columns_data_from_DB(QString DB_column_name, int line_edit_column_number)
{
    QString table_name = current_project +"/"+ current_task +"/"+ current_tab_text;

    QSqlQuery query_le;
    bool prepRet_le = query_le.prepare("select "+DB_column_name+" from '"+table_name+"' " );
    if (!prepRet_le) {
         qDebug() << "prepRet error"<<query_le.lastError().text();
         return;
    }
    if (!query_le.exec()) {
         qDebug() <<"Query didnt execute"<< query_le.lastError().text();
        return;
    }
     QStringList list ;

     while (query_le.next()) {
        list<< query_le.value(0).toString();
    }
    for (int i =0; i<16;i++)
    {
        QString line_edit_name_template =  current_tab_text + "_line_edit_" + QString::number(line_edit_column_number+ (i*4)) ;
        QLineEdit *load_line_edit = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template);
        load_line_edit->setText(list[i]);
    }
}


void MainWindow::load_canvas_checkbox_columns_data_from_DB(QString DB_column_name, int check_box_column_number)
{
    QString table_name = current_project +"/"+ current_task +"/"+ current_tab_text;

    QSqlQuery query_cb;
    bool prepRet_cb = query_cb.prepare("select "+DB_column_name+" from '"+table_name+"' " );
    if (!prepRet_cb) {
         qDebug() << "prepRet error"<<query_cb.lastError().text();
         return;
    }
    if (!query_cb.exec()) {
         qDebug() <<"Query didnt execute"<< query_cb.lastError().text();
        return;
    }
     QStringList list ;

     while (query_cb.next()) {
        list<< query_cb.value(0).toString();
    }
    for (int i =0; i<16;i++)
    {
        QString check_box_name_template =  current_tab_text + "_check_box_" + QString::number(check_box_column_number+ (i*2)) ;
        QCheckBox *load_check_box = ui->tabWidget->findChild<QCheckBox *>(check_box_name_template);
        load_check_box->setChecked(list[i].toInt());
    }
}


void MainWindow::load_current_tab_data_from_db()
{
    if(current_tab_text != "Digital IO")
    {
        QString table_name = current_project +"/"+ current_task +"/"+ current_tab_text;

        db.query_execution("BEGIN TRANSACTION", "transaction not begun. ");

        //loading radio buttons data from DB to canvas
        QSqlQuery query;
        bool prepRet = query.prepare("select radio_button_column from '"+table_name+"' " );
        if (!prepRet) {
             qDebug() << "prepRet error"<<query.lastError().text();
             return;
        }
        if (!query.exec()) {
             qDebug() <<"Query didnt execute"<< query.lastError().text();
            return;
        }
        QStringList rb_list;
        while (query.next()) {
            rb_list << query.value(0).toString();
        }

        for (int i =0; i<16;i++)
        {
            QString radio_button_name_template;
            if (rb_list[i]!=NULL)
            {
                int y =rb_list[i].toInt();
                radio_button_name_template =  current_tab_text + "_radio_button_" + QString::number(y+ (i*6)) ;
            }
            else
            {
                radio_button_name_template =  current_tab_text + "_radio_button_" + QString::number((i+1)*6) ;
            }
            QRadioButton *load_radio_button = ui->tabWidget->findChild<QRadioButton *>(radio_button_name_template);
            load_radio_button->setChecked(1);
        }


        //loading line edit data from DB to the canvas
        load_canvas_line_edit_columns_data_from_DB("min_voltage",1);
        load_canvas_line_edit_columns_data_from_DB("max_voltage",2);
        load_canvas_line_edit_columns_data_from_DB("min_current",3);
        load_canvas_line_edit_columns_data_from_DB("max_current",4);


        //loading check box data from DB to the canvas
        load_canvas_checkbox_columns_data_from_DB("measure_voltage",1);
        load_canvas_checkbox_columns_data_from_DB("measure_current",2);



        //loading heading data from DB to canvas
        load_line_edit_V_A_start_timer_dynamic_load_value_columns_data_from_DB();



        db.query_execution("COMMIT", "not commited 2");

    }
    else{
        QString table_name = current_project +"/"+ current_task +"/"+ current_tab_text;

        db.query_execution("BEGIN TRANSACTION", "transaction not begun. ");

        QSqlQuery query;
        bool prepRet = query.prepare("select code,digital_testing_enabled  from '"+table_name+"' " );
        if (!prepRet) {
             qDebug() << "prepRet error"<<query.lastError().text();
             return;
        }
        if (!query.exec()) {
             qDebug() <<"Query didnt execute"<< query.lastError().text();
            return;
        }
        if (query.next()) {
            ui->plainTextEdit_pg2_digital_testing_code->setPlainText(query.value(0).toString());
            ui->checkBox_pg2_digital_testing_enabled->setChecked(query.value(1).toInt());
        }
        db.query_execution("COMMIT", "not commited 3");
    }
}


void MainWindow::update_current_tab_data()
{
    if(current_task != "" && current_tab_text!="Digital IO"){

        db.query_execution("BEGIN TRANSACTION", "transaction was not begun-tabdata");

        QString table_name = current_project +"/"+ current_task +"/"+ current_tab_text;

        //saving checkbox data from canvas to DB (measure V, measure I)
        for (int k = 1 ; k< 33; k++)
        {
            QString check_box_name_template = current_tab_text + "_check_box_"+ QString::number(k) ;
            QCheckBox *cb = ui->tabWidget->findChild<QCheckBox *>(check_box_name_template);

            if (k%2==1)
            {
                db.query_execution("update '"+table_name+"' set measure_voltage= "+QString::number(cb->isChecked())+" where ID = "+QString::number((k/2)+1)+" ",
                                   "Couldnt update task");
            }

            if (k%2==0)
            {
                db.query_execution("update '"+table_name+"' set measure_current= "+QString::number(cb->isChecked())+" where ID = "+QString::number(k/2)+" ",
                                   "Couldnt update task");
            }
        }


        ////////////////////////////////
        //saving line edit data from canvas to DB

        for (int i = 1 ; i< 65 ;i++)
        {
            QString line_edit_name_template = current_tab_text + "_line_edit_"+ QString::number(i) ;
            QLineEdit *le = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template);

            if (i%4==1)
            {
                db.query_execution("update '"+table_name+"' set min_voltage= '"+le->text()+"' where ID = "+QString::number((i/4)+1)+" ",
                                   "Couldnt update task");
            }
            if (i%4==2)
            {
                db.query_execution("update '"+table_name+"' set max_voltage= '"+le->text()+"' where ID = "+QString::number((i/4)+1)+" ",
                                   "Couldnt update task");
            }
            if (i%4==3)
            {
                db.query_execution("update '"+table_name+"' set min_current= '"+le->text()+"' where ID = "+QString::number((i/4)+1)+" ",
                                   "Couldnt update task");
            }
            if (i%4==0)
            {
                db.query_execution("update '"+table_name+"' set max_current= '"+le->text()+"' where ID = "+QString::number(i/4)+" ",
                                   "Couldnt update task");
            }
        }


        //saving radio button data from canvas to DB
        for (int i = 1 ; i< 97 ;i++)
        {
            QString radio_button_name_template =  current_tab_text + "_radio_button_" + QString::number(i) ;
            QRadioButton *rb = ui->tabWidget->findChild<QRadioButton *>(radio_button_name_template);


            if (rb->isChecked()==1)
            {
                int find_column;
                int find_row;
                if ((i%6)==0)
                {
                    find_column= 6;
                    find_row = i/6;
                }
                else
                {
                    find_column = i%6;
                    find_row = (i/6) +1;
                }

                db.query_execution("update '"+table_name+"' set radio_button_column = '"+QString::number(find_column)+"' where ID = "+QString::number(find_row)+" ",
                                   "Couldnt update task");
            }
        }


        //saving V,A,start timer, FB data to DB
        for (int p=1; p<4; p++)
        {
            QString line_edit_name_template_V = current_tab_text+"_line_edit_VS"+ QString::number(p)+"_V";
            QString line_edit_name_template_A = current_tab_text+"_line_edit_VS"+ QString::number(p)+"_A";
            QString line_edit_name_template_start_timer = current_tab_text+"_line_edit_VS"+ QString::number(p)+"_start_timer";
            QString combobox_name_template = current_tab_text + "_comboBox_" + QString::number(p);

            QLineEdit *line_edit_V = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template_V);
            QLineEdit *line_edit_A = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template_A);
            QLineEdit *line_edit_start_timer = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template_start_timer);
            QComboBox *combo_box = ui->tabWidget->findChild<QComboBox *>(combobox_name_template);

            db.query_execution("update '"+table_name+"' set V_A_start_timer_dynamic_load_value = '"+line_edit_V->text()+"' where ID = "+QString::number(16+p)+" ",
                               "Couldnt update task");
            db.query_execution("update '"+table_name+"' set V_A_start_timer_dynamic_load_value = '"+line_edit_A->text()+"' where ID = "+QString::number(19+p)+" ",
                               "Couldnt update task");
            db.query_execution("update '"+table_name+"' set V_A_start_timer_dynamic_load_value = '"+line_edit_start_timer->text()+"' where ID = "+QString::number(22+p)+" ",
                               "Couldnt update start timer value");
            db.query_execution("update '"+table_name+"' set FB = '"+combo_box->currentText()+"' where ID = "+QString::number(25+p)+" ",
                               "Couldnt update combo box FB_pin_name");

        }

        //update current measure range combo box
        QString combobox_name_template = current_tab_text + "_comboBox_4";
        QComboBox *combo_box = ui->tabWidget->findChild<QComboBox *>(combobox_name_template);
        db.query_execution("update '"+table_name+"' set FB = '"+combo_box->currentText()+"' where ID = "+QString::number(33)+" ",
                           "Couldnt update combo box FB_pin_name");



        // saving I_OL, I_OH data to DB.

        QString line_edit_name_template_I_OL = current_tab_text+"_line_edit_I_OL";
        QString line_edit_name_template_I_OH = current_tab_text+"_line_edit_I_OH";
        QLineEdit *line_edit_I_OL = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template_I_OL);
        QLineEdit *line_edit_I_OH = ui->tabWidget->findChild<QLineEdit *>(line_edit_name_template_I_OH);
        db.query_execution("update '"+table_name+"' set V_A_start_timer_dynamic_load_value = '"+line_edit_I_OL->text()+"' where ID = "+QString::number(29)+" ",
                           "Couldnt update task");
        db.query_execution("update '"+table_name+"' set V_A_start_timer_dynamic_load_value = '"+line_edit_I_OH->text()+"' where ID = "+QString::number(30)+" ",
                           "Couldnt update task");


        // saving sink source radiobuttons data to DB.

        QString radiobutton_name_template_sink = current_tab_text + "_radio_button_sink";
        QString radiobutton_name_template_source = current_tab_text + "_radio_button_source";
        QRadioButton *radiobutton_sink = ui->tabWidget->findChild<QRadioButton *>(radiobutton_name_template_sink);
        QRadioButton *radiobutton_source = ui->tabWidget->findChild<QRadioButton *>(radiobutton_name_template_source);
        db.query_execution("update '"+table_name+"' set sink_source = '"+QString::number(radiobutton_sink->isChecked())+"' where ID = "+QString::number(31)+" ",
                           "Couldnt update task");
        db.query_execution("update '"+table_name+"' set sink_source = '"+QString::number(radiobutton_source->isChecked())+"' where ID = "+QString::number(32)+" ",
                           "Couldnt update task");


        db.query_execution("COMMIT", "commit unsuccessful");
    }
    else if(current_task != ""){
        db.query_execution("BEGIN TRANSACTION", "transaction was not begun-tabdata");
        QString table_name = current_project +"/"+ current_task +"/"+ current_tab_text;

        QPlainTextEdit *pte = ui->tabWidget->findChild<QPlainTextEdit *>("plainTextEdit_pg2_digital_testing_code");
        QCheckBox *cb = ui->tabWidget->findChild<QCheckBox *>("checkBox_pg2_digital_testing_enabled");

        db.query_execution("update '"+table_name+"' set code = '"+ pte->toPlainText()+"' where ID = "+QString::number(1)+" ", "Couldnt update task digital Io");
        db.query_execution("update '"+table_name+"' set digital_testing_enabled = '"+QString::number(ui->checkBox_pg2_digital_testing_enabled->isChecked())+"' where ID = "+QString::number(1)+" ", "Couldnt update task digital Io");

        //digital_testing_enabled
        db.query_execution("COMMIT", "commit unsuccessful");
    }
    else{
        qDebug()<<"Task not selected. Therefore nothing to update in tabs.";
    }
}


void MainWindow::update_combobox_details(QComboBox *combo_box , int ID_number, int index, int column)
{
    QString table_name = current_project +"/"+ current_task +"/"+ current_tab_text;
    QString selected_combobox_item = combo_box->itemText(index);
    if (index!=0)
    {
        QString radiobutton_name = current_tab_text + "_radio_button_" +  QString::number(((index-1)*6) + column) ;
        qDebug()<<"rb name: "<< radiobutton_name;

        QRadioButton *rb = ui->tabWidget->findChild<QRadioButton *>(radiobutton_name);
        if ((rb->isChecked()==0) )
        {
            QMessageBox::warning(this, "pin not enabled", "Please select the radiobutton before selecting from the dropdown! ");
            combo_box->setCurrentIndex(0);
            qDebug()<<"A combobox details not updated";
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::on_pushButton_img_clicked()
{
    Register registerWindow;
    registerWindow.setModal(true);
    QObject::connect(&registerWindow, SIGNAL(signal_emit_item(QString)), this, SLOT(slot_add_item(QString)), Qt::UniqueConnection);
    registerWindow.exec();
}


void MainWindow::on_pushButton_delete_clicked()
{
    delete_handler(ui->listWidget, "ICregistry", "ProjectName" , ui->listWidget->currentItem()->text());
}

void MainWindow::on_pushButton_back_clicked()
{
    if (ui->stackedWidget->currentIndex()==1)
    {
        int current = ui->stackedWidget->currentIndex();
        ui->stackedWidget->setCurrentIndex(current-1);
        ui->pushButton_back->hide();
        ui->label_address_bar->setText("");


        update_task_order_number();
        if (ui->listWidget_pg2_tests->selectedItems().length()!=0)
        {
            update_current_tab_data();
        }
        current_task="";

    }
    if (ui->stackedWidget->currentIndex()==2)
    {
        int current = ui->stackedWidget->currentIndex();
        ui->stackedWidget->setCurrentIndex(current-1);

        if(current_task!=""){
            ui->label_address_bar->setText(current_project+"/"+current_task+"/"+current_tab_text);
        }
        else{
            ui->label_address_bar->setText(current_project);
        }
        db.connopen();
    }
}

void MainWindow::on_pushButton_open_project_clicked()
{

    if(ui->listWidget->selectedItems().size()!=0)
    {
        current_project = ui->listWidget->currentItem()->text();
        int current_page_index = ui->stackedWidget->currentIndex();
        qDebug()<<"current index : "<<current_page_index;
        ui->stackedWidget->setCurrentIndex(current_page_index+1);
        ui->listWidget_pg2_tests->clear();
        ui->pushButton_back->show();

        ui->frame2_pg2->setDisabled(1);
        //call a function to load an empty canvas

        if (db.query_execution("CREATE TABLE if not exists '"+current_project+"' (ID INTEGER PRIMARY KEY AUTOINCREMENT,test_seq_name VARCHAR(30) UNIQUE CHECK (length(test_seq_name)>0) , duplicate_number INTEGER, order_number INTEGER )","Error creating project table"))
        {
            qDebug()<< "created table for the selected IC";

            Load_table_listwidget("select test_seq_name from '"+current_project+"' order by order_number ",ui->listWidget_pg2_tests, ":/prefix1/img/img_gear2.png");
            ui->label_address_bar->setText(current_project);
            ui->listWidget_pg2_tests->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(ui->listWidget_pg2_tests, SIGNAL(customContextMenuRequested(const QPoint &)),
                    this, SLOT(ProvideContextMenu(const QPoint &)) , Qt::UniqueConnection);
        }
    }
    else
        QMessageBox::warning(this, "Warning", "Item not selected !");
}

void MainWindow::ProvideContextMenu(const QPoint &pos)
{
    QPoint item = ui->listWidget_pg2_tests->mapToGlobal(pos);
    QMenu submenu;
    submenu.addAction("Duplicate");
    submenu.addAction("Delete");
    submenu.addAction("Rename");
    QAction* rightClickItem = submenu.exec(item);
    if (rightClickItem && rightClickItem->text().contains("Delete") )
    {
        delete_handler(ui->listWidget_pg2_tests, current_project, "test_seq_name", current_project+"/"+ ui->listWidget_pg2_tests->currentItem()->text());
    }
    else if (rightClickItem && rightClickItem->text().contains("Duplicate") )
    {
        QSqlQuery query;
        query.prepare("select duplicate_number from '"+current_project+"' where test_seq_name= '"+ui->listWidget_pg2_tests->currentItem()->text()+"' ");
        if(query.exec())
        {
            int i;
            while (query.next()){
                QString a = query.value(0).toString();
                qDebug()<< a;
                i = a.toInt();
                i+=1;
                qDebug()<< i;
            }

            QString display_text_item = ui->listWidget_pg2_tests->currentItem()->text() + "("+ QString::number(i)+ ")";
            qDebug()<< display_text_item;
            if (db.query_execution("update '"+current_project+"' set duplicate_number= '"+QString::number(i)+"' where test_seq_name ='"+ui->listWidget_pg2_tests->currentItem()->text()+"' ",
                            "Error - ReEnter data correctly1"))
            {
                duplicate_item_handler(display_text_item );
            }
        }
    }


    else if (rightClickItem && rightClickItem->text().contains("Rename") )
    {

        QListWidgetItem *item = ui->listWidget_pg2_tests->currentItem();

        item->setFlags(item->flags() | Qt::ItemIsEditable);

        ui->listWidget_pg2_tests->editItem(item);

        task_name_before_renaming = ui->listWidget_pg2_tests->currentItem()->text();
        connect(ui->listWidget_pg2_tests->itemDelegate(), SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SLOT(ListWidgetEditEnd(QWidget*, QAbstractItemDelegate::EndEditHint )) , Qt::UniqueConnection);

    }
}

void MainWindow::ListWidgetEditEnd(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    db.query_execution("BEGIN TRANSACTION", "transaction was not begun");
    QString NewValue = reinterpret_cast<QLineEdit*>(editor)->text();

    if (db.query_execution("update '"+current_project+"' set test_seq_name= '"+NewValue+"', duplicate_number = 0 where test_seq_name ='"+task_name_before_renaming+"' ",
                    "Couldnt update task") )
    {
        if (task_name_before_renaming == current_task)
        {
            current_task= NewValue;
        }
        for (int i =0; i<tab_list.length();i++)
        {
            QString new_table_name= current_project+"/"+NewValue+"/"+tab_list[i];
            QString old_table_name= current_project+"/"+task_name_before_renaming+"/"+tab_list[i];
            db.query_execution("alter table '"+old_table_name+"' rename to '"+new_table_name+"'  " , "Error renaming child tables");
        }
        QMessageBox::information(this,"Updated Successfully ","Task name updated");

    }
    else
    {
        ui->listWidget_pg2_tests->currentItem()->setText(task_name_before_renaming);
        QMessageBox::warning(this,"Update error ","Task name should be unique and not null");
    }
    db.query_execution("COMMIT", "not commited 1");
}

void MainWindow::on_pushButton_pg2_add_test_clicked()
{
    QString new_test = ui->lineEdit_pg2_add_test->text();
    
    add_item_handler(new_test);
}




void MainWindow::on_listWidget_pg2_tests_indexesMoved(const QModelIndexList &indexes)
{
    qDebug()<<"on_listWidget_pg2_tests_indexesMoved!";
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "close app??",
                                                                tr("Are you sure?\n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();


    } else {
        update_task_order_number();
        update_current_tab_data();
        event->accept();
        db.connclose();
    }
}

void MainWindow::on_listWidget_pg2_tests_itemDoubleClicked(QListWidgetItem *item)
{


    if (ui->frame2_pg2->isEnabled())
    {
        update_current_tab_data();
        QList<QListWidgetItem *>items = ui->listWidget_pg2_tests->findItems(current_task, Qt::MatchExactly);
        items[0]->setBackground(QColor(0,0,0,0));
    }

    current_task = item->text();  
    ui->frame2_pg2->setEnabled(1);


    item->setBackground(QColor(102,178,255));
    int index = ui->tabWidget->currentIndex();
    current_tab_text = ui->tabWidget->tabText(index);
    load_current_tab_data_from_db();

    QString address_bar_name = current_project + "/"+ current_task +"/"+current_tab_text;
    ui->label_address_bar->setText(address_bar_name);

}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    QString address_bar_name = current_project+"/"+ current_task +"/"+ui->tabWidget->tabText(index);
    ui->label_address_bar->setText(address_bar_name);
    update_current_tab_data();

    current_tab_text= ui->tabWidget->tabText(index);
    load_current_tab_data_from_db();


}

void MainWindow::on_A_comboBox_1_activated(int index)
{
    update_combobox_details(ui->A_comboBox_1, 23, index , 1);
}

void MainWindow::on_A_comboBox_2_activated(int index)
{
    update_combobox_details(ui->A_comboBox_2, 24, index , 2);
}

void MainWindow::on_A_comboBox_3_activated(int index)
{
    update_combobox_details(ui->A_comboBox_3, 25, index , 3);
}

void MainWindow::on_B_comboBox_1_activated(int index)
{
    update_combobox_details(ui->B_comboBox_1, 23, index, 1);
}

void MainWindow::on_B_comboBox_2_activated(int index)
{
    update_combobox_details(ui->B_comboBox_2, 24, index, 2);
}

void MainWindow::on_B_comboBox_3_activated(int index)
{
    update_combobox_details(ui->B_comboBox_3, 25, index, 3);
}

void MainWindow::vs1_FB_validator()
{
    QRadioButton *rb = (QRadioButton *)sender();

    QString aa = rb->objectName();
    QString bb = aa.mid(15,2);

    int row_number = ((bb.toInt()-1)/6)+1;

    QString combobox_name_template = current_tab_text + "_comboBox_1" ;
    QComboBox *combo_box = ui->tabWidget->findChild<QComboBox *>(combobox_name_template);


    if ((combo_box->currentIndex()==row_number) && !(rb->isChecked()))
    {
        combo_box->setCurrentIndex(0);
    }
}


void MainWindow::vs2_FB_validator()
{
    QRadioButton *rb = (QRadioButton *)sender();

    QString aa = rb->objectName();
    QString bb = aa.mid(15,2);

    int row_number = ((bb.toInt()-2)/6)+1;


    QString combobox_name_template = current_tab_text + "_comboBox_2" ;
    QComboBox *combo_box = ui->tabWidget->findChild<QComboBox *>(combobox_name_template);

    if ((combo_box->currentIndex()==row_number) && !(rb->isChecked()))
    {
        combo_box->setCurrentIndex(0);
    }
}


void MainWindow::vs3_FB_validator()
{
    QRadioButton *rb = (QRadioButton *)sender();

    QString aa = rb->objectName();
    QString bb = aa.mid(15,2);

    int row_number = ((bb.toInt()-3)/6)+1;

    QString combobox_name_template = current_tab_text + "_comboBox_3" ;
    QComboBox *combo_box = ui->tabWidget->findChild<QComboBox *>(combobox_name_template);

    if ((combo_box->currentIndex()==row_number) && !(rb->isChecked()))
    {
        combo_box->setCurrentIndex(0);
    }
}

void MainWindow::on_pushButton_execute_clicked()
{
    update_task_order_number();
    update_current_tab_data();
    int current_page_index = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(current_page_index+1);
    ui->label_address_bar->setText("Test Execution");
    ui->label_pg3_project->setText("Project : "+ current_project);

    ui->tableWidget->setRowCount(0);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->horizontalHeader()->resizeSection(0,30);
    ui->tableWidget->horizontalHeader()->resizeSection(1,270);
    ui->tableWidget->horizontalHeader()->resizeSection(2,70);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "" << "TASK FLOW" << "STATUS");

    //clear and reset all previous data in edit fields and background colors
    ui->lineEdit_pg3_operator_name->clear();
    ui->lineEdit_pg3_designator_name->clear();
    ui->lineEdit_pg3_iterations->clear();
    ui->label_pg3_current_iteration->clear();
    ui->label_pg3_failed_ICs->clear();
    ui->label_pg3_passed_ICs->clear();
    ui->label_pg3_total_ICs->clear();

    ui->lineEdit_pg3_operator_name->setText("User");
    ui->lineEdit_pg3_designator_name->setText("IC");
    ui->lineEdit_pg3_iterations->setText(QString::number(1));

    for (int i= ui->listWidget_pg2_tests->count() -1;i>-1; i--)
    {
        add_row_to_table_task_flow(ui->listWidget_pg2_tests->item(i)->text());
    }
}

void MainWindow::add_row_to_table_task_flow(QString task_name)
{
    ui->tableWidget->insertRow(0);

    QTableWidgetItem *item2 = new QTableWidgetItem();
    item2->setCheckState(Qt::Checked);
    ui->tableWidget->setItem(0, 0, item2);

    ui->tableWidget->setItem(0,1,new QTableWidgetItem(task_name));
    QIcon loading_icon (":/prefix1/img/loading3.png");
    QTableWidgetItem* icon_item = new QTableWidgetItem;
    icon_item->setIcon(loading_icon);
    icon_item->setTextAlignment(Qt::AlignHCenter);
    ui->tableWidget->setItem(0,2, icon_item);

}
