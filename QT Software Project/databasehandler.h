#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include<QListWidget>
#include<QIcon>
#include<QMessageBox>
#include <QtSql>

class databaseHandler
{
  public:
    QSqlDatabase mydb;
    bool connopen();
    void connclose();
    int query_execution(QString query_string ,QString error_topic );
    int drop_tables(QString);
};


#endif // DATABASEHANDLER_H
