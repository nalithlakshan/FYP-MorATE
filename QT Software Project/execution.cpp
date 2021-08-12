#include "execution.h"
#include <QFileDialog>


void MainWindow::on_pushButton_pg3_start_clicked()
{
    ui->textEdit_pg3_report_preview->clear();
    QString designator_name = ui->lineEdit_pg3_designator_name->text();
    qDebug()<<"designator nameee: "<< designator_name;
    int ID_of_1st_iteration = ui->lineEdit_pg3_ID_first_iteration->text().toInt();
    int no_of_iterations = ui->lineEdit_pg3_iterations->text().toInt();
    qDebug()<<"no of iterationss : "<< no_of_iterations;


    QString directory_path = ui->lineEdit_pg3_fill_path->text();
    QDir new_dir(directory_path);
    QDateTime my_time_stamp(QDateTime::currentDateTime());
    report_folder_name = designator_name + "_" + my_time_stamp.toString("yyyyMMddhhmmss");
    new_dir.mkdir(report_folder_name);

    QString operator_name = ui->lineEdit_pg3_operator_name->text();

    int taskflow_row_count = ui->tableWidget->rowCount();
    QVector<int> tableWidget_state(taskflow_row_count);
    QVector<QString> tableWidget_taskname(taskflow_row_count);

    for (int i = 0; i < taskflow_row_count; i++) {
        tableWidget_state[i]   = ui->tableWidget->item(i,0)->checkState();
        tableWidget_taskname[i]= ui->tableWidget->item(i,1)->text();
    }
    qDebug()<<"before emmision";
    qDebug()<<"main thread: "<<QThread::currentThread();

    QVariant tableWidget_state_vr;
    tableWidget_state_vr.setValue(tableWidget_state);

    QVariant tableWidget_taskname_vr;
    tableWidget_taskname_vr.setValue(tableWidget_taskname);


    emit sendStart(
                designator_name,
                ID_of_1st_iteration,
                no_of_iterations,
                directory_path,
                report_folder_name,
                operator_name,
                current_project,
                taskflow_row_count,
                tableWidget_state_vr,
                tableWidget_taskname_vr
                );
    qDebug()<<"after emmission";


}



void MainWindow::on_pushButton_pg3_stop_clicked()
{
//    emit sendStop();
//    usb->thread()->quit();
//    usb->thread()->wait(1000);
//    usb->thread()->terminate();
}



void MainWindow::on_pushButton_pg3_browse_clicked()
{
    QString file_path = QFileDialog::getExistingDirectory(this, "Select folder to save report files");
    qDebug()<< "file path:" << file_path;
    ui->lineEdit_pg3_fill_path->setText(file_path);
}




////////////////////////////////// USB Inferface ////////////////////////////////
void MainWindow::usbErrorHandler(QString message){
    QMessageBox::warning(this,"Error!",message);
}

void MainWindow::usbWarningHandler(QString message){
    QMessageBox::warning(this,"Warning!",message);
}

void MainWindow::usbInfoHandler(QString message){
    QMessageBox::warning(this,"Info",message);
}

void MainWindow::receive_current_iteration(QString iteration){
    ui->label_pg3_current_iteration->setText(iteration);
    qDebug()<<"recieved iteration";
}

void MainWindow::receive_status_icon_update_row(int i, int task_pass_flag){
    QIcon pass_icon (":/prefix1/img/pass.png");
    QIcon fail_icon (":/prefix1/img/fail.png");
    QIcon loading_icon (":/prefix1/img/loading3.png");

    QTableWidgetItem* icon_item = new QTableWidgetItem;
    if(task_pass_flag == 1){
        icon_item->setIcon(pass_icon);
        icon_item->setTextAlignment(Qt::AlignHCenter);
        ui->tableWidget->setItem(i,2, icon_item);
//        ui->scrollArea_pg3_tasks->setStyleSheet("background-image:url(\":/prefix1/img/passed_background.png\"); ");

    }
    else if(task_pass_flag == 0){
        icon_item->setIcon(fail_icon);
        icon_item->setTextAlignment(Qt::AlignHCenter);
        ui->tableWidget->setItem(i,2, icon_item);
        ui->scrollArea_pg3_tasks->setStyleSheet("background-image:url(\":/prefix1/img/failed_background.png\"); ");

    }
    else{
        icon_item->setIcon(loading_icon);
        icon_item->setTextAlignment(Qt::AlignHCenter);
        ui->tableWidget->setItem(i,2, icon_item);
        ui->scrollArea_pg3_tasks->setStyleSheet("background-image:url(\":/prefix1/img/passed_background.png\"); ");

    }
}

void MainWindow::receive_summary_activation_signal(QStringList failed_ICs,QStringList passed_ICs,QStringList all_ICs){
//    ui->frame_pg3_summary->setEnabled(1);
    ui->listWidget_pg3->clear();
    ui->label_pg3_failed_ICs->setText(QString::number(failed_ICs.length()));
    ui->label_pg3_passed_ICs->setText(QString::number(passed_ICs.length()));
    ui->label_pg3_total_ICs->setText(QString::number(all_ICs.length()));

    for (int i=0;i<failed_ICs.length() ;i++ ) {
        ui->listWidget_pg3->addItem(failed_ICs[i]);
    }
    failed_ICs_mw = failed_ICs;
    passed_ICs_mw = passed_ICs;
    all_ICs_mw = all_ICs;

}


void MainWindow::on_comboBox_pg3_currentIndexChanged(const QString &arg1)
{
    ui->listWidget_pg3->clear();
    ui->textEdit_pg3_report_preview->clear();
    if(arg1 == "Failed ICs"){
        for (int i=0;i<failed_ICs_mw.length() ;i++ ) {
            ui->listWidget_pg3->addItem(failed_ICs_mw[i]);
        }
    }
    else if(arg1 == "Passed ICs"){
        for (int i=0;i<passed_ICs_mw.length() ;i++ ) {
            ui->listWidget_pg3->addItem(passed_ICs_mw[i]);
        }
    }
    else if(arg1 == "All ICs"){
        for (int i=0;i<all_ICs_mw.length() ;i++ ) {
            ui->listWidget_pg3->addItem(all_ICs_mw[i]);
        }
    }

}


void MainWindow::on_listWidget_pg3_itemClicked(QListWidgetItem *item)
{
    QString directory_path = ui->lineEdit_pg3_fill_path->text();
    QString designator_name = ui->lineEdit_pg3_designator_name->text();
    QString file_path = directory_path+"/"+report_folder_name+"/"+item->text()+".txt";
    readfile(file_path);
}


void MainWindow::readfile(QString filename){
    QFile file(filename);
    if(!file.exists()){
        qDebug() << "NO existe el archivo "<<filename;
    }else{
        qDebug() << filename<<" encontrado...";
    }
    QString line;
    ui->textEdit_pg3_report_preview->clear();
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream stream(&file);
        while (!stream.atEnd()){
            line = stream.readLine();
            ui->textEdit_pg3_report_preview->setText(ui->textEdit_pg3_report_preview->toPlainText()+line+"\n");
            qDebug() << "linea: "<<line;
        }
    }
    file.close();
}


void MainWindow::print_to_review(QString text)
{
    ui->textEdit_pg3_report_preview->append(text);

}
