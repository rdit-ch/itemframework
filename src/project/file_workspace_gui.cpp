#include "file_workspace_gui.h"
#include "file_workspace_new_dialog.h"
#include "file_project_new_dialog.h"
#include "file_project_load_dialog.h"
#include "file_workspace_load_dialog.h"
#include "file_workspace_edit_dialog.h"
#include "item/item_view.h"
#include "project_manager_config.h"
#include <QAction>
#include <QMenu>

FileWorkspaceGui::FileWorkspaceGui()
{
    // Create a new Project Action
    QAction* newProject = Gui_Manager::instance()->get_action("New Project");
    // Open an existing Project Action
    QAction* openProject = Gui_Manager::instance()->get_action("Open Project");
    QAction* saveProject = Gui_Manager::instance()->get_action("Save Project");
    QAction* saveAllProjects = Gui_Manager::instance()->get_action("Save all Projects ...");
    // Connect actions
    connect(newProject, &QAction::triggered, this, &FileWorkspaceGui::showNewProjectDialog);
    connect(openProject, &QAction::triggered, this, &FileWorkspaceGui::showOpenProjectDialog);
    connect(saveProject, &QAction::triggered, this, &AbstractWorkspaceGui::onSaveProject);
    connect(saveAllProjects, &QAction::triggered, this, &AbstractWorkspaceGui::onSaveAllProjects);
    connect(this, &AbstractWorkspaceGui::showContextMenu, this, &FileWorkspaceGui::customContextMenuRequested);
}

FileWorkspaceGui::~FileWorkspaceGui()
{
}

QMenu* FileWorkspaceGui::createWorkspaceContextMenue()
{
    QMenu* contextMenue = new QMenu();
    QAction* newProject = new QAction(QString("Add New Project..."), 0);
    QAction* addProject = new QAction(QString("Add Existing Project..."), 0);
    QAction* saveAllProjects = new QAction(QString("Save All"), 0);
    QAction* unloadAllProjects = new QAction(QString("Close All"), 0);
    QAction* workspaceInfo = new QAction(QString("Workspace Info"), 0);

    connect(newProject, &QAction::triggered, this, &FileWorkspaceGui::showNewProjectDialog);
    connect(addProject, &QAction::triggered, this, &FileWorkspaceGui::showOpenProjectDialog);
    connect(saveAllProjects, &QAction::triggered, this, &FileWorkspaceGui::onSaveAllProjects);
    connect(unloadAllProjects, &QAction::triggered, this, &FileWorkspaceGui::onUnloadAllProjects);
    connect(workspaceInfo, &QAction::triggered, this, &FileWorkspaceGui::showWorkspaceInformation);

    contextMenue->addAction(newProject);
    contextMenue->addAction(addProject);
    contextMenue->addAction(contextMenue->addSeparator());
    contextMenue->addAction(saveAllProjects);
    contextMenue->addAction(unloadAllProjects);
    contextMenue->addAction(contextMenue->addSeparator());
    contextMenue->addAction(workspaceInfo);
    return contextMenue;
}

QString FileWorkspaceGui::workspaceTypeName() const
{
    // Return the workspace name to use it for e.g combobox, label, etc.
    return tr("File Workspace");
}

QDialog* FileWorkspaceGui::dialogNewWorkspace(QDialog* parent) const
{
    QDialog* dialogNewWorkspace = new FileWorkspaceNewDialog(_lastUsedWorkspacePath, parent);
    connect(dialogNewWorkspace, &QDialog::accepted, this, &FileWorkspaceGui::newWorkspaceAccepted);
    // Return a QDialog pointer. This widget provides an UI to create a new workspace
    return dialogNewWorkspace;
}

QDialog* FileWorkspaceGui::dialogLoadWorkspace(QDialog* parent) const
{
    // Return a fileDialog-LoadWorkspace widget pointer.  This widget provides an UI to load workspace.
    QDialog* dialogLoadWorkspace = new FileWorkspaceLoadDialog(_lastUsedWorkspacePath, parent);
    connect(dialogLoadWorkspace, &QDialog::accepted, this, &FileWorkspaceGui::loadWorkspaceAccepted);
    // Return a QDialog pointer. This widget provides an UI to create a new workspace
    return dialogLoadWorkspace;
}

QDialog* FileWorkspaceGui::dialogEditWorkspace(QDialog* parent, QSharedPointer<AbstractWorkspace> workspace) const
{
    FileWorkspaceEditDialog* fileWorkspaceEditDialog = new FileWorkspaceEditDialog(_lastUsedWorkspacePath, parent);
    fileWorkspaceEditDialog->setWorkspace(workspace);
    connect(fileWorkspaceEditDialog, &QDialog::accepted, this, &FileWorkspaceGui::editWorkspaceAccepted);
    return fileWorkspaceEditDialog;
}

void FileWorkspaceGui::addListWidgetItem(QListWidget* parentListWidget)
{
    QListWidgetItem* listWidgetItem = new QListWidgetItem(parentListWidget);
    listWidgetItem->setText(QStringLiteral("File"));
    listWidgetItem->setIcon(QIcon(":/core/projectmanager/localdrive"));
}

bool FileWorkspaceGui::isTypeFriendly(const QSharedPointer<AbstractWorkspace>& workspace) const
{
    if (qSharedPointerCast<FileWorkspace>(workspace).isNull()) {
        return false;
    }

    return true;
}

bool FileWorkspaceGui::removeProject(const QSharedPointer<ProjectGui>& projectGui)
{
    bool remove = false;
    int ret = QMessageBox::question(0, tr("Remove Project from Workspace."),
                                    tr("Do you want to remove this Project?\n%1").arg(projectGui->project()->name()),
                                    QMessageBox::Ok | QMessageBox::Cancel);

    if (ret == QMessageBox::Ok) {
        remove = true;

        if (projectGui->isLoaded()) {
            if (!projectGui->unload()) {
                remove = false;
            }
        }

        if (remove) {
            _projectListDockWidget->removeProject(projectGui);
            workspace()->removeProject(projectGui->project());
        }
    }

    return remove;
}

bool FileWorkspaceGui::deleteProject(const QSharedPointer<ProjectGui>& projectGui)
{
    bool remove = false;
    int ret = QMessageBox::question(0, tr("Delete Project."),
                                    tr("Do you want to delete this Project?\n%1").arg(projectGui->project()->name()),
                                    QMessageBox::Ok | QMessageBox::Cancel);

    if (ret == QMessageBox::Ok) {
        remove = true;

        if (projectGui->isLoaded()) {
            if (!projectGui->unload()) {
                remove = false;
            }
        }

        if (remove) {
            _projectListDockWidget->removeProject(projectGui);
            workspace()->deleteProject(projectGui->project());
        }
    }

    return remove;
}

bool FileWorkspaceGui::saveExternChangedProjects()
{
    return workspace()->saveExternChangedProjects();
}

void FileWorkspaceGui::resetExternChangedProjects()
{
    return workspace()->resetExternChangedProjects();
}

void FileWorkspaceGui::customContextMenuRequested(const QPoint& position)
{
    QMenu* contextMenu = createWorkspaceContextMenue();
    contextMenu->exec(position);
}

void FileWorkspaceGui::loadWorkspaceAccepted()
{
    FileWorkspaceLoadDialog* fileWorkspaceLoadDialog = qobject_cast<FileWorkspaceLoadDialog*>(sender());

    if (fileWorkspaceLoadDialog == nullptr) {
        return;
    }

    fileWorkspaceLoadDialog->setVisible(true);
    QStringList workspaceFile = fileWorkspaceLoadDialog->selectedFiles();

    if (!workspaceFile.isEmpty()) {
        const QString workspaceFilePath = workspaceFile.first();
        // Create new file workspace with prefered filename and workspacename
        QSharedPointer<FileWorkspace> fileWorkspace = FileWorkspace::createFileWorkspaceFromFile(workspaceFilePath);

        if (fileWorkspace.isNull()) {
            return;
        }

        _lastUsedWorkspacePath = fileWorkspace->directory();

        if (!fileWorkspace->isValid()) {
            const QString workspaceErrorMessage = tr("Workspace is invalid.\n\nConnection: \n%1\n\nError:\n%2")
                                                  .arg(fileWorkspace->connectionString())
                                                  .arg(fileWorkspace->lastError());

            QMessageBox::warning(0, tr("Loading workspace."), workspaceErrorMessage, QMessageBox::Ok);
            return;
        }

        // Emit acceptWorkspace signal to send this workspace to the select workspace dialog
        emit acceptWorkspace(fileWorkspace);
    }
}

void FileWorkspaceGui::newWorkspaceAccepted()
{
    FileWorkspaceNewDialog* fileWorkspaceNewDialog = qobject_cast<FileWorkspaceNewDialog*>(sender());

    if (fileWorkspaceNewDialog == nullptr) {
        return;
    }

    const FileWorkspaceData* workspaceProperties = fileWorkspaceNewDialog->data();
    // Create new file workspace with prefered filename and workspacename
    QSharedPointer<FileWorkspace> fileWorkspace = newFileWorkspace(workspaceProperties);

    if (fileWorkspace.isNull()) {
        return;
    }

    _lastUsedWorkspacePath = fileWorkspace->directory();

    if (!fileWorkspace->isValid()) {
        const QString workspaceErrorMessage = tr("%1.\n\nConnection: \n%2")
                                              .arg(fileWorkspace->lastError())
                                              .arg(fileWorkspace->connectionString());
        QMessageBox::warning(0, tr("Loading workspace."), workspaceErrorMessage, QMessageBox::Ok);
        return;
    }

    // Emit acceptWorkspace signal to send this workspace to the select workspace dialog
    emit acceptWorkspace(fileWorkspace);
}

void FileWorkspaceGui::editWorkspaceAccepted()
{
    FileWorkspaceEditDialog* fileWorkspaceEditDialog = qobject_cast<FileWorkspaceEditDialog*>(sender());

    if (fileWorkspaceEditDialog == nullptr) {
        return;
    }

    const FileWorkspaceData* workspacePropertiesInitial = fileWorkspaceEditDialog->workspacePropertiesInitial();
    const FileWorkspaceData* workspacePropertiesEdited = fileWorkspaceEditDialog->workspacePropertiesEdited();
    bool workspaceIsPartOfRecentList = false;

    for (const QSharedPointer<AbstractWorkspace> abstractWorkspace : projectManager()->recentWorkspaces()) {

        QSharedPointer<FileWorkspace> fileWorkspace = qSharedPointerCast<FileWorkspace>(abstractWorkspace);

        if (fileWorkspace.isNull()) {
            continue;
        }

        if (fileWorkspace->absoluteFilePath() == workspacePropertiesInitial->absoluteFilePath) {

            fileWorkspace->setName(workspacePropertiesEdited->name);
            fileWorkspace->setAbsoluteFilePath(workspacePropertiesEdited->absoluteFilePath);
            fileWorkspace->setDescription(workspacePropertiesEdited->description);
            workspaceIsPartOfRecentList = true;

            if (!fileWorkspace->save()) {
                //qDebug() << fileWorkspace->lastError();
            }
        }
    }
}

void FileWorkspaceGui::showNewProjectDialog()
{
    QSharedPointer<FileWorkspace> fileWorkspace = workspaceTypeCast<FileWorkspace>(workspace());

    if (fileWorkspace.isNull()) {
        return;
    }

    FileProjectNewDialog* fileProjectNewDialog = new FileProjectNewDialog(fileWorkspace->directory());

    if (fileProjectNewDialog->exec() == QDialog::Accepted) {

        FileProjectData* projectProperties = fileProjectNewDialog->data();
        const QString projectName = projectProperties->name;
        const QString projectPath = projectProperties->filePath;
        const QString projectDescription = projectProperties->description;
        const bool fastLoad = projectProperties->fastLoad;

        QDomDocument projectDomDocument = AbstractProject::projectDomDocumentTemplate(projectName,
                                          FileProVersion,
                                          projectDescription);

        QSharedPointer<AbstractProject> project = QSharedPointer<AbstractProject>(new FileProject(workspace()->settingsScope(),
                projectPath,
                projectDomDocument));

        project->setFastLoad(fastLoad);
        project->save();
        fileWorkspace->addProject(project);
        addProjectGui(QSharedPointer<ProjectGui>(new ProjectGui(this, project)));
    }
}

void FileWorkspaceGui::showOpenProjectDialog()
{
    QSharedPointer<FileWorkspace> fileWorkspace = workspaceTypeCast<FileWorkspace>(workspace());

    if (fileWorkspace.isNull()) {
        return;
    }

    FileProjectLoadDialog* fileProjectLoadDialog = new FileProjectLoadDialog(fileWorkspace->directory());

    if (fileProjectLoadDialog->exec() == QDialog::Accepted) {

        const QStringList projectFiles = fileProjectLoadDialog->selectedFiles();

        for (const QString projectPath : projectFiles) {
            QDomDocument projectDomDocument = FileHelper::domDocumentFromXMLFile(projectPath);
            QSharedPointer<AbstractProject> project = QSharedPointer<AbstractProject>(new FileProject(workspace()->settingsScope(),
                    projectPath,
                    projectDomDocument));

            if (fileWorkspace->addProject(project)) {
                addProjectGui(QSharedPointer<ProjectGui>(new ProjectGui(this, project)));
            }
        }
    }
}

void FileWorkspaceGui::showWorkspaceInformation()
{
    const QString workspaceInformation = tr("Name: %1\n\nFile: %2\n\nProject count: %3\n\nDescription: \n%4")
                                         .arg(workspace()->name())
                                         .arg(workspace()->connectionString())
                                         .arg(workspace()->projectCount())
                                         .arg(workspace()->description());
    QMessageBox::information(0, tr("Workspace Information"), workspaceInformation, QMessageBox::Ok);
}

QSharedPointer<FileWorkspace> FileWorkspaceGui::newFileWorkspace(const FileWorkspaceData* workspaceProperties)
{
    QSharedPointer<FileWorkspace> fileWorkspace;

    if (workspaceProperties != nullptr) {
        fileWorkspace = QSharedPointer<FileWorkspace>(new FileWorkspace);
        const QString absFilePath = workspaceProperties->directory + Slash + workspaceProperties->file;
        fileWorkspace->setVersion(FileWspVersion);
        fileWorkspace->setAbsoluteFilePath(absFilePath);
        fileWorkspace->setName(workspaceProperties->name);
        fileWorkspace->setDescription(workspaceProperties->description);
        fileWorkspace->setDefault(workspaceProperties->isDefault);

        if (!fileWorkspace->save()) {
            //qDebug() << fileWorkspace->lastError();
        }

        fileWorkspace->init();
    }

    return fileWorkspace;
}

void FileWorkspaceGui::onProjectLoad(const QSharedPointer<ProjectGui>& projectGui)
{
    AbstractWorkspaceGui::onProjectLoad(projectGui);
}

void FileWorkspaceGui::onProjectUnload(const QSharedPointer<ProjectGui>& projectGui)
{
    AbstractWorkspaceGui::onProjectUnload(projectGui);
}
