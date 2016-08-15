#include <QDebug>
#include "file_project.h"
#include "project_manager_config.h"
#include "abstract_workspace.h"

FileProject::FileProject(SettingsScope* parentSettingsScope,
                         const QString& filePath,
                         const QDomDocument& projectDomDocument)
    : AbstractProject(parentSettingsScope)
{
    QDomDocument domDocument = projectDomDocument;
    bool projectIsValid = true;
    _fileInfo = QFileInfo(filePath);
    setAutosaveFilePath(QString("%1/.%2.%3").arg(_fileInfo.absolutePath()).arg(_fileInfo.baseName()).arg(ProFileAutosaveExt));

    if (autosaveExists()) {
        _autosaveDomDocument = FileHelper::domDocumentFromXMLFile(_autosaveFilePath);

        if (!setAutosaveDomDocument(_autosaveDomDocument)) {
            FileHelper::removeFile(_autosaveFilePath);
        }
    }

    if (!setDomDocument(domDocument)) {
        projectIsValid = false;
    }

    if (!init()) {
        projectIsValid = false;
    }

    setValid(projectIsValid);
}

FileProject::~FileProject()
{
}

bool FileProject::autosaveExists()
{
    const QIODevice::OpenMode openMode = QIODevice::ReadOnly;

    if (FileHelper::testFileOpenMode(_autosaveFilePath, openMode)) {
        return true;
    }

    return false;
}

void FileProject::cleanAutosave()
{
    FileHelper::removeFile(_autosaveFilePath);
    setAutosaveDomDocument(_domDocument);
}

QString FileProject::autosaveInfo()
{
    QString info;

    if (autosaveExists()) {
        QFileInfo autosaveFile = QFileInfo(_autosaveFilePath);

        info = QString("%1 \"%2\"\n\n%3 %4\n%5 %6\n%7 \"%8\"\n%9 %10")
               .arg("Found a swap file by the name")
               .arg(autosaveFile.fileName())
               .arg("owned by:")
               .arg(autosaveFile.owner())
               .arg("file name:")
               .arg(autosaveFile.filePath())
               .arg("while opening file")
               .arg(_fileInfo.fileName())
               .arg("dated:")
               .arg(autosaveFile.created().toString(LastUsedDateFormat));
    } else {
        info = QString("No swap file found.");
    }

    return info;
}

void FileProject::setFallbackAttributes()
{
    QFileInfo fileInfo(_fileInfo.filePath());
    setName(fileInfo.baseName());
    setVersion("0.0");
    setDescription(QString("Project %1 is invalid").arg(name()));
}

QString FileProject::relativFilePath() const
{
    return _relativFilePath;
}

void FileProject::setRelativFilePath(const QString &relativFilePath)
{
    _relativFilePath = relativFilePath;
}

void FileProject::setAutosaveFilePath(const QString &autosaveFilePath)
{
    if(_autosaveFilePath == autosaveFilePath){
        return;
    }

    if(isLoaded()){
        _fileSystemWatcher.disconnect(this);
        FileHelper::removeFile(_autosaveFilePath);
    }

    _autosaveFilePath = autosaveFilePath;

    if(isLoaded()){
        if(autosave()){
            _fileSystemWatcher.addPath(_autosaveFilePath);
        }
    }
}

bool FileProject::save()
{
    const QIODevice::OpenMode openMode = QIODevice::WriteOnly;
    QDomElement projectRootDomElement =  _domDocument.documentElement();

    if (!settingsScope()->save(_domDocument, projectRootDomElement)) {
        return false;
    }

    if (!FileHelper::testFileOpenMode(_fileInfo.filePath(), openMode)) {
        return false;
    }

    if (isLoaded()) {
        _internalSave = true;
    }

    QFile file(_fileInfo.filePath());
    file.open(openMode);
    file.write(_domDocument.toByteArray());
    file.close();

    setExternChanged(false);
    setDirty(false);

    if (FileHelper::fileExists(_autosaveFilePath)) {
        autosave();
    }

    return true;
}

bool FileProject::autosave()
{
    const QIODevice::OpenMode openMode = QIODevice::WriteOnly;
    QDomElement projectRootDomElement =  _domDocument.documentElement();

    if (!settingsScope()->save(_domDocument, projectRootDomElement)) {
        return false;
    }

    QFile autosaveFile(_autosaveFilePath);
    if (!FileHelper::fileExists(_autosaveFilePath) && !FileHelper::testFileOpenMode(_autosaveFilePath, openMode)) {
        return false;
    }

    autosaveFile.open(openMode);
    autosaveFile.write(_domDocument.toByteArray());
    autosaveFile.close();
    return true;
}

void FileProject::reset()
{
    bool projectIsValid = true;
    setDirty(false);
    setExternChanged(false);

    if (!FileHelper::fileExists(_fileInfo.filePath())) {
        setLastError(FileHelper::lastError());
        projectIsValid = false;
    }

    if (projectIsValid && !FileHelper::testFileOpenMode(_fileInfo.filePath(), QIODevice::ReadWrite)) {
        setLastError(FileHelper::lastError());
        projectIsValid = false;
    }

    QDomDocument projectDomDocument = FileHelper::domDocumentFromXMLFile(_fileInfo.filePath());

    if (projectIsValid && !FileHelper::lastError().isEmpty()) {
        setLastError(FileHelper::lastError());
        projectIsValid = false;
    }

    if (projectIsValid && !setDomDocument(projectDomDocument)) {
        projectIsValid = false;
    }

    setValid(projectIsValid);
}

QString FileProject::connectionString()
{
    return _fileInfo.filePath();
}

QString FileProject::filePath() const
{
    return _fileInfo.filePath();
}

QString FileProject::fileName() const
{
    return _fileInfo.fileName();
}

QString FileProject::path() const
{
    return _fileInfo.path();
}

QDomDocument FileProject::domDocument() const
{
    return _domDocument;
}

bool FileProject::setDomDocument(const QDomDocument& projectDomDocument)
{
    _domDocument = projectDomDocument;

    if (validateProjectDomDocument(_domDocument)) {
        setName(_domDocument.documentElement().attribute(ProDomElmNameAttLabel));
        setVersion(_domDocument.documentElement().attribute(ProDomElmVersionAttLabel));
        setDescription(_domDocument.documentElement().attribute(ProDomElmDescriptionAttLabel));
        return true;
    }

    QString name = _domDocument.documentElement().attribute(ProDomElmNameAttLabel);

    if (name.isEmpty()) {
        setName(_fileInfo.baseName());
        setLastError(QString("No Project name found. Use fallback name: %1.").arg(_fileInfo.baseName()));
    }

    QString version = _domDocument.documentElement().attribute(ProDomElmVersionAttLabel);

    if (version.isEmpty()) {
        setVersion(QString("0.0"));
        setLastError(QString("No Project version found. Set version to '0.0'"));
    }

    return false;
}

void FileProject::setFilePath(const QString &filePath)
{
    if (_fileInfo.filePath() == filePath || filePath.isEmpty()) {
        return;
    }

    _fileInfo.setFile(filePath);
    setAutosaveFilePath(QString("%1/.%2.%3").arg(_fileInfo.absolutePath()).arg(_fileInfo.baseName()).arg(ProFileAutosaveExt));
}

QDomDocument FileProject::autosaveDomDocument() const
{
    return _autosaveDomDocument;
}

bool FileProject::setAutosaveDomDocument(const QDomDocument& domDocument)
{

    if (!validateProjectDomDocument(domDocument)) {
        return false;
    }

    const QString name = domDocument.documentElement().attribute(ProDomElmNameAttLabel);

    if (name.isEmpty()) {
        return false;
    }

    const QString version = domDocument.documentElement().attribute(ProDomElmVersionAttLabel);

    if (version.isEmpty()) {
        return false;
    }

    return true;
}

void FileProject::setLoaded(bool isLoaded)
{
    if (isLoaded) {
        connect(&_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &FileProject::onProjectFileChanged);
        _fileSystemWatcher.addPath(_fileInfo.filePath());
    } else {
        _fileSystemWatcher.disconnect(this);
        cleanAutosave();
    }

    AbstractProject::setLoaded(isLoaded);
}

void FileProject::onProjectFileChanged()
{
    _fileSystemWatcher.addPath(_fileInfo.filePath());

    if (!isLoaded()) {
        return;
    }

    if (_internalSave) {
        _internalSave = false;
        return;
    }

    if (!compareDomDocumentMD5(_domDocument, FileHelper::domDocumentFromXMLFile(_fileInfo.filePath()))) {
        setExternChanged(true);
        emit externDomChange();
    }
}
