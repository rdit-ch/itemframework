#include "abstract_workspace.h"
#include <QTextStream>
#include <QDebug>
#include "project_manager_config.h"

AbstractWorkspace::AbstractWorkspace(const QString& typeString)
{
    _typeString = typeString;
    _settingsScope.reset(new SettingsScope("Related Workspace", SettingsScope::applicationScope()));
}


AbstractWorkspace::~AbstractWorkspace()
{
}

bool AbstractWorkspace::setWorkspaceProperties(const QDomDocument& domDocument)
{
    if (!validateWorkspaceDomDocument(domDocument)) {
        qDebug() << lastError();
        return false;
    }

    QDomDocument const workspaceDomDocument = domDocument;
    QDomElement const rootElement = workspaceDomDocument.documentElement();
    QString workspaceName = rootElement.attribute(WspDomElmNameAttLabel);

    if (!workspaceName.isEmpty()) {
        setName(workspaceName);
    }

    QString workspaceDescription = rootElement.attribute(WspDomElmDescriptionAttLabel);
    setDescription(workspaceDescription);

    QString workspaceVersion = rootElement.attribute(WspDomElmVersionAttLabel);
    int minorVersion = 0;
    int majorVersion = 0;
    QTextStream(&workspaceVersion) >> majorVersion >> minorVersion;

    _version = workspaceVersion;
    _majorWorkspaceVersion = majorVersion;
    _minorWorkspaceVersion = minorVersion;

    return true;
}

QDomDocument AbstractWorkspace::workspaceDomDocument() const
{
    return _workspaceDomDocument;
}

void AbstractWorkspace::setWorkspaceDomDocument(const QDomDocument &workspaceDomDocument)
{
    _workspaceDomDocument = workspaceDomDocument;
}

void AbstractWorkspace::setConnectionString(const QString& connectionString)
{
    if (_connectionString == connectionString) {
        return;
    }

    _connectionString = connectionString;
    emit workspaceConnectionChanged(_connectionString);
}

QString AbstractWorkspace::connectionString() const
{
    return _connectionString;
}

QString AbstractWorkspace::lastUsedDateTime() const
{
    if (_lastUsedDateTime.isValid()) {
        return _lastUsedDateTime.toString(LastUsedDateFormat);
    }

    return QDateTime::currentDateTime().toString(LastUsedDateFormat);
}

void AbstractWorkspace::setLastUsedDateTime(const QString& lastUsedDateTime)
{
    QDateTime dateTime = QDateTime::fromString(lastUsedDateTime, LastUsedDateFormat);

    if (dateTime.isValid()) {
        _lastUsedDateTime = dateTime;
    } else {
        _lastUsedDateTime = QDateTime::currentDateTime();
    }
}

const QString AbstractWorkspace::lastError() const
{
    return _lastError;
}

void AbstractWorkspace::clearLastError()
{
    _lastError = "";
}

void AbstractWorkspace::setLastError(const QString& lastError)
{
    _lastError = lastError;
}

bool AbstractWorkspace::isValid() const
{
    return _isValid;
}

void AbstractWorkspace::setValid(bool isValid)
{
    if(_isValid == isValid){
        return;
    }

    _isValid = isValid;
    emit isValidChanged(_isValid);
}

bool AbstractWorkspace::isOpen() const
{
    return _isOpen;
}

void AbstractWorkspace::setOpen(bool isOpen)
{
    _isOpen = isOpen;
}

QString AbstractWorkspace::description() const
{
    return _description;
}

void AbstractWorkspace::setDescription(const QString& description)
{
    if (_description == description) {
        return;
    }

    _description = description;
    emit workspaceDescriptionChanged(_description);
}

void AbstractWorkspace::setProjects(const QVector<QSharedPointer<AbstractProject>>& projects)
{
    _projects = projects;
}

QVector<QSharedPointer<AbstractProject> > AbstractWorkspace::projects() const
{
    return _projects;
}

QSharedPointer<AbstractProject> AbstractWorkspace::project(QString connectionString) const
{
    for(QSharedPointer<AbstractProject> project : _projects){
        if(project->connectionString() == connectionString){
            return project;
        }
    }
    return QSharedPointer<AbstractProject>();
}

int AbstractWorkspace::projectCount() const
{
    return _projects.count();
}

QString AbstractWorkspace::className() const
{
    return metaObject()->className();
}


QString AbstractWorkspace::version() const
{
    return _version;
}

void AbstractWorkspace::setVersion(const QString& version)
{
    _version = version;
}

int AbstractWorkspace::minorWorkspaceVersion() const
{
    return _minorWorkspaceVersion;
}

void AbstractWorkspace::setMinorWorkspaceVersion(int minorWorkspaceVersion)
{
    _minorWorkspaceVersion = minorWorkspaceVersion;
}

int AbstractWorkspace::majorWorkspaceVersion() const
{
    return _majorWorkspaceVersion;
}

void AbstractWorkspace::setMajorWorkspaceVersion(int majorWorkspaceVersion)
{
    _majorWorkspaceVersion = majorWorkspaceVersion;
}

bool AbstractWorkspace::isDefault() const
{
    return _isDefault;
}

void AbstractWorkspace::setDefault(bool isDefault)
{
    _isDefault = isDefault;
}

QString AbstractWorkspace::typeString() const
{
    return _typeString;
}

QString AbstractWorkspace::name() const
{
    return _name;
}

void AbstractWorkspace::setName(const QString& name)
{
    if (_name == name || name.isEmpty()) {
        return;
    }

    _name = name;
    _settingsScope->setName(_name);
    emit workspaceNameChanged(_name);
}

SettingsScope* AbstractWorkspace::settingsScope()
{
    return _settingsScope.data();
}

QDomDocument AbstractWorkspace::workspaceDomDocumentTemplate(const QString& name, const QString& version, const QString& description)
{
    QDomDocument workspaceDomDocument(WspDomDoctype);
    workspaceDomDocument.appendChild(workspaceDomDocument.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\"")));
    QDomElement rootelement = workspaceDomDocument.createElement(WspDomElmTagWsp);
    rootelement.setAttribute(WspDomElmNameAttLabel, name);
    rootelement.setAttribute(WspDomElmDescriptionAttLabel, description);
    rootelement.setAttribute(WspDomElmVersionAttLabel, version);
    workspaceDomDocument.appendChild(rootelement);
    return workspaceDomDocument;
}

bool AbstractWorkspace::validateWorkspaceDomDocument(const QDomDocument& domDocument)
{
    QDomDocument const workspaceDomDocument = domDocument;
    if(workspaceDomDocument.isNull()){
        return false;
    }

    QDomElement const rootElement = workspaceDomDocument.documentElement();

    if (rootElement.isNull() || rootElement.tagName() != WspDomElmTagWsp) {

        if (rootElement.isNull()) {
            setLastError(QString("Rootelement is null."));
        }

        if (rootElement.tagName() != WspDomElmTagWsp) {
            setLastError(QString("Rootelement tag name is wrong. expect: %1 recieved: %2")
                         .arg(WspDomElmTagWsp)
                         .arg(rootElement.tagName()));
        }

        return false;
    }

    if (!rootElement.hasAttribute(WspDomElmNameAttLabel)) {
        setLastError(QString("Rootelement miss attribute %1.").arg(WspDomElmNameAttLabel));
        return false;
    }

    if (!rootElement.hasAttribute(WspDomElmDescriptionAttLabel)) {
        setLastError(QString("Rootelement miss attribute %1.").arg(WspDomElmDescriptionAttLabel));
        return false;
    }

    if (rootElement.hasAttribute(WspDomElmVersionAttLabel)) {
        QString workspaceVersion = rootElement.attribute(WspDomElmVersionAttLabel);

        if (workspaceVersion.isEmpty()) {
            setLastError(QString("Rootelement missing workspace version."));
            return false;
        }

        int minorVersion = 0;
        int majorVersion = 0;
        QTextStream(&workspaceVersion) >> majorVersion >> minorVersion;

        if (majorVersion < MajorWspVersion) {
            setLastError(QString("Rootelement wrong workspace major version. expect >= %1  - version = %2")
                         .arg(MajorWspVersion)
                         .arg(majorVersion));
            return false;
        }
    }

    return true;
}

bool AbstractWorkspace::contains(const QSharedPointer<AbstractProject>& project) const
{
    for (const QSharedPointer<AbstractProject>& importedProject : _projects) {
        if (importedProject == project) {
            return true;
        }
    }

    return false;
}

bool AbstractWorkspace::addProject(const QSharedPointer<AbstractProject>& project)
{
    if (!contains(project)) {
        _projects.append(project);
        return save();
    }

    return false;
}

QStringList AbstractWorkspace::projectNames() const
{
    QStringList projectList;

    for (const QSharedPointer<AbstractProject>& project : _projects) {
        projectList.append(project->name());
    }

    return projectList;
}

bool AbstractWorkspace::removeProject(const QSharedPointer<AbstractProject>& project)
{
    if (_projects.removeOne(project)) {
        return save();
    }

    return false;
}

bool AbstractWorkspace::removeProjects()
{
    _projects.clear();
    return save();
}

bool AbstractWorkspace::saveExternChangedProjects() const
{
    bool saveSuccessful = true;

    for (const QSharedPointer<AbstractProject>& project : _projects) {
        if (project->isExternChanged()) {
            if (!project->save()) {
                saveSuccessful = false;
            }
        }
    }

    return saveSuccessful;
}

void AbstractWorkspace::resetExternChangedProjects() const
{
    for (const QSharedPointer<AbstractProject>& project : _projects) {
        if (project->isExternChanged()) {
            project->reset();
        }
    }
}

bool AbstractWorkspace::saveProjects() const
{
    bool success = true;

    for (const QSharedPointer<AbstractProject>& project : _projects) {
        if (!project->save()) {
            success = false;
        }
    }

    return success;
}

void AbstractWorkspace::resetProjects() const
{
    for (const QSharedPointer<AbstractProject>& project : _projects) {
        project->reset();
    }
}

