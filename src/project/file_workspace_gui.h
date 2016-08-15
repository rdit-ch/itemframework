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
class FileProjectEditDialog;
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
    QDialog* dialogNewWorkspace(QDialog* parent = 0) const Q_DECL_OVERRIDE;
    QDialog* dialogLoadWorkspace(QDialog* parent = 0) const Q_DECL_OVERRIDE;
    QDialog* dialogEditWorkspace(QDialog* parent = 0, QSharedPointer<AbstractWorkspace> workspace = QSharedPointer<AbstractWorkspace>()) const Q_DECL_OVERRIDE;
    void addListWidgetItem(QListWidget* parentListWidget) Q_DECL_OVERRIDE;
    bool isTypeFriendly(const QSharedPointer<AbstractWorkspace>& workspace) const Q_DECL_OVERRIDE;
    bool removeProject(const QSharedPointer<ProjectGui>& projectGui, bool showMessagebox = true) Q_DECL_OVERRIDE;
    bool deleteProject(const QSharedPointer<ProjectGui>& projectGui, bool showMessagebox = true) Q_DECL_OVERRIDE;
    void editProject(QSharedPointer<ProjectGui> projectGui) Q_DECL_OVERRIDE;
    bool saveExternChangedProjects() Q_DECL_OVERRIDE;
    void resetExternChangedProjects() Q_DECL_OVERRIDE;
    void showChangeSourceDialog(const QSharedPointer<AbstractWorkspace>& workspace) Q_DECL_OVERRIDE;
    bool exportToFileSystem(QString exportPath, ExportOptions options, QVector<QSharedPointer<AbstractProject>> projects) Q_DECL_OVERRIDE;
    void searchProjectSource(QSharedPointer<ProjectGui> projectGui) Q_DECL_OVERRIDE;
    void createProjectsFromImport(const QStringList& projectPaths, bool overwrite = false) Q_DECL_OVERRIDE;

private slots:
    void showExportWorkspaceWizard();
    void showImportWorkspaceWizard();
    void workspaceMenuRequested(const QPoint& position);
    void newWorkspaceAccepted();
    void loadWorkspaceAccepted();
    void editWorkspaceAccepted();
    void showNewProjectDialog();
    void showEditWorkspaceDialog();
    void showOpenProjectDialog();
    void showWorkspaceInformation();
    void onProjectLoad(const QSharedPointer<ProjectGui>& projectGui) override;
    void onProjectUnload(const QSharedPointer<ProjectGui>& projectGui) override;

private:
    QSharedPointer<FileWorkspace> newFileWorkspace(const FileWorkspaceData* workspaceProperties);
    QString lastUsedWorkspacePath() const;
    FileWorkspaceNewDialog* _widgetNewWorkspace = nullptr;
    FileWorkspaceLoadDialog* _widgetLoadWorkspace = nullptr;
    QString _lastUsedWorkspacePath;
};
#endif // FILE_WORKSPACE_GUI_H
