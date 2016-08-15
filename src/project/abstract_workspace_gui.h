#ifndef ABSTRACT_WORKSPACE_GUI_H
#define ABSTRACT_WORKSPACE_GUI_H

#include <QObject>
#include "item/item_view.h"
#include "helper/dom_helper.h"
#include "abstract_workspace.h"
#include "projectlist_dockwidget.h"
#include "project_manager.h"
#include "project_save_reminder_dialog.h"

enum WorkspaceGuiType {
    newWorkspace,
    loadWorkspace,
    editWorkspace
};

class AbstractWorkspaceGui : public QObject
{
    Q_OBJECT
public:
    virtual ~AbstractWorkspaceGui();
    virtual QDialog* dialogNewWorkspace(QDialog* parent = 0) const = 0;
    virtual QDialog* dialogLoadWorkspace(QDialog* parent = 0) const = 0;
    virtual QDialog* dialogEditWorkspace(QDialog* parent = 0, QSharedPointer<AbstractWorkspace> workspace = QSharedPointer<AbstractWorkspace>()) const = 0;
    virtual QString workspaceTypeName() const = 0;
    virtual void addListWidgetItem(QListWidget* parentListWidget) = 0;
    virtual bool isTypeFriendly(const QSharedPointer<AbstractWorkspace>& workspace) const = 0;
    virtual bool removeProject(const QSharedPointer<ProjectGui>& projectGui, bool showMessagebox = true) = 0;
    virtual bool deleteProject(const QSharedPointer<ProjectGui>& projectGui, bool showMessagebox = true) = 0;
    virtual void editProject(QSharedPointer<ProjectGui> projectGui) = 0;
    virtual bool saveExternChangedProjects() = 0;
    virtual void resetExternChangedProjects() = 0;
    virtual void showChangeSourceDialog(const QSharedPointer<AbstractWorkspace>& workspace) = 0;
    virtual bool exportToFileSystem(QString exportPath, ExportOptions options, QVector<QSharedPointer<AbstractProject>> projects) = 0;
    virtual void searchProjectSource(QSharedPointer<ProjectGui> projectGui) = 0;
    virtual void createProjectsFromImport(const QStringList& projectPaths, bool overwrite = false) = 0;

    QSharedPointer<AbstractWorkspace> workspace() const;
    void setItemViewTabWidget(QTabWidget* itemViewTabWidget);
    QTabWidget* itemViewTabWidget() const;
    void setProjectListDockWidget(ProjectListDockWidget* projectListDockWidget);
    ProjectListDockWidget* projectListDockWidget() const;
    void setProjectManager(ProjectManager* projectManager);
    ProjectManager* projectManager() const;
    void deleteWorkspace(const QSharedPointer<AbstractWorkspace>& workspace);
    bool openWorkspace(const QSharedPointer<AbstractWorkspace>& workspace);
    bool closeWorkspace();
    void addProjectGui(const QSharedPointer<ProjectGui>& projectGui);
    bool loadProject(const QSharedPointer<ProjectGui>& projectGui);
    bool unloadProject(const QSharedPointer<ProjectGui>& projectGui, bool saveReminder=true);
    bool unloadProjects(const QVector<QSharedPointer<ProjectGui>>& projectGui);
    bool unloadAllProjectsExceptThis(const QSharedPointer<ProjectGui>& projectGui);
    bool unloadAllProjectsExceptVisible();
    QMenu* switchWorkspaceMenu() const;
    void setSwitchWorkspaceMenu(QMenu* switchWorkspaceMenu);

    QSharedPointer<ProjectGui> projectGui(const QSharedPointer<AbstractProject>& project);

private:
    void setWorkspace(const QSharedPointer<AbstractWorkspace>& workspace);
    void setGuiElements();
    void unsetGuiElements();
    bool showSaveReminder(const QVector<QSharedPointer<ProjectGui>>& dirtyProjects);
    QSharedPointer<AbstractWorkspace> _workspace;
    bool _isOpen = false;

protected:
    AbstractWorkspaceGui();

    template<typename T>
    QSharedPointer<T> workspaceTypeCast(QSharedPointer<AbstractWorkspace> abstractWorkspace)
    {
        return qSharedPointerCast<T>(abstractWorkspace);
    }

    ProjectListDockWidget* _projectListDockWidget = nullptr;
    QTabWidget* _itemViewTabWidget = nullptr;
    ProjectManager* _projectManager = nullptr;
    QMenu* _switchWorkspaceMenu = nullptr;

    virtual void onProjectLoad(const QSharedPointer<ProjectGui>& projectGui);
    virtual void onProjectUnload(const QSharedPointer<ProjectGui>& projectGui);

signals:
    void acceptWorkspace(QSharedPointer<AbstractWorkspace> workspace);
    void showContextMenu(const QPoint& position);
    void switchWorkspace();

private slots:
    void closeProjectTabWidget(int tabIndex);
    void projectGuiLabelChanged(QSharedPointer<ProjectGui> projectGui);


public slots:
    void workspaceMenuRequested(const QPoint& position);
    void multiProjectMenuRequested(const QPoint& position, const QList<QSharedPointer<ProjectGui>> projectGuis);
    void onSaveProject();
    void onSaveAllProjects();
    void onUnloadAllProjects();
    void onLoadAllProjects();
    void onRemoveSelectedProjects();
    void onDeleteSelectedProjects();
    void onUnloadAllProjectsExceptSelected();
    void onEnableFastLoadForSelected(bool enable);

};
#endif // ABSTRACT_WORKSPACE_GUI_H
