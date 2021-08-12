#include "databaseHandler.h"
#include <QMessageBox>

bool databaseHandler::connopen()
{
    mydb = QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName("ICtester.db");
    // A new database named "ICtester" is created in this specified folder.
    if (!mydb.open()){
        qDebug()<<( "Error opening the DB 1"); //use qdebug instead of labels because labels are not used in header files
        return false; // because return type is bool.
    }else{
        qDebug()<<("DB open");
        return true;
    }
}


void databaseHandler::connclose(){
    mydb.close();
    mydb.removeDatabase(QSqlDatabase::defaultConnection); //remove if ant default connection is there.
}


int databaseHandler::query_execution(QString query_string ,QString error_topic )
{
    QSqlQuery query;
    query.prepare(query_string);
    if(query.exec())
    {
//        qDebug()<<"query executed successfully11";
    }
    else
    {
      qDebug()<< error_topic << query.lastError().text() ;
      return 0;
    }
    return 1;
}

int databaseHandler::drop_tables(QString table_name)
{
    query_execution("DROP TABLE IF EXISTS '"+table_name+"'   " , "Error droping the table");

    QSqlQuery query;
    bool prepRet = query.prepare("select tbl_name from 'sqlite_master' where type = 'table' and tbl_name like '"+table_name+"/%' ");
    if (!prepRet) {
         return 0;
    }
    if (!query.exec()) {
         qDebug() <<"Query didnt execute"<< query.lastError().text();
        return 0;
    }
    QStringList drop_table_list;
    while (query.next()) {
        drop_table_list << query.value(0).toString();
    }
    qDebug()<<"drop table list created: "<<drop_table_list;
    for (int i =0; i<drop_table_list.length();i++){
        query_execution("DROP TABLE IF EXISTS '"+drop_table_list[i]+"'   " , "Error droping the table");
    }
    return 1;
}

//#######################################################################################################
