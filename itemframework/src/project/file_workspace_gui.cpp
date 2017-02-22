#include "file_workspace_gui.h"
#include "file_workspace_new_dialog.h"
#include "file_project_new_dialog.h"
#include "file_project_load_dialog.h"
#include "file_project_edit_dialog.h"
#include "file_workspace_load_dialog.h"
#include "file_workspace_edit_dialog.h"
#include "item/item_view.h"
#include "wizard_workspace_export.h"
#include "wizard_workspace_import.h"
#include <QAction>
#include <QMenu>

FileWorkspaceGui::FileWorkspaceGui()
{
    // Create a new Project Action
    QAction* newProject = GuiManager::instance()->action("newProject");
    // Open an existing Project Action
    QAction* openProject = GuiManager::instance()->action("openProject");
    QAction* saveProject = GuiManager::instance()->action("saveProject");
    QAction* saveAllProjects = GuiManager::instance()->action("saveAllProjects");
    // Connect actions
    connect(newProject, &QAction::triggered, this, &FileWorkspaceGui::showNewProjectDialog);
    connect(openProject, &QAction::triggered, this, &FileWorkspaceGui::showOpenProjectDialog);
    connect(saveProject, &QAction::triggered, this, &AbstractWorkspaceGui::onSaveProject);
    connect(saveAllProjects, &QAction::triggered, this, &AbstractWorkspaceGui::onSaveAllProjects);
    connect(this, &AbstractWorkspaceGui::showContextMenu, this, &FileWorkspaceGui::workspaceMenuRequested);
}

FileWorkspaceGui::~FileWorkspaceGui()
{
}

QString FileWorkspaceGui::workspaceTypeName() const
{
    // Return the workspace name to use it for e.g combobox, label, etc.
    return tr("File Workspace");
}

bool FileWorkspaceGui::isTypeFriendly(const QSharedPointer<AbstractWorkspace>& workspace) const
{
    return !qSharedPointerCast<FileWorkspace>(workspace).isNull();
}

void FileWorkspaceGui::addListWidgetItem(QListWidget* parentListWidget)
{
    QListWidgetItem* listWidgetItem = new QListWidgetItem(parentListWidget);
    listWidgetItem->setText(tr("File"));
    listWidgetItem->setIcon(QIcon(":/core/projectmanager/localdrive"));
}

QDialog* FileWorkspaceGui::dialogNewWorkspace(QDialog* parent)
{
    FileWorkspaceNewDialog* fileWorkspaceNewDialog = new FileWorkspaceNewDialog(lastUsedWorkspacePath(), parent);

    connect(fileWorkspaceNewDialog, &QDialog::accepted, [this, fileWorkspaceNewDialog](){
        const FileWorkspaceData* workspaceData = fileWorkspaceNewDialog->data();

        if(!FileHelper::directoryExists(workspaceData->directory)){
            if(!FileHelper::createDirectory(workspaceData->directory)){
                QMessageBox::warning(0, tr("Create new workspace."), FileHelper::lastError(), QMessageBox::Ok);
                return;
            }
        }

        // Create new file workspace with prefered filename and workspacename
        auto fileWorkspace = newFileWorkspace(workspaceData);
        setLastUsedWorkspacePath(fileWorkspace->path());

        if (!fileWorkspace->isValid()) {
            const QString workspaceErrorMessage = tr("%1.\n\nConnection: \n%2")
                                                  .arg(fileWorkspace->lastError())
                                                  .arg(fileWorkspace->connectionString());
            QMessageBox::warning(0, tr("Loading workspace."), workspaceErrorMessage, QMessageBox::Ok);
            return;
        }

        // Emit acceptWorkspace signal to send this workspace to the select workspace dialog
        emit acceptWorkspace(fileWorkspace);

    });
    // Return a QDialog pointer. This widget provides an UI to create a new workspace
    return fileWorkspaceNewDialog;
}

QDialog* FileWorkspaceGui::dialogLoadWorkspace(QDialog* parent)
{
    // Return a fileDialog-LoadWorkspace widget pointer.  This widget provides an UI to load workspace.
    FileWorkspaceLoadDialog* fileWorkspaceLoadDialog = new FileWorkspaceLoadDialog(lastUsedWorkspacePath(), parent);

    connect(fileWorkspaceLoadDialog, &QDialog::accepted, [this, fileWorkspaceLoadDialog](){
        fileWorkspaceLoadDialog->setVisible(true);
        QStringList workspaceFile = fileWorkspaceLoadDialog->selectedFiles();

        if (!workspaceFile.isEmpty()) {
            // Create new file workspace with prefered filename and workspacename
            auto fileWorkspace = FileWorkspace::createFileWorkspaceFromFile(workspaceFile.first());
            setLastUsedWorkspacePath(fileWorkspace->path());

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
    });

    // Return a QDialog pointer. This widget provides an UI to create a new workspace
    return fileWorkspaceLoadDialog;
}

QDialog* FileWorkspaceGui::dialogEditWorkspace(QDialog* parent, QSharedPointer<AbstractWorkspace> workspace) const
{
    FileWorkspaceEditDialog* fileWorkspaceEditDialog = new FileWorkspaceEditDialog(lastUsedWorkspacePath(), parent);
    fileWorkspaceEditDialog->setWorkspace(workspace);
    bool workspaceIsNull = workspace.isNull();

    connect(fileWorkspaceEditDialog, &QDialog::accepted, [this, fileWorkspaceEditDialog, workspaceIsNull](){
        const FileWorkspaceData* workspacePropertiesInitial = fileWorkspaceEditDialog->workspacePropertiesInitial();
        const FileWorkspaceData* workspacePropertiesEdited = fileWorkspaceEditDialog->workspacePropertiesEdited();

        QSharedPointer<FileWorkspace> fileWorkspace = fileWorkspaceEditDialog->workspace();

        if(workspaceIsNull){
            // Edit workspace called by edit button.
            // Search for this workspace in recent workspace list to get the right shared pointer object.
            for(QSharedPointer<AbstractWorkspace> workspace : projectManager()->recentWorkspaces()){
                if(workspace->compare(fileWorkspace)){
                    fileWorkspace = qSharedPointerCast<FileWorkspace>(workspace);
                    break;
                }
            }
        }

        if(!fileWorkspace.isNull()){
             fileWorkspace->setFileEditMode(true);

            if(workspacePropertiesEdited->filePath != workspacePropertiesInitial->filePath){
                // Copy the workspace file to a new destination

                if(!FileHelper::directoryExists(workspacePropertiesEdited->directory)){
                    if(!FileHelper::createDirectory(workspacePropertiesEdited->directory)){
                        QMessageBox::warning(0, tr("Create new workspace."), FileHelper::lastError(), QMessageBox::Ok);
                        return;
                    }
                }

                QFile dstWorkspaceFile(workspacePropertiesEdited->filePath);
                QFile srcWorkspaceFile(workspacePropertiesInitial->filePath);



                if (dstWorkspaceFile.exists() && !dstWorkspaceFile.remove()) {
                    QMessageBox::warning(0, tr("Edit workspace file failed."),
                                                   QString("Destination file %1 exists and could not be removed.").arg(dstWorkspaceFile.fileName()),
                                                   QMessageBox::Ok,
                                                   QMessageBox::Ok);
                    return;
                }

                if (!srcWorkspaceFile.copy(dstWorkspaceFile.fileName())) {
                    QMessageBox::warning(0, tr("Edit workspace file failed."),
                                                   QString("Source file %1 could not be copied to destination file %2.").
                                                                          arg(srcWorkspaceFile.fileName()).
                                                                          arg(dstWorkspaceFile.fileName()),
                                                   QMessageBox::Ok,
                                                   QMessageBox::Ok);
                    return;
                }

                if (!srcWorkspaceFile.remove()) {
                    QMessageBox::warning(0, tr("Edit workspace file failed."),
                                                   QString("Source file %1 could not be removed.").arg(srcWorkspaceFile.fileName()),
                                                   QMessageBox::Ok,
                                                   QMessageBox::Ok);
                    return;
                }


                fileWorkspace->setFilePath(workspacePropertiesEdited->filePath);
            }

            fileWorkspace->setName(workspacePropertiesEdited->name);
            fileWorkspace->setDescription(workspacePropertiesEdited->description);
            fileWorkspace->update();
            fileWorkspace->save();
            fileWorkspace->setFileEditMode(false);
        }
    });

    return fileWorkspaceEditDialog;
}

bool FileWorkspaceGui::removeProject(const QSharedPointer<ProjectGui>& projectGui, bool showMessagebox)
{
    bool remove = false;
    if(showMessagebox){
        int ret = QMessageBox::question(0, tr("Remove Project from Workspace."),
                                        tr("Do you want to remove this Project?\n%1").arg(projectGui->project()->name()),
                                        QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            return remove;
        }
    }

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
    return remove;
}

bool FileWorkspaceGui::deleteProject(const QSharedPointer<ProjectGui>& projectGui, bool showMessagebox)
{
    bool remove = false;
    if(showMessagebox){
        int ret = QMessageBox::question(0, tr("Delete Project."),
                                        tr("Do you want to delete this Project?\n%1").arg(projectGui->project()->name()),
                                        QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            return remove;
        }
    }

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

void FileWorkspaceGui::showChangeSourceDialog(const QSharedPointer<AbstractWorkspace> &workspace)
{
    QSharedPointer<FileWorkspace> fileWorkspace = qSharedPointerCast<FileWorkspace>(workspace);
    QString fileName = QFileDialog::getOpenFileName(0, tr("Search Workspace File"),
                                                    fileWorkspace->path(),
                                                    tr("Traviz Workspace (*.twsp)"));

    if (!fileName.isEmpty()) {
        fileWorkspace->setFilePath(fileName);
        fileWorkspace->init();
        fileWorkspace->save();
    }
}

void FileWorkspaceGui::showExportWorkspaceWizard()
{
    WizardWorkspaceExport wizardWorkspaceExport(this);
    wizardWorkspaceExport.exec();
}

void FileWorkspaceGui::showImportWorkspaceWizard()
{
    WizardWorkspaceImport wizardWorkspaceImport(this);
    wizardWorkspaceImport.exec();
}

void FileWorkspaceGui::editProject(QSharedPointer<ProjectGui> projectGui)
{
    QSharedPointer<FileProject> fileProject = qSharedPointerCast<FileProject>(projectGui->project());

    if(!fileProject.isNull()){

        FileProjectEditDialog editDialog(fileProject);
        if(editDialog.exec() == QDialog::Accepted){

            QSharedPointer<FileWorkspace> fileWorkspace = qSharedPointerCast<FileWorkspace>(workspace());
            const FileProjectData* projectPropertiesInitial = editDialog.projectPropertiesInitial();
            const FileProjectData* projectPropertiesEdited = editDialog.projectPropertiesEdited();

            QString srcFilePath = projectPropertiesInitial->filePath;
            QString dstFilePath = projectPropertiesEdited->filePath;

            fileProject->setName(projectPropertiesEdited->name);
            fileProject->setDescription(projectPropertiesEdited->description);

            if(srcFilePath != dstFilePath){
                QFile dstProjectFile(projectPropertiesEdited->filePath);
                QFile srcProjectFile(projectPropertiesInitial->filePath);

                if (dstProjectFile.exists() && !dstProjectFile.remove()) {
                    return;
                }

                if (!srcProjectFile.copy(dstProjectFile.fileName())) {
                    return;
                }

                if (!srcProjectFile.remove()) {
                    return;
                }

                fileProject->setFilePath(projectPropertiesEdited->filePath);
            }

            projectGui->save();

            if(!fileWorkspace.isNull() && fileWorkspace->contains(fileProject)){
                fileWorkspace->save();
            }
        }
    }
}

void FileWorkspaceGui::workspaceMenuRequested(const QPoint& position)
{
    QMenu contextMenu;
    QAction* newProject = contextMenu.addAction(tr("New Project..."));
    QAction* addProject = contextMenu.addAction(tr("Add Existing Project..."));
    QAction* editWorkspace = contextMenu.addAction(tr("Edit Workspace \"%1\"").arg(workspace()->name()));
    contextMenu.addSeparator();
    QAction* switchWorkspace = contextMenu.addMenu(switchWorkspaceMenu());
    contextMenu.addSeparator();
    QAction* importProjects = contextMenu.addAction(tr("Import..."));
    QAction* exportWorkspace = contextMenu.addAction(tr("Export..."));
    contextMenu.addSeparator();
    QAction* saveAllProjects = contextMenu.addAction(tr("Save All Projects"));
    QAction* unloadAllProjects = contextMenu.addAction(tr("Close All Projects"));
    contextMenu.addSeparator();
    QAction* workspaceInfo = contextMenu.addAction(tr("Workspace Info"));

    connect(newProject, &QAction::triggered, this, &FileWorkspaceGui::showNewProjectDialog);
    connect(addProject, &QAction::triggered, this, &FileWorkspaceGui::showOpenProjectDialog);
    connect(saveAllProjects, &QAction::triggered, this, &FileWorkspaceGui::onSaveAllProjects);
    connect(unloadAllProjects, &QAction::triggered, this, &FileWorkspaceGui::onUnloadAllProjects);
    connect(workspaceInfo, &QAction::triggered, this, &FileWorkspaceGui::showWorkspaceInformation);
    connect(importProjects, &QAction::triggered, this, &FileWorkspaceGui::showImportWorkspaceWizard);
    connect(exportWorkspace, &QAction::triggered, this, &FileWorkspaceGui::showExportWorkspaceWizard);


    connect(editWorkspace, &QAction::triggered, [this](){
        QDialog* editWorkspaceDialog = dialogEditWorkspace(0, workspace());
        editWorkspaceDialog->exec();
    });

    //connect(switchWorkspace, &QAction::triggered, this, &AbstractWorkspaceGui::switchWorkspace);
    contextMenu.exec(position);
}

void FileWorkspaceGui::showNewProjectDialog()
{
    QSharedPointer<FileWorkspace> fileWorkspace = workspaceTypeCast<FileWorkspace>(workspace());

    if (fileWorkspace.isNull()) {
        return;
    }

    FileProjectNewDialog* fileProjectNewDialog = new FileProjectNewDialog(fileWorkspace->path());

    if (fileProjectNewDialog->exec() == QDialog::Accepted) {

        FileProjectData* projectProperties = fileProjectNewDialog->data();
        const QString projectName = projectProperties->name;
        const QString projectPath = projectProperties->filePath;
        const QString projectDescription = projectProperties->description;
        const bool fastLoad = projectProperties->fastLoad;

        QDomDocument projectDomDocument = AbstractProject::projectDomDocumentTemplate(projectName,
                                          FileProVersion,
                                          projectDescription);

        QSharedPointer<FileProject> fileProject = QSharedPointer<FileProject>(new FileProject(workspace()->settingsScope(),
                                                                        projectPath,
                                                                        projectDomDocument));
        fileProject->setFastLoad(fastLoad);
        fileProject->setRelativFilePath(FileHelper::absoluteToRelativeFilePath(fileProject->filePath(), fileWorkspace->filePath()));
        fileProject->save();
        fileWorkspace->addProject(fileProject);
        addProjectGui(QSharedPointer<ProjectGui>(new ProjectGui(this, fileProject)));
        fileProject->autosave();
    }
}

void FileWorkspaceGui::showOpenProjectDialog()
{
    QSharedPointer<FileWorkspace> fileWorkspace = workspaceTypeCast<FileWorkspace>(workspace());

    if (fileWorkspace.isNull()) {
        return;
    }

    FileProjectLoadDialog* fileProjectLoadDialog = new FileProjectLoadDialog(fileWorkspace->path());

    if (fileProjectLoadDialog->exec() == QDialog::Accepted) {

        const QStringList projectFiles = fileProjectLoadDialog->selectedFiles();

        for (const QString projectPath : projectFiles) {
            QDomDocument projectDomDocument = FileHelper::domDocumentFromXMLFile(projectPath);
            QSharedPointer<FileProject> fileProject = QSharedPointer<FileProject>(new FileProject(workspace()->settingsScope(),
                    projectPath,
                    projectDomDocument));

            if (fileWorkspace->addProject(fileProject)) {
                addProjectGui(QSharedPointer<ProjectGui>(new ProjectGui(this, fileProject)));
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
    QSharedPointer<FileWorkspace> fileWorkspace = QSharedPointer<FileWorkspace>(new FileWorkspace);
    fileWorkspace->setVersion(FileWspVersion);
    fileWorkspace->setFilePath(workspaceProperties->filePath);
    fileWorkspace->setName(workspaceProperties->name);
    fileWorkspace->setDescription(workspaceProperties->description);
    fileWorkspace->setDefault(workspaceProperties->isDefault);
    fileWorkspace->save();
    fileWorkspace->init();
    return fileWorkspace;
}

QString FileWorkspaceGui::lastUsedWorkspacePath() const
{
    return _lastUsedWorkspacePath;
}

void FileWorkspaceGui::setLastUsedWorkspacePath(const QString &lastUsedWorkspacePath)
{
    if(lastUsedWorkspacePath.isEmpty()){
        const auto fileWorkspace = qSharedPointerCast<FileWorkspace>(_projectManager->currentWorkspace());
        if(!fileWorkspace.isNull()){
            _lastUsedWorkspacePath = fileWorkspace->path();
        } else {
            _lastUsedWorkspacePath = DefaultWorkspaceFolder;
        }
    }

    if(lastUsedWorkspacePath == _lastUsedWorkspacePath){
        return;
    }

    _lastUsedWorkspacePath = lastUsedWorkspacePath;
}

void FileWorkspaceGui::onProjectLoad(const QSharedPointer<ProjectGui>& projectGui)
{
    AbstractWorkspaceGui::onProjectLoad(projectGui);
}

void FileWorkspaceGui::onProjectUnload(const QSharedPointer<ProjectGui>& projectGui)
{
    AbstractWorkspaceGui::onProjectUnload(projectGui);
}

bool FileWorkspaceGui::exportToFileSystem(QString exportPath, ExportOptions options, QVector<QSharedPointer<AbstractProject>> projects)
{
    const auto fileWorkspace = qSharedPointerCast<FileWorkspace>(workspace());
    int workspaceOverwrite = -1;
    int generalOverwrite = -1;

    QString finalExportPath = exportPath;
    if(options.createFolderStructure){
        finalExportPath.append(QString("/%1").arg(fileWorkspace->name()));
    }

    QDir exportDir(finalExportPath);
    if (!exportDir.exists()) {
        if(!exportDir.mkpath(".")){
            return false;
        }
    }

    QFileInfo exportFileInfo(QString("%1/%2").arg(finalExportPath).arg(fileWorkspace->fileName()));
    if(!options.overwriteExistingFiles){

        if(exportFileInfo.exists()){
            QString message = QString("Overwrite \"%1\" in folder \"%2\" ?").arg(exportFileInfo.fileName()).arg(exportFileInfo.absoluteDir().path());
            int ret = QMessageBox::information(0, tr("Overwrite File"),
                                           message,
                                           QMessageBox::YesAll | QMessageBox::No | QMessageBox::NoAll | QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Yes);
            switch (ret) {
            case QMessageBox::YesAll:
                generalOverwrite = 1;
                break;
            case QMessageBox::No:
                workspaceOverwrite = 0;
                break;
            case QMessageBox::NoAll:
                generalOverwrite = 0;
                break;
            case QMessageBox::Cancel:
                return false;
            case QMessageBox::Yes:
                workspaceOverwrite = 1;
                break;
            }
        }
    }else {
        generalOverwrite = 1;
    }

    FileWorkspaceData workspaceProperties;
    workspaceProperties.directory = exportFileInfo.absolutePath();
    workspaceProperties.filePath = exportFileInfo.filePath();
    workspaceProperties.fileName = exportFileInfo.fileName();
    workspaceProperties.name = fileWorkspace->name();
    workspaceProperties.isDefault = fileWorkspace->isDefault();
    workspaceProperties.description = fileWorkspace->description();
    auto exportedFileWorkspace = newFileWorkspace(&workspaceProperties);

    if(workspaceOverwrite != 0 || generalOverwrite == 1){
        if(!exportedFileWorkspace->save()){
            return false;
        }
    }

    for(QSharedPointer<AbstractProject> project : projects){
        int projectOverwrite = 1;
        auto fileProject = qSharedPointerCast<FileProject>(project);
        QFileInfo projectFileInfo(fileProject->filePath());
        QFileInfo exportProjectFileInfo(QString("%1/%2").arg(finalExportPath).arg(projectFileInfo.fileName()));

        if(exportProjectFileInfo.exists() && generalOverwrite == -1){

            int ret = QMessageBox::information(0, tr("Overwrite File"),
                                           QString("Overwrite \"%1\" in folder \"%2\" ?").arg(exportProjectFileInfo.fileName()).arg(exportProjectFileInfo.absoluteDir().path()),
                                           QMessageBox::YesAll | QMessageBox::No | QMessageBox::NoAll | QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Yes);
            switch (ret) {
            case QMessageBox::YesAll:
                generalOverwrite = 1;
                break;
            case QMessageBox::No:
                projectOverwrite = 0;
                break;
            case QMessageBox::NoAll:
                generalOverwrite = 0;
                break;
            case QMessageBox::Cancel:
                continue;
            case QMessageBox::Yes:
                projectOverwrite = 1;
                break;
            }
        }

        if(generalOverwrite == 1 || projectOverwrite == 1){
            auto exportFileProject = QSharedPointer<FileProject>(new FileProject(exportedFileWorkspace->settingsScope()
                                                                      ,exportProjectFileInfo.absoluteFilePath()
                                                                      ,project->domDocument()));
            exportFileProject->save();
            exportFileProject->setRelativFilePath(FileHelper::absoluteToRelativeFilePath(exportProjectFileInfo.absoluteFilePath(), exportedFileWorkspace->filePath()));
            exportedFileWorkspace->addProject(exportFileProject);
        }
    }

    return exportedFileWorkspace->save();
}

void FileWorkspaceGui::searchProjectSource(QSharedPointer<ProjectGui> projectGui)
{
    QSharedPointer<FileProject> fileProject = qSharedPointerCast<FileProject>(projectGui->project());
    if(fileProject.isNull()){
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(0, tr("Search Project File"),
                                                    fileProject->path(),
                                                    tr("Traviz Project (*.tpro)"));

    if (!filePath.isEmpty()) {
        fileProject->setFilePath(filePath);
        workspace()->save();
        loadProject(projectGui);
    }
}

void FileWorkspaceGui::createProjectsFromImport(const QStringList &projectPaths, bool overwrite)
{
    QSharedPointer<FileWorkspace> fileWorkspace = workspaceTypeCast<FileWorkspace>(workspace());

    if (fileWorkspace.isNull()) {
        return;
    }

    int generalOverwrite = -1;
    if(overwrite){
        generalOverwrite = 1;
    }

    QStringList importedProjectInfo;
    for(QString projectPath : projectPaths){

        int projectOverwrite = 1;
        QFileInfo srcProjectFileInfo(projectPath);
        QFile srcProjectFile(projectPath);
        QFile dstProjectFile(QString("%1/%2").arg(fileWorkspace->path()).arg(srcProjectFileInfo.fileName()));

        if(srcProjectFile.fileName() == dstProjectFile.fileName()){
            // Project is in correct folder and can be added.
            QDomDocument projectDomDocument = FileHelper::domDocumentFromXMLFile(srcProjectFile.fileName());
            QSharedPointer<FileProject> fileProject = QSharedPointer<FileProject>(new FileProject(fileWorkspace->settingsScope(),
                    srcProjectFile.fileName(),
                    projectDomDocument));

            if (fileWorkspace->addProject(fileProject)) {
                addProjectGui(QSharedPointer<ProjectGui>(new ProjectGui(this, fileProject)));
                importedProjectInfo.append(fileProject->fileName());
            }
            // Next project
            continue;
        }

        if(dstProjectFile.exists() && generalOverwrite == -1){
            // Show overwrite message box, if file exists and overwrite all was not set.
            QString message = tr("Overwrite \"%1\" in folder \"%2\" ?").arg(dstProjectFile.fileName()).arg(fileWorkspace->path());
            int ret = QMessageBox::information(0, tr("Overwrite File"),
                                           message,
                                           QMessageBox::YesAll | QMessageBox::No | QMessageBox::NoAll | QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Yes);
            switch (ret) {
            case QMessageBox::YesAll:
                generalOverwrite = 1;
                break;
            case QMessageBox::No:
                projectOverwrite = 0;
                break;
            case QMessageBox::NoAll:
                generalOverwrite = 0;
                break;
            case QMessageBox::Cancel:
                return;
            case QMessageBox::Yes:
                projectOverwrite = 1;
                break;
            }
        }

        if(generalOverwrite == 1 || projectOverwrite == 1){
            QDomDocument projectDomDocument = FileHelper::domDocumentFromXMLFile(srcProjectFile.fileName());
            if(dstProjectFile.exists()){
                QSharedPointer<FileProject> fileProject = qSharedPointerCast<FileProject>(fileWorkspace->project(dstProjectFile.fileName()));
                if(!fileProject.isNull()){
                    // Project with the same name is alread present in workspace and will be overwritten.
                    QSharedPointer<ProjectGui> projectGuiPtr = projectGui(fileProject);
                    if(!projectGuiPtr.isNull() && unloadProject(projectGuiPtr,false)){
                        fileProject->setDomDocument(projectDomDocument);
                        fileProject->save();
                        loadProject(projectGuiPtr);
                        importedProjectInfo.append(fileProject->fileName());
                        continue;
                    }
                }
                if(!dstProjectFile.remove()){
                    QMessageBox::warning(0, tr("Error remove File"),
                                                tr("Could not remove file \"%1\"").arg(dstProjectFile.fileName()),
                                                QMessageBox::Ok,
                                                QMessageBox::Ok);
                    continue;
                }
            }

            if (!srcProjectFile.copy(dstProjectFile.fileName())) {
                QMessageBox::warning(0, tr("Import project file failed."),
                                               QString("Source file %1 could not be copied to destination file %2.").
                                                                      arg(srcProjectFile.fileName()).
                                                                      arg(dstProjectFile.fileName()),
                                               QMessageBox::Ok,
                                               QMessageBox::Ok);
                continue;
            }

            QSharedPointer<FileProject> fileProject = QSharedPointer<FileProject>(new FileProject(fileWorkspace->settingsScope(),
                    dstProjectFile.fileName(),
                    projectDomDocument));

            if (fileWorkspace->addProject(fileProject)) {
                addProjectGui(QSharedPointer<ProjectGui>(new ProjectGui(this, fileProject)));
                importedProjectInfo.append(fileProject->fileName());
            }
        }
    }

    QString message;

    if(importedProjectInfo.isEmpty()){
        message = tr("No Projects are imported.");
    } else {
        message = tr("Number of imported Projects: %1\n").arg(importedProjectInfo.count());
        for(QString projectInfo : importedProjectInfo){
            message.append(tr("\n%1").arg(projectInfo));
        }
    }

    QMessageBox::information(0, tr("Project import complete."),
                                   message,
                                   QMessageBox::Ok,
                                   QMessageBox::Ok);
}
