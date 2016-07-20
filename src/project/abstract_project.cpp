#include <QDebug>
#include <QCryptographicHash>
#include <QTimer>
#include "abstract_project.h"
#include "abstract_workspace.h"
#include "project_manager_config.h"

AbstractProject::AbstractProject(SettingsScope* parentSettingsScope)
{
    _parentSettingsScopes = parentSettingsScope;
}

AbstractProject::~AbstractProject()
{
}

bool AbstractProject::init()
{
    // Create settings scope, name it as project is named
    _settingsScope.reset(new SettingsScope(_name, _parentSettingsScopes));

    loadSettingsScope();

    // Connect for setting scope changes (not settings changes) in order to indicate a change
    // when the scope changes. Mainly to to indicate needed save on change.
    QObject::connect(_settingsScope.data(), &SettingsScope::scopeChanged, [this]() {
        // If project is loaded, mark project dirty ?? why @timo ??
        setDirty(true);
    });

    return true;
}

bool AbstractProject::validateProjectDomDocument(const QDomDocument& projectDomDocument)
{
    if (projectDomDocument.isNull()) {
        _lastError = QString("Project DomDocument is null. Project: %1.").arg(name());
        return false;
    }

    if (!projectDomDocument.documentElement().hasAttribute(ProDomElmNameAttLabel)) {
        _lastError = QString("Project has no Attribute %1.").arg(ProDomElmNameAttLabel);
        return false;
    }

    QString name = projectDomDocument.documentElement().attribute(ProDomElmNameAttLabel);

    if (name.isEmpty()) {
        _lastError = QString("No Project name found.");
        return false;
    }

    if (!projectDomDocument.documentElement().hasAttribute(ProDomElmVersionAttLabel)) {
        _lastError = QString("Project has no Attribute %1.").arg(ProDomElmVersionAttLabel);
        return false;
    }

    QString version = projectDomDocument.documentElement().attribute(ProDomElmVersionAttLabel);

    if (version.isEmpty()) {
        _lastError = QString("No Project version found.");
        return false;
    }

    if (!projectDomDocument.documentElement().hasAttribute(ProDomElmDescriptionAttLabel)) {
        _lastError = QString("Project has no Attribute %1.").arg(ProDomElmDescriptionAttLabel);
        return false;
    }

    if (projectDomDocument.documentElement().nodeName() != ProDomElmTagPro) {
        _lastError = QString("projectDomDocument.documentElement().nodeName() != ProjectDomDocName.");
        return false;
    }

    return true;
}

bool AbstractProject::loadSettingsScope()
{
    QDomElement projectDomElement = domDocument().toElement();

    if (projectDomElement.hasChildNodes()) {
        // Handle children nodes
        // Load settings scope, which could be one of the children nodes
        if (!_settingsScope->load(projectDomElement)) {
            // Indicate failure, but keep processing
            return false;
        }
    }

    return true;
}

bool AbstractProject::isExternChanged() const
{
    return _isExternChanged;
}

void AbstractProject::setVersion(const QString& version)
{
    _version = version;
    QTextStream(&_version) >> _majorProjectVersion >> _minorProjectVersion;
}

void AbstractProject::setExternChanged(bool isExternChanged)
{
    _isExternChanged = isExternChanged;
}

QString AbstractProject::lastError() const
{
    return _lastError;
}

void AbstractProject::setLastError(const QString& lastError)
{
    _lastError = lastError;
}

bool AbstractProject::isLoaded() const
{
    return _isLoaded;
}

void AbstractProject::setLoaded(bool isLoaded)
{
    _isLoaded = isLoaded;
}

QString AbstractProject::connectionString()
{
    return _connectionString;
}

QString AbstractProject::description() const
{
    return _description;
}

void AbstractProject::setDescription(const QString& description)
{
    _description = description;

    if (_description.isEmpty()) {
        _description = QLatin1String("No description set.");
    }
}

bool AbstractProject::isFastLoad() const
{
    return _isFastLoad;
}

void AbstractProject::setFastLoad(bool isFastLoad)
{
    _isFastLoad = isFastLoad;
}

SettingsScope* AbstractProject::settingsScope() const
{
    return _settingsScope.data();
}

bool AbstractProject::isDirty() const
{
    return _isDirty;
}

void AbstractProject::setDirty(bool isDirty)
{
    if (_isDirty != isDirty) {
        _isDirty = isDirty;
        emit stateChange();
    }
}

QString AbstractProject::version() const
{
    return _version;
}

QDomDocument AbstractProject::projectDomDocumentTemplate(const QString& name, const QString& version, const QString& description)
{
    QDomDocument projectDomDocument = QDomDocument(ProDomDoctype);
    projectDomDocument.appendChild(projectDomDocument.
                                   createProcessingInstruction(QLatin1String("xml"),
                                           QLatin1String("version=\"1.0\"")));

    QDomElement rootelement = projectDomDocument.createElement(ProDomElmTagPro);
    rootelement.setAttribute(ProDomElmNameAttLabel, name);
    rootelement.setAttribute(ProDomElmVersionAttLabel, version);
    rootelement.setAttribute(ProDomElmDescriptionAttLabel, description);
    projectDomDocument.appendChild(rootelement);

    return projectDomDocument;
}

bool AbstractProject::compareDomDocumentMD5(const QDomDocument& dom1, const QDomDocument& dom2)
{
    const QByteArray hashDom1 = QCryptographicHash::hash(dom1.toByteArray(), QCryptographicHash::Md5);
    const QByteArray hashDom2 = QCryptographicHash::hash(dom2.toByteArray(), QCryptographicHash::Md5);

    if (hashDom1.operator != (hashDom2)) {
        return false;
    }

    return true;
}

int AbstractProject::majorProjectVersion() const
{
    return _majorProjectVersion;
}

int AbstractProject::minorProjectVersion() const
{
    return _minorProjectVersion;
}

const QString& AbstractProject::name() const
{
    return _name;
}

void AbstractProject::setName(const QString& name)
{
    if (name != _name) {
        _name = name;

        if (!_settingsScope.isNull()) {
            _settingsScope->setName(_name);
        }
    }
}

void AbstractProject::setValid(bool isValid)
{
    _isValid = isValid;
}

bool AbstractProject::isValid() const
{
    return _isValid;
}
