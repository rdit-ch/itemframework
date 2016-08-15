#ifndef SELECT_WORKSPACE_DIALOG_H
#define SELECT_WORKSPACE_DIALOG_H

#include <QDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QStackedWidget>
#include "project_manager.h"
#include "abstract_workspace_gui.h"

namespace Ui
{
class SelectWorkspaceDialog;
}

class SelectWorkspaceDialog : public QDialog
{
    Q_OBJECT

public:
    SelectWorkspaceDialog(ProjectManager* projectManager);
    ~SelectWorkspaceDialog();
    QSharedPointer<AbstractWorkspace> selectedWorkspace() const;
    QVector<AbstractWorkspaceGui*> workspaceGuiVector() const;
    void setWorkspaceGuiVector(const QVector<AbstractWorkspaceGui*>& workspaceGuiVector);

private:
    enum TreeWidgetColumn {
        Workspace,
        Open,
        Default,
        LastUsed,
        Name,
        Type,
        Connection
    };


    void setSelectedWorkspace(const QSharedPointer<AbstractWorkspace>& workspace);
    void clearSelectedWorkspace();
    void showContextMenu(const QPoint& position, const QSharedPointer<AbstractWorkspace>& workspace);
    QDialog* createDialogWorkspaceGui(const WorkspaceGuiType workspaceGuiType, const QVector<AbstractWorkspaceGui*> workspaceGuiVector);

    QVector<QTreeWidgetItem*> treeWidgetItems() const;
    Ui::SelectWorkspaceDialog* _ui;
    ProjectManager* _projectManager = nullptr;
    QSharedPointer<AbstractWorkspace> _selectedWorkspace;
    QVector<AbstractWorkspaceGui*> _workspaceGuiVector;

private slots:
    void setRecentUsedWorkspaces();
    void searchRecentUsedWorkspaces();
    void customContextMenuRequested(const QPoint& position);
    void acceptWorkspace(QSharedPointer<AbstractWorkspace> selectedWorkspace);
    void recentWorkspaceSelected(QTreeWidgetItem* item, int column);
    void loadRecentUsedWorkspace(QTreeWidgetItem* item, int column);
    void showDialogLoadWorkspace();
    void showDialogNewWorkspace();
    void showDialogEditWorkspace();
    void removeFromRecentList();
    void deleteWorkspace();
    void recentUsedWorkspaceSelectionChanged();
    void defaultWorkspaceChange(bool checked);
    void onWorkspaceUpdated();
};
#endif // SELECT_WORKSPACE_DIALOG_H
