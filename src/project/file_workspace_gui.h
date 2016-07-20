#ifndef FILE_WORKSPACE_GUI_H
#define FILE_WORKSPACE_GUI_H

#include <QWidget>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include "abstract_workspace_gui.h"
#include "project_gui.h"
#include "file_workspace.h"
#include "file_datatype_helper.h"

class FileProjectNewDialog;
class FileProjectLoadDialog;
class FileWorkspaceNewDialog;
class FileWorkspaceLoadDialog;
class FileWorkspaceEditDialog;

class FileWorkspaceGui : public AbstractWorkspaceGui
{
    Q_OBJECT

public:
    Q_INVOKABLE FileWorkspaceGui();
    ~FileWorkspaceGui();
    QString workspaceTypeName() const override;
    QDialog* dialogNewWorkspace(QDialog* parent = 0) const override;
    QDialog* dialogLoadWorkspace(QDialog* parent = 0) const override;
    QDialog* dialogEditWorkspace(QDialog* parent = 0, QSharedPointer<AbstractWorkspace> workspace = QSharedPointer<AbstractWorkspace>()) const override;
    void addListWidgetItem(QListWidget* parentListWidget) override;
    bool isTypeFriendly(const QSharedPointer<AbstractWorkspace>& workspace) const override;
    bool removeProject(const QSharedPointer<ProjectGui>& projectGui) override;
    bool deleteProject(const QSharedPointer<ProjectGui>& projectGui) override;
    bool saveExternChangedProjects() override;
    void resetExternChangedProjects() override;

private slots:
    void customContextMenuRequested(const QPoint& position);
    void newWorkspaceAccepted();
    void loadWorkspaceAccepted();
    void editWorkspaceAccepted();
    void showNewProjectDialog();
    void showOpenProjectDialog();
    void showWorkspaceInformation();
    void onProjectLoad(const QSharedPointer<ProjectGui>& projectGui) override;
    void onProjectUnload(const QSharedPointer<ProjectGui>& projectGui) override;

private:
    QSharedPointer<FileWorkspace> newFileWorkspace(const FileWorkspaceData* workspaceProperties);
    QMenu* createWorkspaceContextMenue();
    FileWorkspaceNewDialog* _widgetNewWorkspace = nullptr;
    FileWorkspaceLoadDialog* _widgetLoadWorkspace = nullptr;
    QString _lastUsedWorkspacePath;
};
#endif // FILE_WORKSPACE_GUI_H
