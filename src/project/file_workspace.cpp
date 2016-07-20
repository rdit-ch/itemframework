#include <QDebug>
#include "file_workspace.h"
#include "project_manager_config.h"

FileWorkspace::FileWorkspace(): AbstractWorkspace(QLatin1String("File"))
{
    setValid(false);
}

FileWorkspace::~FileWorkspace()
{
}

void FileWorkspace::init()
{
    if (_filePath.isEmpty()) {
        return;
    }

    const QFileInfo workspaceFileInfo(_filePath);
    const QString fileBaseName = workspaceFileInfo.baseName();

    if (!FileHelper::fileExists(_filePath)) {
        setLastError(FileHelper::lastError());
        setName(fileBaseName);
        return;
    }

    if (!FileHelper::testFileOpenMode(_filePath, QIODevice::ReadWrite)) {
        setLastError(FileHelper::lastError());
        setName(fileBaseName);
        return;
    }

    QDomDocument workspaceDomDocument = FileHelper::domDocumentFromXMLFile(_filePath);

    if (workspaceDomDocument.isNull()) {
        setLastError(FileHelper::lastError());
        return;
    }

    if (!setWorkspaceProperties(workspaceDomDocument, fileBaseName)) {
        return;
    }

    initProjectList(workspaceDomDocument);
    setValid(true);
}

void FileWorkspace::initProjectList(const QDomDocument& workspaceDomDocument)
{
    // Get root dom element (xml -> TravizWorkspace) of workspaceDomDocument
    const QDomElement rootElement = workspaceDomDocument.documentElement();
    // Get first project dom node (xml -> TravizProject) of workspaceDomDocument
    QDomNode projectNode = rootElement.firstChild();

    // For each project dom node in workspaceDomDocument
    while (!projectNode.isNull()) {
        // Analyze project xml structure of workspace dom document
        // Get project dom element from node.
        QDomElement projectElement = projectNode.toElement();

        if (!projectElement.isNull()) {
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
                projectNode = projectNode.nextSibling();
                continue;
            }

            // Set project file path dom element
            const QDomElement projectFilePathElement = projectElement.firstChild().toElement();

            if (projectFilePathElement.isNull()) {
                // Project information incomplete (xml error).
                // ProjectFilePath node is null Ignore current project and go to next project dom node.
                projectNode = projectNode.nextSibling();
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
                projectNode = projectNode.nextSibling();
                continue;
            }

            // Analyze project xml structure was successfull.
            // Start creating project object QSharedPointer<Project> procedure.

            const QString absProjectFile = FileHelper::relativeToAbsoluteFilePath(projectFilePath, _filePath);

            // Create project dom document from xml project file (projectFilePath).
            const QDomDocument projectDomDocument = FileHelper::domDocumentFromXMLFile(absProjectFile);
            QSharedPointer<AbstractProject> project = QSharedPointer<AbstractProject>(new FileProject(settingsScope(),
                    absProjectFile,
                    projectDomDocument));
            // Set the fast load attribute.
            project->setFastLoad(fastLoad);
            // Add project to this workspace. Remeber that the project can be invalid.
            addProject(project);
        }

        // Set next project dom node as current.
        projectNode = projectNode.nextSibling();
    }
}

bool FileWorkspace::save()
{
    bool isSaved = false;
    QIODevice::OpenModeFlag openMode = QIODevice::WriteOnly;
    QFile file(_filePath);

    if (!FileHelper::fileExists(_filePath)) {
        setLastError(FileHelper::lastError());

        if (!file.open(openMode)) {
            if (!FileHelper::testFileOpenMode(_filePath, openMode)) {
                setLastError(FileHelper::lastError());
            }

            return false;
        }

        file.close();
    } else {
        if (!FileHelper::testFileOpenMode(_filePath, openMode)) {
            setLastError(FileHelper::lastError());
            return false;
        }
    }

    if (file.open(openMode)) {

        QDomDocument workspaceDomDocument = workspaceDomDocumentTemplate(name(), version(), description());
        QDomElement rootDomElement = workspaceDomDocument.documentElement();

        for (const QSharedPointer<AbstractProject>& project : projects()) {
            const QSharedPointer<FileProject> fileProject = qSharedPointerCast<FileProject>(project);
            const QString relProjectFile = FileHelper::absoluteToRelativeFilePath(fileProject->filePath(), _filePath);
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


void FileWorkspace::setAbsoluteFilePath(const QString& workspaceFilePath)
{
    if (_filePath == workspaceFilePath) {
        return;
    }

    if (!_filePath.isEmpty()) {

        QFile dstWorkspaceFile(workspaceFilePath);
        QFile srcWorkspaceFile(_filePath);

        if (dstWorkspaceFile.exists() && !dstWorkspaceFile.remove()) {
            setLastError(QString("Destination file %1 exists and could not be removed.").arg(dstWorkspaceFile.fileName()));
            return;
        }

        if (!srcWorkspaceFile.copy(dstWorkspaceFile.fileName())) {
            setLastError(QString("Source file %1 could not be copied to destination file %2.").
                         arg(srcWorkspaceFile.fileName()).
                         arg(dstWorkspaceFile.fileName()));
            return;
        }

        if (!srcWorkspaceFile.remove()) {
            setLastError(QString("Source file %1 could not be removed.").arg(_filePath));
            return;
        }
    }

    QFileInfo workspaceFileInfo = QFileInfo(workspaceFilePath);
    _filePath = workspaceFilePath;
    _fileName = workspaceFileInfo.fileName();
    _directory = workspaceFileInfo.absolutePath();
    setConnectionString(workspaceFilePath);
}

QString FileWorkspace::absoluteFilePath() const
{
    return _filePath;
}

QString FileWorkspace::directory() const
{
    return _directory;
}

QString FileWorkspace::fileName() const
{
    return _fileName;
}

bool FileWorkspace::test()
{
    if (!FileHelper::fileExists(_filePath)) {
        setLastError(FileHelper::lastError());
        return false;
    }

    if (!FileHelper::testFileOpenMode(_filePath, QIODevice::ReadWrite)) {
        setLastError(FileHelper::lastError());
        return false;
    }

    if (!isValid()) {
        setLastError(QString("Workspace %1 is not valid.\n\nFile: %2\n\n%3")
                     .arg(name())
                     .arg(_filePath)
                     .arg(lastError()));
        return false;
    }

    return true;
}

bool FileWorkspace::compare(const QSharedPointer<AbstractWorkspace>& otherWorkspace) const
{
    if (typeString() == otherWorkspace->typeString()) {
        QSharedPointer<FileWorkspace> otherFileWorkspace = qSharedPointerCast<FileWorkspace>(otherWorkspace);

        if (!otherFileWorkspace.isNull()) {
            if (_filePath == otherFileWorkspace->absoluteFilePath()) {
                return true;
            }
        }
    }

    return false;
}

QSharedPointer<FileWorkspace> FileWorkspace::createFileWorkspaceFromFile(const QString& workspaceFilePath)
{
    QSharedPointer<FileWorkspace> fileWorkspace = QSharedPointer<FileWorkspace>(new FileWorkspace);
    fileWorkspace->setAbsoluteFilePath(workspaceFilePath);
    fileWorkspace->init();
    return fileWorkspace;
}

bool FileWorkspace::deleteProject(const QSharedPointer<AbstractProject>& project)
{
    QSharedPointer<FileProject> fileProject = qSharedPointerCast<FileProject>(project);

    if (!FileHelper::removeFile(fileProject->filePath())) {
        //qDebug() << FileHelper::lastError();
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
    if (!FileHelper::removeFile(_filePath)) {
        //qDebug() << FileHelper::lastError();
        return false;
    }

    return true;
}
