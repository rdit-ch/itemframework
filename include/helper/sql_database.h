#ifndef SQL_DATABASE_H_
#define SQL_DATABASE_H_

#include "appcore.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QObject>
#include <QSharedPointer>

/**
 * @brief The SqlDatabase class is equivalent to QSqlDatabase with some small changes:
 * - The database does not need to have a connection name, but is still shared.
 * - The database will automatically be closed if no more references (e.g SqlQuery's) are pointing to it
 * - The database only works with SqlQuery instead of QSqlQuery.
 * \sa SqlQuery
 */

class CORE_EXPORT SqlDatabase : private QSqlDatabase
{
    friend class SqlQuery;
public:
    /**
     * @brief Creates an empty, invalid SqlDatabase object. Use SqlDatabase(const QString& type)
     * to get valid SqlDatabase objects.
     */
    SqlDatabase();

    /**
     * @brief Creates a SqlDatabase connection that uses the driver referred to by \a type.
     * If the type is not recognized, the database connection will have no functionality.
     *
     * @param type The database type (see also QSqlDatabase::QSqlDatabase(const QString& type)).
     */
    SqlDatabase(const QString& type);

    /**
     * @brief Executes an SQL statement on the database and returns a SqlQuery Object. Use lastError() to retrive error information.
     * @param query The SQL query string to execute
     * @return a SqlQuery object containing the result
     */
    class SqlQuery exec(const QString& query) const;

    //All methods from the base class are made public:
    using QSqlDatabase::close;
    using QSqlDatabase::commit;
    using QSqlDatabase::connectOptions;
    //Exception: QSqlDatabase::connectionName cannot be used
    using QSqlDatabase::databaseName;
    using QSqlDatabase::driver;
    using QSqlDatabase::driverName;
    using QSqlDatabase::hostName;
    using QSqlDatabase::isOpen;
    using QSqlDatabase::isOpenError;
    using QSqlDatabase::isValid;
    using QSqlDatabase::lastError;
    using QSqlDatabase::numericalPrecisionPolicy;
    using QSqlDatabase::open;
    using QSqlDatabase::password;
    using QSqlDatabase::port;
    using QSqlDatabase::primaryIndex;
    using QSqlDatabase::record;
    using QSqlDatabase::rollback;
    using QSqlDatabase::setConnectOptions;
    using QSqlDatabase::setDatabaseName;
    using QSqlDatabase::setHostName;
    using QSqlDatabase::setNumericalPrecisionPolicy;
    using QSqlDatabase::setPassword;
    using QSqlDatabase::setPort;
    using QSqlDatabase::setUserName;
    using QSqlDatabase::tables;
    using QSqlDatabase::transaction;
    using QSqlDatabase::userName;

};

/**
 * @brief The SqlQuery class inherits from QSqlQuery class, but:
 * - Works with SqlDatabase instead of QSqlDatabase
 * - Keeps a reference to the SqlDatabase it works with, so that the database is automatically closed if no more queries are pointing to it.
 * - Has a database() method to get the database back
 * \sa SqlDatabase
 */
class CORE_EXPORT SqlQuery: public QSqlQuery
{
    friend class TestSqlDatabase;
public:
    /**
     * @brief Constructs an invalid query
     */
    SqlQuery();

    /**
     * @brief Destroys the SqlQuery. If this query is the last object which holds a reference to the associate SqlDatabase, then the database will also be closed/deleted.
     */
    ~SqlQuery();


    /**
     * @brief Constructs a SqlQuery using the Query-String and a Database. If the query is not an empty string, the query will be executed.
     * @param query The SQL query string to execute
     * @param db The SqlDatabase to execute the query on
     */
    SqlQuery(const QString& query, SqlDatabase db);

    /**
     * @brief Constructs a SqlQuery for the given SqlDatabase
     * @param db The SqlDatabase to execute the query on
     */
    SqlQuery(SqlDatabase db);

    /**
     * @brief Returns the associated SqlDatabase object
     * @return the SqlDatabase object
     */
    SqlDatabase database() const;

private:
    QSharedPointer<class DatabaseHolder> _deleter;
};

#endif /* SQL_DATABASE_H_ */
