#ifndef SQL_DATABASE_P_H
#define SQL_DATABASE_P_H

#include "helper/sql_database.h"

class DatabaseHolder : public QObject
{
    Q_OBJECT
public:
    DatabaseHolder(SqlDatabase db);
    ~DatabaseHolder();
    SqlDatabase _database;
};

#endif // SQL_DATABASE_P_H

