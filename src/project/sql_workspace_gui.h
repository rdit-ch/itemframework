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
    void addListWidgetItem(QListWidget* parentListWidget) override;
    QString workspaceTypeName() const override;
    QDialog* dialogNewWorkspace(QDialog* parent = 0) const override;
    QDialog* dialogLoadWorkspace(QDialog* parent = 0) const override;
    QDialog* dialogEditWorkspace(QDialog* parent = 0, QSharedPointer<AbstractWorkspace> workspace = QSharedPointer<AbstractWorkspace>()) const override;
    bool isTypeFriendly(const QSharedPointer<AbstractWorkspace>& workspace) const override;
    bool removeProject(const QSharedPointer<ProjectGui>& projectGui) override;
    bool deleteProject(const QSharedPointer<ProjectGui>& projectGui) override;
    bool saveExternChangedProjects() override;
    void resetExternChangedProjects() override;

signals:
    void acceptWorkspace(QSharedPointer<AbstractWorkspace>);
};

#endif // SQL_WORKSPACE_GUI_H
