#include "sql_workspace.h"
#include <QDebug>

SqlWorkspace::SqlWorkspace(): AbstractWorkspace(QLatin1Literal("Sql"))
{
}

SqlWorkspace::~SqlWorkspace()
{
}

void SqlWorkspace::init()
{

}

bool SqlWorkspace::save()
{
    return true;
}

bool SqlWorkspace::test()
{
    return true;
}

bool SqlWorkspace::deleteProject(const QSharedPointer<AbstractProject>& project)
{
    Q_UNUSED(project);
    return true;
}

bool SqlWorkspace::deleteWorkspace(bool deleteProjects)
{
    Q_UNUSED(deleteProjects);
    return true;
}

bool SqlWorkspace::compare(const QSharedPointer<AbstractWorkspace>& workspace) const
{
    Q_UNUSED(workspace);
    return true;
}

int SqlWorkspace::port() const
{
    return _port;
}

void SqlWorkspace::setPort(int port)
{
    _port = port;
}

QString SqlWorkspace::user() const
{
    return _user;
}

void SqlWorkspace::setUser(const QString& user)
{
    _user = user;
}

QString SqlWorkspace::password() const
{
    return _password;
}

void SqlWorkspace::setPassword(const QString& password)
{
    _password = password;
}

QString SqlWorkspace::host() const
{
    return _host;
}

void SqlWorkspace::setHost(const QString& host)
{
    _host = host;
}

QString SqlWorkspace::database() const
{
    return _database;
}

void SqlWorkspace::setDatabase(const QString& database)
{
    _database = database;
}

QString SqlWorkspace::sourceInformation() const
{
    return _sourceInformation;
}

void SqlWorkspace::setSourceInformation(const QString& sourceInformation)
{
    _sourceInformation = sourceInformation;
}

void SqlWorkspace::initProjectList()
{

}
