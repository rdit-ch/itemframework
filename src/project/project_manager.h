#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include <QObject>
#include "file_workspace.h"
#include "sql_workspace.h"

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectManager(QObject* parent = 0);
    ~ProjectManager();
    static QDomDocument recentWorkspacesDomTemplate();

    void initRecentWorkspaces();
    bool saveRecentWorkspacesSettings() const;
    QSharedPointer<AbstractWorkspace> defaultWorkspace() const;
    bool isDefaultWorkspaceSet() const;
    QSharedPointer<AbstractWorkspace> currentWorkspace() const;
    void setCurrentWorkspace(const QSharedPointer<AbstractWorkspace>& currentWorkspace);
    QVector<QSharedPointer<AbstractWorkspace> > recentWorkspaces() const;
    void setRecentWorkspaces(const QVector<QSharedPointer<AbstractWorkspace> >& recentWorkspaces);
    void removeRecentWorkspaces(const QSharedPointer<AbstractWorkspace>& workspace);
    bool addRecentWorkspace(const QSharedPointer<AbstractWorkspace>& recentWorkspace);
    int workspaceCount() const;
    QVector<QSharedPointer<AbstractWorkspace>> workspaces() const;

    QString workDirectoryPath() const;
    void setWorkDirectoryPath(const QString &workDirectoryPath);

private:
    QString _workDirectory;
    QVector<QSharedPointer<AbstractWorkspace>> _recentWorkspaces;
    QSharedPointer<AbstractWorkspace> _currentWorkspace;
    void addWorkspaceElement(QSharedPointer<AbstractWorkspace> workspace, QDomDocument& dom) const;

signals:
    void recentWorkspaceListChanged();
};

#endif // PROJECT_MANAGER_H
