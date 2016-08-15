#include <QWidget>
#include "abstract_workspace_gui.h"
#include "project_manager_config.h"
#include <QCheckBox>
#include <QMessageBox>
#include <QMenu>
#include <QAction>

AbstractWorkspaceGui::AbstractWorkspaceGui()
{
}

AbstractWorkspaceGui::~AbstractWorkspaceGui()
{
}


ProjectManager* AbstractWorkspaceGui::projectManager() const
{
    return _projectManager;
}

ProjectListDockWidget* AbstractWorkspaceGui::projectListDockWidget() const
{
    return _projectListDockWidget;
}

void AbstractWorkspaceGui::setGuiElements()
{
    connect(_itemViewTabWidget, &QTabWidget::tabCloseRequested, this, &AbstractWorkspaceGui::closeProjectTabWidget);
    connect(_projectListDockWidget, &ProjectListDockWidget::loadProject, this, &AbstractWorkspaceGui::loadProject);
    connect(_projectListDockWidget, &ProjectListDockWidget::showWorkspaceContextMenu, this, &AbstractWorkspaceGui::workspaceMenuRequested);
    connect(_projectListDockWidget, &ProjectListDockWidget::showMultiProjectContextMenu, this, &AbstractWorkspaceGui::multiProjectMenuRequested);
}

void AbstractWorkspaceGui::unsetGuiElements()
{
    disconnect(_itemViewTabWidget, 0, this, 0);
    disconnect(_projectListDockWidget, 0, this, 0);
    disconnect(Gui_Manager::instance(), 0, this, 0);
}

QSharedPointer<AbstractWorkspace> AbstractWorkspaceGui::workspace() const
{
    return _workspace;
}

void AbstractWorkspaceGui::setWorkspace(const QSharedPointer<AbstractWorkspace>& workspace)
{
    if (!_workspace.isNull()) {
        disconnect(_workspace.data());
    }

    _workspace = workspace;
}

QTabWidget* AbstractWorkspaceGui::itemViewTabWidget() const
{
    return _itemViewTabWidget;
}

void AbstractWorkspaceGui::setItemViewTabWidget(QTabWidget* itemViewTabWidget)
{
    _itemViewTabWidget = itemViewTabWidget;
}

void AbstractWorkspaceGui::setProjectManager(ProjectManager* projectManager)
{
    _projectManager = projectManager;
}

void AbstractWorkspaceGui::setProjectListDockWidget(ProjectListDockWidget* projectListDockWidget)
{
    _projectListDockWidget = projectListDockWidget;
}

// Project
// ---------------------------------------------------------------------------------------------------------------------

void AbstractWorkspaceGui::addProjectGui(const QSharedPointer<ProjectGui>& projectGui)
{
    connect(projectGui.data(), &ProjectGui::projectGuiLabelChanged, this, &AbstractWorkspaceGui::projectGuiLabelChanged);
    if(!_projectListDockWidget->contains(projectGui)){
        _projectListDockWidget->addProject(projectGui);
    }

    if (_isOpen) {
        loadProject(projectGui);
    }
}

bool AbstractWorkspaceGui::loadProject(const QSharedPointer<ProjectGui>& projectGui)
{
    if (_isOpen) {
        if (!projectGui->isLoaded()) {
            if (projectGui->load()) {
                QWidget* widget = projectGui->itemView();
                widget->setProperty(itemViewWidgetPropertyLabel, QVariant::fromValue(projectGui));
                _itemViewTabWidget->insertTab(projectGui->tabWidgetIndex(), widget, projectGui->projectGuiLabel());
                _itemViewTabWidget->setCurrentIndex(_itemViewTabWidget->indexOf(widget));
                return true;
            } else {
                QCheckBox* removeProjectsCheckBox = new QCheckBox(tr("Remove Project from this Workspace."));
                QMessageBox removeProjectsMessageBox;
                removeProjectsMessageBox.setText(tr("Project \"%1\" is invalid.\n\n%2").
                                                 arg(projectGui->project()->name()).
                                                 arg(projectGui->lastError()));
                removeProjectsMessageBox.setCheckBox(removeProjectsCheckBox);
                removeProjectsMessageBox.setStandardButtons(QMessageBox::Ok);
                removeProjectsMessageBox.setIcon(QMessageBox::Warning);
                removeProjectsMessageBox.exec();

                if (removeProjectsMessageBox.checkBox()->isChecked()) {
                    workspace()->removeProject(projectGui->project());
                    _projectListDockWidget->removeProject(projectGui);
                }
            }
        } else {
            _itemViewTabWidget->setCurrentIndex(_itemViewTabWidget->indexOf(projectGui->itemView()));
            return true;
        }
    }

    return false;
}

bool AbstractWorkspaceGui::unloadProject(const QSharedPointer<ProjectGui>& projectGui, bool saveReminder)
{
    if (_isOpen) {
        if (saveReminder && projectGui->project()->isDirty()) {
            const QVector<QSharedPointer<ProjectGui>> dirtyProjects = QVector<QSharedPointer<ProjectGui>>() << projectGui;

            if (!showSaveReminder(dirtyProjects)) {
                return false;
            }
        }

        if (projectGui->unload()) {
            return true;
        }
    }

    return false;
}

bool AbstractWorkspaceGui::unloadProjects(const QVector<QSharedPointer<ProjectGui>>& projectGuis)
{
    bool unloadProjectsSuccessful = true;

    if (_isOpen) {
        if (!_projectListDockWidget->dirtyProjects().isEmpty()) {
            if (!showSaveReminder(_projectListDockWidget->dirtyProjects())) {
                return false;
            }
        }

        for (QSharedPointer<ProjectGui> projectGui : projectGuis) {
            if (!projectGui->unload()) {
                unloadProjectsSuccessful = false;
            }
        }
    }

    return unloadProjectsSuccessful;
}

bool AbstractWorkspaceGui::unloadAllProjectsExceptThis(const QSharedPointer<ProjectGui>& projectGui)
{
    QVector<QSharedPointer<ProjectGui>> projectGuis;

    for (const QSharedPointer<ProjectGui> p : _projectListDockWidget->projectGuis()) {
        if (p != projectGui) {
            projectGuis.append(p);
        }
    }

    return unloadProjects(projectGuis);
}

bool AbstractWorkspaceGui::unloadAllProjectsExceptVisible()
{
    QVector<QSharedPointer<ProjectGui>> projectGuis;
    QSharedPointer<ProjectGui> visibleProjectGui;

    const QWidget* tabWidget = _itemViewTabWidget->currentWidget();

    if (tabWidget != nullptr) {
        visibleProjectGui = tabWidget->property(itemViewWidgetPropertyLabel).value<QSharedPointer<ProjectGui>>();
    }

    for (const QSharedPointer<ProjectGui> p : _projectListDockWidget->projectGuis()) {
        if (p->isLoaded() && p != visibleProjectGui) {
            projectGuis.append(p);
        }
    }

    return unloadProjects(projectGuis);
}

void AbstractWorkspaceGui::closeProjectTabWidget(int tabIndex)
{
    QWidget* tabWidget = nullptr;

    if (tabIndex > -1) {
        tabWidget = _itemViewTabWidget->widget(tabIndex);

        if (tabWidget != nullptr) {
            QSharedPointer<ProjectGui> projectGui = tabWidget->property(itemViewWidgetPropertyLabel).value<QSharedPointer<ProjectGui>>();

            // unloadProject delete related widget to _itemViewTabWidget
            unloadProject(projectGui);
        }
    }
}

void AbstractWorkspaceGui::onProjectLoad(const QSharedPointer<ProjectGui>& projectGui)
{
    Q_UNUSED(projectGui);
}

void AbstractWorkspaceGui::onProjectUnload(const QSharedPointer<ProjectGui>& projectGui)
{
    Q_UNUSED(projectGui);
}

// Workspace
// ---------------------------------------------------------------------------------------------------------------------

bool AbstractWorkspaceGui::openWorkspace(const QSharedPointer<AbstractWorkspace>& workspace)
{
    setWorkspace(workspace);

    // Open this workspace if it is set
    if (!_workspace.isNull()) {
        // Set workspace isOpen flag = true;
        _isOpen = true;
        _workspace->setOpen(true);
        // Set workspace GUI elements to main GUI;
        setGuiElements();

        // Create for each project a projectGui class
        for (QSharedPointer<AbstractProject> project : _workspace->projects()) {

            QSharedPointer<ProjectGui> projectGui = QSharedPointer<ProjectGui>(new ProjectGui(this, project));
            connect(projectGui.data(), &ProjectGui::projectGuiLabelChanged, this, &AbstractWorkspaceGui::projectGuiLabelChanged);

            connect(projectGui.data(), &ProjectGui::projectLoaded, this, &AbstractWorkspaceGui::onProjectLoad);
            connect(projectGui.data(), &ProjectGui::projectUnloaded, this, &AbstractWorkspaceGui::onProjectUnload);

            // To do check if project has changed and show message box for realod project dom
            // if user is interessted into. Currently project dom reload will be forced on loadProject() call.
            _projectListDockWidget->addProject(projectGui);

            if (project->isValid() && project->isFastLoad()) {
                loadProject(projectGui);
            }
        }

        _workspace->setLastUsedDateTime(QDateTime::currentDateTime().toString(LastUsedDateFormat));
        // Return success - open this workspace
        return true;
    }

    // Return failed - dont open this workspace
    return false;
}

bool AbstractWorkspaceGui::closeWorkspace()
{
    // Close this workspace if it is set and if it is opened
    if (!_workspace.isNull() && _isOpen) {
        // Unload all projects (without saving) that includes delete all ProjectGui ItemView
        if (!unloadProjects(_projectListDockWidget->projectGuis())) {
            return false;
        }

        // Set workspace isOpen flag = false;
        _isOpen = false;
        _workspace->setOpen(false);
        // Unset workspace GUI elements and remove it from main GUI;
        unsetGuiElements();
        // Remove all projects from QListWidget Dock
        _projectListDockWidget->removeAllProjects();
        // Remove all ProjectGui ItemView Tabs from QTabWidget
        _itemViewTabWidget->clear();
        // This workspace is now closed and empty -> clear pointer
        _workspace.clear();
        // Return success
        return true;
    }

    // Return failed - close this workspace
    return false;
}

// Custom Context Menu + Trigger
// ---------------------------------------------------------------------------------------------------------------------


void AbstractWorkspaceGui::workspaceMenuRequested(const QPoint& position)
{
    emit showContextMenu(position);
}

void AbstractWorkspaceGui::multiProjectMenuRequested(const QPoint &position, const QList<QSharedPointer<ProjectGui> > projectGuis)
{
    QMenu projectContextMenue;
    QAction* loadProject = projectContextMenue.addAction(QString("Open selected"));
    QAction* fastLoadProject = projectContextMenue.addAction(QString("Open selected directly"));
    fastLoadProject->setCheckable(true);
    fastLoadProject->setChecked(true);
    fastLoadProject->setData(QVariant::fromValue(projectGuis));
    projectContextMenue.addAction(projectContextMenue.addSeparator());
    QAction* saveProject = projectContextMenue.addAction(QString("Save selected"));
    QAction* saveAllProjects = projectContextMenue.addAction(QLatin1String("Save All"));
    projectContextMenue.addAction(projectContextMenue.addSeparator());
    QAction* unloadProject = projectContextMenue.addAction(QString("Close selected"));
    QAction* unloadAllExceptThisProject = projectContextMenue.addAction(QString("Close All Except selected"));
    unloadAllExceptThisProject->setData(QVariant::fromValue(projectGuis));
    QAction* unloadAllExceptVisibleProjects = projectContextMenue.addAction(QString("Close All Except Visible"));
    unloadAllExceptVisibleProjects->setData(QVariant::fromValue(projectGuis));
    QAction* unloadAllProjects = projectContextMenue.addAction(QLatin1String("Close All"));
    projectContextMenue.addAction(projectContextMenue.addSeparator());
    QAction* removeProject = projectContextMenue.addAction(QString("Remove selected Projects from Workspace"));
    removeProject->setData(QVariant::fromValue(projectGuis));
    QAction* deleteProject = projectContextMenue.addAction(QString("Delete selected Projects"));
    deleteProject->setData(QVariant::fromValue(projectGuis));
    projectContextMenue.addAction(projectContextMenue.addSeparator());
    QAction* editProject = projectContextMenue.addAction(QString("Edit selected Projects..."));
    QAction* infoProject = projectContextMenue.addAction(QString("Project Info"));

    connect(saveAllProjects, &QAction::triggered, this, &AbstractWorkspaceGui::onSaveAllProjects);
    connect(unloadAllProjects, &QAction::triggered, this, &AbstractWorkspaceGui::onUnloadAllProjects);
    connect(unloadAllExceptThisProject, &QAction::triggered, this, &AbstractWorkspaceGui::onUnloadAllProjectsExceptSelected);
    connect(unloadAllExceptVisibleProjects, &QAction::triggered, this, &AbstractWorkspaceGui::unloadAllProjectsExceptVisible);
    connect(fastLoadProject, &QAction::triggered, this, &AbstractWorkspaceGui::onEnableFastLoadForSelected);
    connect(removeProject, &QAction::triggered, this, &AbstractWorkspaceGui::onRemoveSelectedProjects);
    connect(deleteProject, &QAction::triggered, this, &AbstractWorkspaceGui::onDeleteSelectedProjects);

    int dialogPositionOffset = 0;
    for(QSharedPointer<ProjectGui> projectGui : projectGuis){

        projectGui->setDialogPositionOffset(QPoint(dialogPositionOffset, dialogPositionOffset));
        dialogPositionOffset+=20;

        if(!projectGui->isFastLoaded()){
            fastLoadProject->setChecked(false);
        }

        connect(infoProject, &QAction::triggered, projectGui.data(), &ProjectGui::showProjectInfo);
        connect(saveProject, &QAction::triggered, projectGui.data(), &ProjectGui::onSaveTrigger);
        connect(loadProject, &QAction::triggered, projectGui.data(), &ProjectGui::onLoadTrigger);
        connect(unloadProject, &QAction::triggered, projectGui.data(), &ProjectGui::onUnloadTrigger);
        connect(editProject, &QAction::triggered, projectGui.data(), &ProjectGui::onEditTrigger);
    }

    projectContextMenue.exec(position);

}

void AbstractWorkspaceGui::onSaveProject()
{
    QWidget* itemViewTabWidget = _itemViewTabWidget->currentWidget();

    if (itemViewTabWidget != nullptr) {
        QSharedPointer<ProjectGui> projectGui;
        projectGui = itemViewTabWidget->property(itemViewWidgetPropertyLabel).value<QSharedPointer<ProjectGui>>();
        projectGui->save();
    }
}

void AbstractWorkspaceGui::onSaveAllProjects()
{
    for (QSharedPointer<ProjectGui> projectGui : _projectListDockWidget->projectGuis()) {
        projectGui->save();
    }
}

void AbstractWorkspaceGui::onLoadAllProjects()
{
    for (QSharedPointer<ProjectGui> projectGui : _projectListDockWidget->projectGuis()) {
        projectGui->load();
    }
}

void AbstractWorkspaceGui::onRemoveSelectedProjects()
{
    QAction* trigger = qobject_cast<QAction*>(sender());
    if(trigger != nullptr){
        int ret = QMessageBox::question(0, tr("Remove selected Projects."),
                                        tr("Do you want to remove selected Projects from this Workspace?"),
                                        QMessageBox::Ok | QMessageBox::Cancel);

        if (ret == QMessageBox::Ok) {
            for(QSharedPointer<ProjectGui> projectGui : trigger->data().value<QList<QSharedPointer<ProjectGui>>>()){
                removeProject(projectGui,false);
            }
        }
    }
}

void AbstractWorkspaceGui::onDeleteSelectedProjects()
{
    QAction* trigger = qobject_cast<QAction*>(sender());
    if(trigger != nullptr){
        int ret = QMessageBox::question(0, tr("Delete selected Projects."),
                                        tr("Do you want to delete selected Projects?"),
                                        QMessageBox::Ok | QMessageBox::Cancel);

        if (ret == QMessageBox::Ok) {
            for(QSharedPointer<ProjectGui> projectGui : trigger->data().value<QList<QSharedPointer<ProjectGui>>>()){
                deleteProject(projectGui,false);
            }
        }
    }
}

void AbstractWorkspaceGui::onUnloadAllProjectsExceptSelected()
{
    QAction* trigger = qobject_cast<QAction*>(sender());
    if(trigger != nullptr){
        QList<QSharedPointer<ProjectGui> > projectGuis = trigger->data().value<QList<QSharedPointer<ProjectGui>>>();
        for(QSharedPointer<ProjectGui> projectGui : _projectListDockWidget->projectGuis()){
            if(!projectGuis.contains(projectGui) && projectGui->isLoaded()){
                projectGui->unload();
            }
        }
    }
}

void AbstractWorkspaceGui::onEnableFastLoadForSelected(bool enable)
{
    QAction* trigger = qobject_cast<QAction*>(sender());
    if(trigger != nullptr){
        for(QSharedPointer<ProjectGui> projectGui : trigger->data().value<QList<QSharedPointer<ProjectGui>>>()){
            projectGui->onFastLoad(enable);
        }
    }
}

void AbstractWorkspaceGui::onUnloadAllProjects()
{
    // Unload all projects (without saving) that includes delete all ProjectGui ItemView
    unloadProjects(_projectListDockWidget->projectGuis());
}

void AbstractWorkspaceGui::projectGuiLabelChanged(QSharedPointer<ProjectGui> projectGui)
{
    if (_isOpen) {
        for (QListWidgetItem* item : _projectListDockWidget->listWidgetItems()) {
            QSharedPointer<ProjectGui> projectGuiFromListWidget = item->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>();

            if (projectGuiFromListWidget == projectGui) {
                item->setText(projectGui->projectGuiLabel());
            }
        }

        int projectGuiTabIndex = _itemViewTabWidget->indexOf(projectGui->itemView());
        _itemViewTabWidget->setTabText(projectGuiTabIndex, projectGui->projectGuiLabel());
    }
}


void AbstractWorkspaceGui::deleteWorkspace(const QSharedPointer<AbstractWorkspace>& workspace)
{
    int ret = QMessageBox::question(0, tr("Delete Workspace."), tr("asdf"), QMessageBox::Ok | QMessageBox::Cancel);

    if (ret == QMessageBox::Ok) {
        workspace->deleteWorkspace();
    }
}

// Dialog
// ---------------------------------------------------------------------------------------------------------------------

bool AbstractWorkspaceGui::showSaveReminder(const QVector<QSharedPointer<ProjectGui>>& dirtyProjects)
{
    if (!dirtyProjects.isEmpty()) {
        ProjectSaveReminderDialog projectSaveReminder(dirtyProjects);

        int ret = projectSaveReminder.exec();

        switch (ret) {
        case QDialog::Accepted:
            for (QSharedPointer<ProjectGui> projectGui : projectSaveReminder.projectGuiSaveSelection()) {
                projectGui->save();
            }

            for (QSharedPointer<ProjectGui> projectGui : projectSaveReminder.projectGuiDiscardSelection()) {
                projectGui->reset();
            }

            break;

        default:
            return false;
        }
    }

    return true;
}

QMenu* AbstractWorkspaceGui::switchWorkspaceMenu() const
{
    return _switchWorkspaceMenu;
}

void AbstractWorkspaceGui::setSwitchWorkspaceMenu(QMenu *switchWorkspaceMenu)
{
    _switchWorkspaceMenu = switchWorkspaceMenu;
}

QSharedPointer<ProjectGui> AbstractWorkspaceGui::projectGui(const QSharedPointer<AbstractProject> &project)
{
    for (QListWidgetItem* item : _projectListDockWidget->listWidgetItems()) {
        QSharedPointer<ProjectGui> projectGuiFromListWidget = item->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>();
        if(projectGuiFromListWidget.isNull()){
            continue;
        }

        if (projectGuiFromListWidget->project() == project) {
            return projectGuiFromListWidget;
        }
    }
    return QSharedPointer<ProjectGui>();
}
