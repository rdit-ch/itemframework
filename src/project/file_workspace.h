#ifndef FILE_WORKSPACE_H
#define FILE_WORKSPACE_H

#include <QFile>
#include <QDir>
#include <QFileSystemWatcher>
#include "abstract_workspace.h"
#include "file_project.h"

class FileWorkspace : public AbstractWorkspace
{
    Q_OBJECT
    Q_PROPERTY(QString absoluteFilePath READ absoluteFilePath WRITE setAbsoluteFilePath USER true)

public:
    Q_INVOKABLE FileWorkspace();
    ~FileWorkspace();

    static QSharedPointer<FileWorkspace> createFileWorkspaceFromFile(const QString& absoluteFilePath);
    bool compare(const QSharedPointer<AbstractWorkspace>& otherWorkspace) const override;
    bool deleteWorkspace(bool deleteProjects = false) override;
    bool deleteProject(const QSharedPointer<AbstractProject>& project) override;
    bool save() override;
    bool test() override;
    void init() override;

    void setAbsoluteFilePath(const QString& absoluteFilePath);
    QString absoluteFilePath() const;
    QString directory() const;
    QString fileName() const;

private:
    void initProjectList(const QDomDocument& workspaceDomDocument);
    QString _filePath;
    QString _directory;
    QString _fileName;
    bool _internalSave = false;
};
#endif // FILE_WORKSPACE_H
