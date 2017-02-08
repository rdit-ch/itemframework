#ifndef SQL_WORKSPACE_H
#define SQL_WORKSPACE_H

#include "abstract_workspace.h"

class SqlWorkspace : public AbstractWorkspace
{
    Q_OBJECT
    Q_PROPERTY(int port READ port WRITE setPort USER true)
    Q_PROPERTY(QString user READ user WRITE setUser USER true)
    Q_PROPERTY(QString password READ password WRITE setPassword USER true)
    Q_PROPERTY(QString host READ host WRITE setHost USER true)
    Q_PROPERTY(QString database READ database WRITE setDatabase USER true)

public:
    Q_INVOKABLE SqlWorkspace();
    ~SqlWorkspace();

    void init() Q_DECL_OVERRIDE;
    void update() Q_DECL_OVERRIDE;
    bool compare(const QSharedPointer<AbstractWorkspace>& otherWorkspace) const Q_DECL_OVERRIDE;
    bool save() Q_DECL_OVERRIDE;
    bool test() Q_DECL_OVERRIDE;
    bool deleteProject(const QSharedPointer<AbstractProject>& project) Q_DECL_OVERRIDE;
    bool deleteWorkspace(bool deleteProjects = false) Q_DECL_OVERRIDE;

    int port() const;
    void setPort(int port);
    QString user() const;
    void setUser(const QString& user);
    QString password() const;
    void setPassword(const QString& password);
    QString host() const;
    void setHost(const QString& host);
    QString database() const;
    void setDatabase(const QString& database);
    QString sourceInformation() const;
    void setSourceInformation(const QString& sourceInformation);

private:
    int _port;
    QString _user;
    QString _password;
    QString _host;
    QString _database;
    QString _sourceInformation;
    void initProjectList();
};
#endif // SQL_WORKSPACE_H
