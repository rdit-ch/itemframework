#include "helper/sql_database_p.h"
#include <QDebug>

DatabaseHolder::DatabaseHolder(SqlDatabase db) : _database(db)
{
    //qDebug() << "Creating DB holder to db with driver" << db.driver();
}

DatabaseHolder::~DatabaseHolder()
{
    //qDebug() << "Destroying DB holder to db with driver" << _database.driver();
}

//Custom deleter function for DatabaseHolder's
static void deleteQObject(DatabaseHolder* p)
{
    if (p != nullptr) {
        p->deleteLater(); //delete object when the eventloop regains control
    }
}


SqlDatabase::SqlDatabase()
{

}

SqlDatabase::SqlDatabase(const QString& type) : QSqlDatabase(type)
{

}


SqlQuery SqlDatabase::exec(const QString& query) const
{
    return SqlQuery(query, *this);
}

SqlQuery::SqlQuery()
{

}

SqlQuery::~SqlQuery()
{
    //qDebug() << "Scheduling deletion of holder to db with driver" << _deleter->_database.driver();
}

SqlQuery::SqlQuery(const QString& query, SqlDatabase db) :
    QSqlQuery(query, db),
    _deleter(new DatabaseHolder{db}, &deleteQObject)
{

}

SqlQuery::SqlQuery(SqlDatabase db) :
    QSqlQuery(db),
    _deleter(new DatabaseHolder{db}, &deleteQObject)
{

}

SqlDatabase SqlQuery::database() const
{
    return (!_deleter.isNull()) ? _deleter->_database : SqlDatabase();
}



