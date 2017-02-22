#ifndef SQL_WORKSPACE_GUI_H
#define SQL_WORKSPACE_GUI_H

#include "abstract_workspace_gui.h"
#include "sql_workspace_new_dialog.h"
#include "sql_workspace.h"

class SqlWorkspaceGui : public AbstractWorkspaceGui
{
    Q_OBJECT

public:
    Q_INVOKABLE SqlWorkspaceGui();
    void addListWidgetItem(QListWidget* parentListWidget) Q_DECL_OVERRIDE;
    QString workspaceTypeName() const Q_DECL_OVERRIDE;
    QDialog* dialogNewWorkspace(QDialog* parent = 0)  Q_DECL_OVERRIDE;
    QDialog* dialogLoadWorkspace(QDialog* parent = 0) Q_DECL_OVERRIDE;
    QDialog* dialogEditWorkspace(QDialog* parent = 0, QSharedPointer<AbstractWorkspace> workspace = QSharedPointer<AbstractWorkspace>()) const Q_DECL_OVERRIDE;
    bool isTypeFriendly(const QSharedPointer<AbstractWorkspace>& workspace) const Q_DECL_OVERRIDE;
    bool removeProject(const QSharedPointer<ProjectGui>& projectGui, bool showMessagebox = true) Q_DECL_OVERRIDE;
    bool deleteProject(const QSharedPointer<ProjectGui>& projectGui, bool showMessagebox = true) Q_DECL_OVERRIDE;
    bool saveExternChangedProjects() Q_DECL_OVERRIDE;
    void resetExternChangedProjects() Q_DECL_OVERRIDE;
    void editProject(QSharedPointer<ProjectGui> projectGui) Q_DECL_OVERRIDE;
    void showChangeSourceDialog(const QSharedPointer<AbstractWorkspace>& workspace) Q_DECL_OVERRIDE;
    bool exportToFileSystem(QString exportPath, ExportOptions options, QVector<QSharedPointer<AbstractProject>> projects) Q_DECL_OVERRIDE;
    void searchProjectSource(QSharedPointer<ProjectGui> projectGui) Q_DECL_OVERRIDE;
    void createProjectsFromImport(const QStringList& projectPaths, bool overwrite = false) Q_DECL_OVERRIDE;

signals:
    void acceptWorkspace(QSharedPointer<AbstractWorkspace>);
};

#endif // SQL_WORKSPACE_GUI_H
