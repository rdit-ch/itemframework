#include "file_workspace.h"
#include "project_manager_config.h"
#include "file_project.h"

FileWorkspace::FileWorkspace(): AbstractWorkspace(tr("File"))
{
}

FileWorkspace::~FileWorkspace()
{
}

// ---------------------------------------------------------------------------------------------------------------------
// Workspace validate functions
// ---------------------------------------------------------------------------------------------------------------------

bool FileWorkspace::validateFileProperties()
{
    if (_fileInfo.filePath().isEmpty()) {
        return false;
    }

    if (!FileHelper::fileExists(_fileInfo.filePath())) {
        setLastError(FileHelper::lastError());
        return false;
    }

    if (!FileHelper::testFileOpenMode(_fileInfo.filePath(), QIODevice::ReadWrite)) {
        setLastError(FileHelper::lastError());
        return false;
    }

    clearLastError();
    return true;
}

void FileWorkspace::setFileEditMode(bool editMode)
{
    _editMode = editMode;
}

// ---------------------------------------------------------------------------------------------------------------------
// Workspace init
// ---------------------------------------------------------------------------------------------------------------------

void FileWorkspace::init()
{
    if(!validateFileProperties()){
        setValid(false);

        if(!_fileInfo.filePath().isEmpty()){
            setName(_fileInfo.baseName());
        }
        return;
    }

    const QDomDocument dom = FileHelper::domDocumentFromXMLFile(_fileInfo.filePath());

    if (!setWorkspaceProperties(dom)) {
        setValid(false);
        return;
    }

    setWorkspaceDomDocument(dom);
    initProjects();
    setValid(true);
    emit workspaceUpdated();
}

void FileWorkspace::initProjects()
{
    // Get root dom element (xml -> TravizWorkspace) of workspaceDomDocument
    const QDomElement rootElement = workspaceDomDocument().documentElement();
    // Get first project dom node (xml -> TravizProject) of workspaceDomDocument
    QDomElement projectElement = rootElement.firstChildElement(ProDomElmTagPro);

    // For each project dom node in workspaceDomDocument
    while (!projectElement.isNull()) {
        // Analyze project xml structure of workspace dom document
        // Get project dom element from node.


            bool fastLoad = false;

            // Check attribute fast load is available.
            if (projectElement.hasAttribute(WspDomElmProFastLoadAttLabel)) {
                // Set attribute fast load.
                fastLoad = QVariant(projectElement.attribute(WspDomElmProFastLoadAttLabel)).toBool();
            }

            // Check project has child nodes.
            if (!projectElement.hasChildNodes()) {
                // Project information incomplete (xml error).
                // No child nodes found. Ignore current project and go to next project dom node.
                projectElement = projectElement.nextSiblingElement(ProDomElmTagPro);
                continue;
            }

            // Set project file path dom element
            const QDomElement projectFilePathElement = projectElement.firstChild().toElement();

            if (projectFilePathElement.isNull()) {
                // Project information incomplete (xml error).
                // ProjectFilePath node is null Ignore current project and go to next project dom node.
                projectElement = projectElement.nextSiblingElement(ProDomElmTagPro);
                continue;
            }

            // Create project file path string object.
            QString projectFilePath;

            // Check ProjectFilePath node has attribute value.
            if (projectFilePathElement.hasAttribute(WspDomElmProPathAttLabel)) {
                // Set projectFilePath.
                projectFilePath = projectFilePathElement.attribute(WspDomElmProPathAttLabel);
            }

            // Check if projectFilePath is not empty.
            if (projectFilePath.isEmpty()) {
                // Project information incomplete (xml error).
                // ProjectFilePath node is null Ignore current project and go to next project dom node.
                projectElement = projectElement.nextSiblingElement(ProDomElmTagPro);
                continue;
            }

            // Analyze project xml structure was successfull.
            // Start creating project object QSharedPointer<Project> procedure.

            const QString absProjectFile = FileHelper::relativeToAbsoluteFilePath(projectFilePath, _fileInfo.filePath());

            // Create project dom document from xml project file (projectFilePath).
            const QDomDocument projectDomDocument = FileHelper::domDocumentFromXMLFile(absProjectFile);
            QSharedPointer<FileProject> project = QSharedPointer<FileProject>(new FileProject(settingsScope(), absProjectFile, projectDomDocument));
            project->setRelativFilePath(projectFilePath);
            // Set the fast load attribute.
            project->setFastLoad(fastLoad);
            // Add project to this workspace. Remeber that the project can be invalid.
            addProject(project);


        // Set next project dom node as current.
        projectElement = projectElement.nextSiblingElement(ProDomElmTagPro);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// Workspace update
// ---------------------------------------------------------------------------------------------------------------------

void FileWorkspace::update()
{
    if(!validateFileProperties()){
        setValid(false);
        if(!_fileInfo.filePath().isEmpty()){
            setName(_fileInfo.baseName());
        }
    } else {
        setValid(true);
        updateProjects();
    }

    emit workspaceUpdated();
}

void FileWorkspace::updateProjects()
{
    for(QSharedPointer<AbstractProject> project : projects()){
        QSharedPointer<FileProject> fileProject = qSharedPointerCast<FileProject>(project);
        if(!fileProject->isValid()){
            const QString absProjectFile = FileHelper::relativeToAbsoluteFilePath(fileProject->relativFilePath(), _fileInfo.filePath());
            fileProject->setFilePath(absProjectFile);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// Workspace test
// ---------------------------------------------------------------------------------------------------------------------

bool FileWorkspace::test()
{
    update();
    return isValid();
}

// ---------------------------------------------------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------------------------------------------------

bool FileWorkspace::save()
{
    bool isSaved = false;
    const QIODevice::OpenModeFlag openMode = QIODevice::WriteOnly;
    QFile file(_fileInfo.filePath());

    if (!FileHelper::fileExists(_fileInfo.filePath())) {
        setLastError(FileHelper::lastError());
        if (!file.open(openMode)) {

            if (!FileHelper::testFileOpenMode(_fileInfo.filePath(), openMode)) {
                setLastError(FileHelper::lastError());
            }
            return isSaved;
        }

        file.close();
    } else if (!FileHelper::testFileOpenMode(_fileInfo.filePath(), openMode)) {
        setLastError(FileHelper::lastError());
        return isSaved;
    }

    clearLastError();

    if (file.open(openMode)) {
        QDomDocument workspaceDomDocument = workspaceDomDocumentTemplate(name(), version(), description());
        QDomElement rootDomElement = workspaceDomDocument.documentElement();
        if(!settingsScope()->save(workspaceDomDocument,rootDomElement)){
            setLastError("Failed to save setting scope");

            return false;
        }
        for (const QSharedPointer<AbstractProject>& project : projects()) {
            const QSharedPointer<FileProject> fileProject = qSharedPointerCast<FileProject>(project);
            const QString relProjectFile = FileHelper::absoluteToRelativeFilePath(fileProject->filePath(), _fileInfo.filePath());

            QDomElement projectDomElement = workspaceDomDocument.createElement(ProDomElmTagPro);
            projectDomElement.setAttribute(WspDomElmProFastLoadAttLabel, QVariant(fileProject->isFastLoad()).toString());
            rootDomElement.appendChild(projectDomElement);

            QDomElement projectFilePathDomElement = workspaceDomDocument.createElement(WspDomElmTagProConnection);
            projectFilePathDomElement.setAttribute(WspDomElmProPathAttLabel, relProjectFile);
            projectDomElement.appendChild(projectFilePathDomElement);
        }

        // Write workspace file
        if (file.write(workspaceDomDocument.toByteArray()) > 0) {
            isSaved = true;
        }

        file.close();
    }

    return isSaved;
}

void FileWorkspace::setFilePath(const QString& filePath)
{
    // No file changes
    if (_fileInfo.filePath() == filePath) {
        return;
    }

    _fileInfo.setFile(filePath);
    setConnectionString(_fileInfo.filePath());

    if(isOpen() && !_fileSystemWatcher.files().contains(_fileInfo.filePath())){
        _fileSystemWatcher.addPath(_fileInfo.filePath());
    }
}

QString FileWorkspace::filePath() const
{
    return _fileInfo.filePath();
}

QString FileWorkspace::path() const
{
    return _fileInfo.path();
}

QString FileWorkspace::fileName() const
{
    return _fileInfo.fileName();
}

void FileWorkspace::setOpen(bool isOpen)
{
    if (isOpen) {
        connect(&_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &FileWorkspace::onWorkspaceFileChanged);
        _fileSystemWatcher.addPath(_fileInfo.filePath());
    } else {
        _fileSystemWatcher.disconnect(this);
    }

    AbstractWorkspace::setOpen(isOpen);
}

bool FileWorkspace::compare(const QSharedPointer<AbstractWorkspace>& otherWorkspace) const
{
    QSharedPointer<FileWorkspace> otherFileWorkspace = qSharedPointerCast<FileWorkspace>(otherWorkspace);

    if (!otherFileWorkspace.isNull()) {
        if (_fileInfo.filePath() == otherFileWorkspace->filePath()) {
            return true;
        }
    }

    return false;
}

QSharedPointer<FileWorkspace> FileWorkspace::createFileWorkspaceFromFile(const QString& workspaceFilePath)
{
    QSharedPointer<FileWorkspace> fileWorkspace = QSharedPointer<FileWorkspace>(new FileWorkspace);
    fileWorkspace->setFilePath(workspaceFilePath);
    fileWorkspace->init();
    return fileWorkspace;
}

bool FileWorkspace::deleteProject(const QSharedPointer<AbstractProject>& project)
{
    QSharedPointer<FileProject> fileProject = qSharedPointerCast<FileProject>(project);

    if (!FileHelper::removeFile(fileProject->filePath())) {
        setLastError(FileHelper::lastError());
    } else {
        clearLastError();
    }

    return removeProject(project);
}

bool FileWorkspace::deleteWorkspace(bool deleteProjects)
{
    if (deleteProjects) {
        // Going to delete all projects
        for (const QSharedPointer<AbstractProject>& project : projects()) {
            deleteProject(project);
        }
    }

    // Delete Workspace File
    if (!FileHelper::removeFile(_fileInfo.filePath())) {
        setLastError(FileHelper::lastError());
        return false;
    }

    clearLastError();
    return true;
}

void FileWorkspace::onWorkspaceFileChanged()
{
    if(!_fileInfo.exists() && !_editMode){
        // Workspace file was moved, this is just a simple solution.
        save();
    }

    _fileSystemWatcher.addPath(_fileInfo.filePath());
}
