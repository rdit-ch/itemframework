#ifndef FILE_WORKSPACE_H
#define FILE_WORKSPACE_H

#include <QFileSystemWatcher>
#include <QFileInfo>
#include "abstract_workspace.h"

class FileWorkspace : public AbstractWorkspace
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath USER true)

public:
    Q_INVOKABLE FileWorkspace();
    ~FileWorkspace();
    static QSharedPointer<FileWorkspace> createFileWorkspaceFromFile(const QString& filePath);

    bool compare(const QSharedPointer<AbstractWorkspace>& otherWorkspace) const Q_DECL_OVERRIDE;
    bool deleteWorkspace(bool deleteProjects = false) Q_DECL_OVERRIDE;
    bool deleteProject(const QSharedPointer<AbstractProject>& project) Q_DECL_OVERRIDE;
    bool save() Q_DECL_OVERRIDE;
    bool test() Q_DECL_OVERRIDE;
    void init() Q_DECL_OVERRIDE;
    void update() Q_DECL_OVERRIDE;
    void setOpen(bool isOpen) Q_DECL_OVERRIDE;

    void setFilePath(const QString& filePath);
    void setFileEditMode(bool editMode);
    QString filePath() const;
    QString path() const;
    QString fileName() const;

private:
    void initProjects();
    void updateProjects();
    bool validateFileProperties();
    bool _fileInfoChanged = false;
    bool _internalSave = false;
    bool _editMode = false;
    QFileInfo _fileInfo;
    QFileSystemWatcher _fileSystemWatcher;

private slots:
    void onWorkspaceFileChanged();
};
#endif // FILE_WORKSPACE_H
