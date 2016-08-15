#include "project_manager_gui.h"
#include "helper/startup_helper.h"
#include "plugin/plugin_manager.h"
#include "gui/gui_manager.h"
#include "select_workspace_dialog.h"
#include "file_workspace_gui.h"
#include "sql_workspace_gui.h"
#include "project_manager.h"
#include "project_manager_config.h"
#include <QMenu>
#include <QAction>


STARTUP_ADD_SINGLETON(ProjectManagerGui)

ProjectManagerGui::ProjectManagerGui()
{
    // Create a new ProjectManager
    _projectManager = new ProjectManager(this);
    // Create a new ProjectListDockWidget to provide the workspace projectlist DockWidget in mainGUI
    _projectListDockWidget = new ProjectListDockWidget();
    // Create a new QTabWidget to provide the workspace itemview Widget in mainGUI
    _itemViewTabWidget = new QTabWidget();
    _itemViewTabWidget->setTabsClosable(true);
    _itemViewTabWidget->setElideMode(Qt::ElideNone);
    _itemViewTabWidget->setMovable(true);
    _itemViewTabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);

    // Register all InterfaceWorkspaceGui classes at PluginManager
    PluginManager::instance()->addPluginComponent<FileWorkspaceGui, AbstractWorkspaceGui>();

    // Uncomment next line if you implement SQL project handling
    //PluginManager::instance()->addPluginComponent<SqlWorkspaceGui, AbstractWorkspaceGui>();

    // Get all available AbstractWorkspaceGui instances from PluginManager and
    // store these into the QVector _workspaceGuiVector
    for (AbstractWorkspaceGui* abstractWorkspaceGui : PluginManager::instance()->createInstances<AbstractWorkspaceGui>()) {
        abstractWorkspaceGui->setProjectManager(_projectManager);
        abstractWorkspaceGui->setItemViewTabWidget(_itemViewTabWidget);
        abstractWorkspaceGui->setProjectListDockWidget(_projectListDockWidget);
        _abstractWorkspaceGuiClasses.append(abstractWorkspaceGui);
    }

    // Submenu to switch workspace fast. The Pixmap (icon) shows the current workspace.
    _menuWorkspaces = new QMenu(0);
    _menuePointSelected = QPixmap(4, 4);
    _menuePointSelected.fill(Qt::black);

    // Set Main gui elements and connect them.
    setMainGuiElements();

    connect(_itemViewTabWidget->tabBar(), &QTabWidget::customContextMenuRequested, this, &ProjectManagerGui::customContextMenuRequested);
    connect(_projectManager, &ProjectManager::recentWorkspaceListChanged, this, &ProjectManagerGui::onRecentWorkspaceListChanged);
    _projectManager->initRecentWorkspaces();
}

ProjectManagerGui::~ProjectManagerGui()
{
    // Delete created pointers
    qDeleteAll(_abstractWorkspaceGuiClasses);
    delete _itemViewTabWidget;
    delete _projectListDockWidget;
    delete _menuWorkspaces;
    delete _projectManager;
}

bool ProjectManagerGui::postInit()
{
    // StartUp Helper postInit function
    return true;
}

bool ProjectManagerGui::preDestroy()
{
    // StartUp Helper preDestroy function
    return true;
}

void ProjectManagerGui::setMainGuiElements()
{
    // Set project manager main gui elements.
    Gui_Manager::instance()->include_In_Layout(_projectListDockWidget, Window_Layout::Left_Area);
    Gui_Manager::instance()->set_central_widget(_itemViewTabWidget);

    // Connect close application menu entry.
    Gui_Manager::instance()->register_close_handler(std::bind(&ProjectManagerGui::closeWorkspace,this));

    // Connect show workspace manager menu entry.
    QAction* actionWorkspaceManager = Gui_Manager::instance()->get_action("Workspace Manager...");
    connect(actionWorkspaceManager, &QAction::triggered, this, &ProjectManagerGui::openWorkspaceManager);
}

AbstractWorkspaceGui* ProjectManagerGui::abstractWorkspaceGuiClass(
    const QSharedPointer<AbstractWorkspace>& workspace,
    const QVector<AbstractWorkspaceGui*>& workspaceGuis)
{
    // Find correct parent workspace gui class.
    for (AbstractWorkspaceGui* workspaceGui : workspaceGuis) {
        if (workspaceGui->isTypeFriendly(workspace)) {
            return workspaceGui;
        }
    }

    qWarning() << QString("No present WorkspaceGui class was matched with Workspace %1.").arg(workspace->metaObject()->className());
    return nullptr;
}

void ProjectManagerGui::updateMenuEntries()
{
    // Get main menu entry "Workspaces".
    QAction* actionWorkspaces = Gui_Manager::instance()->get_action("Switch Workspace...");
    const QSharedPointer<AbstractWorkspace> currentWorkspace = _projectManager->currentWorkspace();
    _menuWorkspaces->clear();
    _menuWorkspaces->setTitle(actionWorkspaces->text());

    // Rebuild submenu to switch workspace fast.
    for (const QSharedPointer<AbstractWorkspace> workspace : _projectManager->recentWorkspaces()) {

        // Create new qaction for every recent used workspace.
        QAction* action = new QAction(workspace->name(), _menuWorkspaces);
        // Store abstract workspace as qvariant data in qaction object.
        action->setData(QVariant::fromValue(workspace));

        if (!currentWorkspace.isNull() && workspace->compare(currentWorkspace)) {
            // If workspace is "current workspace" then mark entry (black square pixmap icon).
            action->setIcon(QIcon(_menuePointSelected));
        }

        // Connect action trigger to switch workspace.
        connect(action, &QAction::triggered, this, &ProjectManagerGui::onSwitchWorkspaceAction);

        if (!workspace->isValid()) {
            action->setDisabled(true);
            action->setToolTip(workspace->lastError());
        }

        // Add action to submenu "Workspaces".
        _menuWorkspaces->addAction(action);
    }

    if(!currentWorkspace.isNull()){
         AbstractWorkspaceGui* abstractWorkspaceGui = abstractWorkspaceGuiClass(currentWorkspace, _abstractWorkspaceGuiClasses);
         if(abstractWorkspaceGui != nullptr){
            abstractWorkspaceGui->setSwitchWorkspaceMenu(_menuWorkspaces);
         }
    }
    // Set updated submenu to switch workspace fast to main menu entry "Workspaces"
    actionWorkspaces->setMenu(_menuWorkspaces);
}

bool ProjectManagerGui::start()
{
    bool defaultWorkspaceSet = false;

    // Try to load a default workspace. If there is no default workspace set, show select-workspace-dialog.
    if (_projectManager->isDefaultWorkspaceSet()) {
        QSharedPointer<AbstractWorkspace> workspace = _projectManager->defaultWorkspace();

        // Prepare GUI Projectlist and add it to Traviz main GUI.
        if (openWorkspace(workspace)) {
            defaultWorkspaceSet = true;
        } else {
            QMessageBox::warning(0, tr("Loading workspace."), QString("Error open Workspace %1.\n%2").
                                 arg(workspace->name()).
                                 arg(lastError()), QMessageBox::Ok);
        }
    }

    if (!defaultWorkspaceSet) {
        // Init workspaceDialogCode value to show dialog min. once.
        int workspaceDialogCode = -1;

        // If ProjectManagerGui::switchWorkspace generates an error -> call showSelectWorkspaceDialog() again
        // (this function is called in ProjectManagerGui::showSelectWorkspaceDialog()).
        // This provides the possibility to choose a different workspace.
        while (workspaceDialogCode < 0) {
            workspaceDialogCode = showSelectWorkspaceDialog();
        }

        if (workspaceDialogCode == 0) {
            // If no workspace was selected.
            qWarning() << "Shutdown Traviz, cause no workspace was seleceted.";
            return false;
        }
    }

    return true;
}

int ProjectManagerGui::showSelectWorkspaceDialog()
{
    SelectWorkspaceDialog selectWorkspaceDialog(_projectManager);
    selectWorkspaceDialog.setWorkspaceGuiVector(_abstractWorkspaceGuiClasses);

    // Execute selectWorkspaceDialog.
    int workspaceDialogCode = selectWorkspaceDialog.exec();

    if (workspaceDialogCode == QDialog::Accepted) {
        // Set selected workspace.
        const QSharedPointer<AbstractWorkspace> selectedWorkspace = selectWorkspaceDialog.selectedWorkspace();

        if (openWorkspace(selectedWorkspace)) {
            // If checkbox UseAsDefaultWorkspace is checked enable selected workspace attribute setDefault.
            if (selectedWorkspace->isDefault()) {
                // setDefault to false for each workspace except the selectedWorkspace.
                for (QSharedPointer<AbstractWorkspace> workspace : _projectManager->recentWorkspaces()) {
                    if (!workspace->compare(selectedWorkspace)) {
                        workspace->setDefault(false);
                    }
                }

                // Save changes to recentUsedWorkspaces file.
                _projectManager->saveRecentWorkspacesSettings();
            }
        } else {
            // switchWorkspace function generated an error.
            workspaceDialogCode = -1;
        }
    }

    // Return workspaceDialogCode. Normal return results are 0 and 1 based on QDialog, switchWorkspace error = -1.
    return workspaceDialogCode;
}

bool ProjectManagerGui::openWorkspace(QSharedPointer<AbstractWorkspace> workspace)
{
    if (workspace.isNull()) {
        _lastError = tr("Workspace is null.");
        return false;
    }

    _lastError.clear();

    if (!workspace->test()) {
        _lastError = workspace->lastError();
        return false;
    }
    _lastError.clear();

    // Close current workspace
    QSharedPointer<AbstractWorkspace> currentWorkspace = _projectManager->currentWorkspace();
    AbstractWorkspaceGui* abstractWorkspaceGui = nullptr;

    if (!currentWorkspace.isNull()) {
        if (currentWorkspace->compare(workspace)) {
            _lastError = tr("Workspace %1 is already open").arg(workspace->name());
            return false;
        }

        _lastError.clear();

        if (!closeWorkspace()) {
            _lastError = tr("Could not close Workspace %1.").arg(workspace->name());
            return false;
        }

        _lastError.clear();
        abstractWorkspaceGui = abstractWorkspaceGuiClass(currentWorkspace, _abstractWorkspaceGuiClasses);
        if (abstractWorkspaceGui != nullptr) {
            disconnect(abstractWorkspaceGui, &AbstractWorkspaceGui::switchWorkspace, this, &ProjectManagerGui::onSwitchWorkspaceAction);
        }
    }

    // Link new workspace to gui workspace class
    abstractWorkspaceGui = abstractWorkspaceGuiClass(workspace, _abstractWorkspaceGuiClasses);

    if (abstractWorkspaceGui == nullptr) {
        _lastError = tr("No present WorkspaceGui class was matched with Workspace %1.").arg(workspace->name());
        return false;
    }

    _lastError.clear();

    // Open workspace.
    if (!abstractWorkspaceGui->openWorkspace(workspace)) {
        _lastError = tr("Error open Workspace %1.").arg(workspace->name());
        return false;
    }

    _lastError.clear();

    // Set current workspace, update menu and connect workspace to switch workspace slot
    _projectManager->setCurrentWorkspace(workspace);
    updateMenuEntries();
    connect(abstractWorkspaceGui, &AbstractWorkspaceGui::switchWorkspace, this, &ProjectManagerGui::onSwitchWorkspaceAction);
    return true;
}

bool ProjectManagerGui::closeWorkspace()
{
    QSharedPointer<AbstractWorkspace> workspace = _projectManager->currentWorkspace();
    if (!workspace.isNull()) {
        AbstractWorkspaceGui* abstractWorkspaceGui = abstractWorkspaceGuiClass(workspace, _abstractWorkspaceGuiClasses);

        if (abstractWorkspaceGui == nullptr) {
            return false;
        }
        return abstractWorkspaceGui->closeWorkspace();
    }

    return true;
}

QString ProjectManagerGui::lastError() const
{
    return _lastError;
}

void ProjectManagerGui::openWorkspaceManager()
{
    showSelectWorkspaceDialog();
}

void ProjectManagerGui::onSwitchWorkspaceAction()
{
    // Get sender qobject and cast it to qaction.
    QAction* action = qobject_cast<QAction*>(sender());

    if (action != nullptr) {
        // Handle the data property -> get selected workspace and switch.
        QSharedPointer<AbstractWorkspace> workspace = action->data().value<QSharedPointer<AbstractWorkspace>>();

        if (workspace.isNull()) {
            qWarning() << QString("Workspace cast failed. Workspace is null");
            return;
        }

        if (!openWorkspace(workspace)) {
            qWarning() << QString("Action button switch to workspace %1 failed.").arg(workspace->name());
        }
    } else {
        qWarning() << QString("Error - qobject_cast faild. Could not cast to QAction");
    }
}

void ProjectManagerGui::onWorkspaceNameChanged(const QString& workspaceName)
{
    Q_UNUSED(workspaceName);
    updateMenuEntries();
}

void ProjectManagerGui::onWorkspaceUpdated()
{
    _projectManager->saveRecentWorkspacesSettings();
}

void ProjectManagerGui::onRecentWorkspaceListChanged()
{
    for (const QSharedPointer<AbstractWorkspace> abstractWorkspace : _projectManager->recentWorkspaces()) {
        connect(abstractWorkspace.data(), &AbstractWorkspace::workspaceNameChanged,
                this, &ProjectManagerGui::onWorkspaceNameChanged, Qt::UniqueConnection);
        connect(abstractWorkspace.data(), &AbstractWorkspace::workspaceUpdated,
                this, &ProjectManagerGui::onWorkspaceUpdated, Qt::UniqueConnection);
    }

    updateMenuEntries();
}

void ProjectManagerGui::customContextMenuRequested(const QPoint position)
{
    QWidget* tabWidget = _itemViewTabWidget->widget(_itemViewTabWidget->tabBar()->tabAt(position));

    if (tabWidget != nullptr) {
         _itemViewTabWidget->setCurrentWidget(tabWidget);
        // Get project from QVariant Data property - Qt::UserRole
        QSharedPointer<ProjectGui> projectGui = tabWidget->property(itemViewWidgetPropertyLabel).value<QSharedPointer<ProjectGui>>();
        // Show project context menue
        projectGui->showProjectContextMenue(_itemViewTabWidget->mapToGlobal(position));
    }
}
