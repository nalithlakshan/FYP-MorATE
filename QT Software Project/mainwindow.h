#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include<QDebug>
#include<QFileInfo>
#include <QListWidgetItem>
#include <register.h>
#include <databaseHandler.h>
#include <QSerialPort>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include "usb_interface.h"      //Include USB interface object class here
#include <QVector>
#include <QMetaType>
#include <QVariant>
Q_DECLARE_METATYPE(QVector<int>);
Q_DECLARE_METATYPE(QVector<QString>);

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
//  Ui::MainWindow *ui;

  databaseHandler db; // QSqlDatabase handler object
  QString current_project;
  QString current_task;
  QString current_tab_text;

  int duplicate_task_number=0;
  QString task_name_before_renaming;
  QStringList tab_list={"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P"};

  QStringList failed_ICs_mw;
  QStringList passed_ICs_mw;
  QStringList all_ICs_mw;
  QString report_folder_name;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void slot_add_item(QString);

private slots:
    void Load_table_listwidget(QString, QListWidget* , QString);
    void add_item_to_listwidget(QString , QListWidget* , QString);
    void ProvideContextMenu(const QPoint &);

    void on_pushButton_delete_clicked();
    void on_pushButton_back_clicked();
    void on_pushButton_img_clicked();
    void on_pushButton_open_project_clicked();

    void on_pushButton_pg2_add_test_clicked();

    void ListWidgetEditEnd(QWidget*, QAbstractItemDelegate::EndEditHint);


    void on_listWidget_pg2_tests_indexesMoved(const QModelIndexList &indexes);

    void update_task_order_number();

    void closeEvent (QCloseEvent *event);

    void on_listWidget_pg2_tests_itemDoubleClicked(QListWidgetItem *item);
    void update_current_tab_data();
    void on_tabWidget_currentChanged(int index);

    void delete_handler(QListWidget*, QString, QString, QString);
    void add_item_handler( QString );
    void duplicate_item_handler(QString entry);

    void load_current_tab_data_from_db();

    void on_A_comboBox_1_activated(int index);
    void update_combobox_details(QComboBox* , int, int , int);
    void load_canvas_line_edit_columns_data_from_DB(QString, int);
    void load_canvas_checkbox_columns_data_from_DB(QString, int);
    void load_line_edit_V_A_start_timer_dynamic_load_value_columns_data_from_DB();


    void on_A_comboBox_2_activated(int index);
    void on_A_comboBox_3_activated(int index);
    void on_B_comboBox_1_activated(int index);
    void on_B_comboBox_2_activated(int index);
    void on_B_comboBox_3_activated(int index);

    void vs1_FB_validator();
    void vs2_FB_validator();
    void vs3_FB_validator();

    void on_pushButton_execute_clicked();
    void add_row_to_table_task_flow(QString );
    void on_pushButton_pg3_start_clicked();

    //FUNCTIONS IN EXECUTION.CPP
    void on_pushButton_pg3_browse_clicked();
    void readfile(QString);

    //USB
    void usbErrorHandler(QString);      //add this to show USB interface errors
    void usbWarningHandler(QString);    //add this to show USB interface warning
    void usbInfoHandler(QString);       //add this to show USB info messages
    void receive_current_iteration(QString);
    void receive_status_icon_update_row(int, int);
    void receive_summary_activation_signal(QStringList,QStringList,QStringList);
    void on_comboBox_pg3_currentIndexChanged(const QString &arg1);

    void on_listWidget_pg3_itemClicked(QListWidgetItem *item);

    void on_pushButton_pg3_stop_clicked();
    void print_to_review(QString);

signals:
    //Execution Window related signals
    void sendStart(
       QString designator_name,
       int ID_of_1st_iteration,
       int no_of_iterations,
       QString directory_path,
       QString folder_name,
       QString operator_name,
       QString current_project,
       int taskflow_row_count,
       QVariant tableWidget_state_vr,
       QVariant tableWidget_taskname_vr
       );
    void sendStop();


private:
    Ui::MainWindow *ui;
    usb_interface *usb;          //create private USB object pointer here

};

#endif // MAINWINDOW_H
