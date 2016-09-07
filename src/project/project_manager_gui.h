#ifndef PROJECT_MANAGER_GUI_H
#define PROJECT_MANAGER_GUI_H

#include <QDialog>
#include "abstract_workspace_gui.h"

class ProjectManager;
class ProjectListDockWidget;

class ProjectManagerGui : public QObject, public Singleton<ProjectManagerGui>
{
    Q_OBJECT

// Singleton properties----------------------------------

    Q_INTERFACES(AbstractSingleton)
    Q_CLASSINFO("guiModule", "true")
    Q_CLASSINFO("dependsOn", "PluginManager")
    Q_CLASSINFO("dependsOn", "GuiManager")

public:
    Q_INVOKABLE ProjectManagerGui();
    ~ProjectManagerGui();

protected:
    bool postInit();
    bool preDestroy();
//-------------------------------------------------------

public:
    static AbstractWorkspaceGui* abstractWorkspaceGuiClass(
        const QSharedPointer<AbstractWorkspace>& workspace,
        const QVector<AbstractWorkspaceGui*>& workspaceGuis);

    bool start();
    int showSelectWorkspaceDialog();
    bool openWorkspace(QSharedPointer<AbstractWorkspace> workspace);
    QString lastError() const;

private:
    void setMainGuiElements();
    void updateMenuEntries();
    bool closeWorkspace();
    void addProjectListDockWidgetToMainGui(ProjectListDockWidget* projectListDockWidget);
    void removeProjectHandlerWindowToGui(ProjectListDockWidget* projectHandlerWindow);
    ProjectListDockWidget* prepareProjectListDockWidget(QSharedPointer<AbstractWorkspace> workspace);
    ProjectManager* _projectManager = nullptr;
    ProjectListDockWidget* _projectListDockWidget = nullptr;
    QTabWidget* _itemViewTabWidget = nullptr;
    QVector<AbstractWorkspaceGui*> _abstractWorkspaceGuiClasses;
    QMenu* _menuWorkspaces = nullptr;
    QPixmap _menuePointSelected;
    QString _lastError;

private slots:
    void openWorkspaceManager();
    void onSwitchWorkspaceAction();
    void onWorkspaceNameChanged(const QString& workspaceName);
    void onWorkspaceUpdated();
    void onRecentWorkspaceListChanged();
    void customContextMenuRequested(const QPoint position);
};

#endif // PROJECT_MANAGER_GUI_H
