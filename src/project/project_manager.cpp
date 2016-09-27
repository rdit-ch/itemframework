#include <QSettings>
#include "project_manager.h"
#include "plugin/plugin_manager.h"
#include "helper/dom_helper.h"
#include "project_manager_config.h"

ProjectManager::ProjectManager(QObject* parent) : QObject(parent)
{
    PluginManager::instance()->addPluginComponent<FileWorkspace, AbstractWorkspace>();
    PluginManager::instance()->addPluginComponent<SqlWorkspace, AbstractWorkspace>();
}

ProjectManager::~ProjectManager()
{
    _recentWorkspaces.clear();
}
QSharedPointer<AbstractWorkspace> ProjectManager::currentWorkspace() const
{
    return _currentWorkspace;
}

void ProjectManager::setCurrentWorkspace(const QSharedPointer<AbstractWorkspace>& currentWorkspace)
{
    currentWorkspace->setLastUsedDateTime(QDateTime::currentDateTime().toString(LastUsedDateFormat));
    _currentWorkspace = currentWorkspace;
    emit workspaceChanged();
    saveRecentWorkspacesSettings();
}

QVector<QSharedPointer<AbstractWorkspace> > ProjectManager::recentWorkspaces() const
{
    return _recentWorkspaces;
}

void ProjectManager::setRecentWorkspaces(const QVector<QSharedPointer<AbstractWorkspace> >& recentUsedWorkspaces)
{
    _recentWorkspaces = recentUsedWorkspaces;
    emit recentWorkspaceListChanged();
}

void ProjectManager::removeRecentWorkspaces(const QSharedPointer<AbstractWorkspace>& workspace)
{
    if (_recentWorkspaces.contains(workspace)) {
        _recentWorkspaces.removeAt(_recentWorkspaces.indexOf(workspace));
        emit recentWorkspaceListChanged();
    }
}

bool ProjectManager::addRecentWorkspace(const QSharedPointer<AbstractWorkspace>& recentWorkspace)
{
    bool isPresent = false;

    for (QSharedPointer<AbstractWorkspace> workspace : _recentWorkspaces) {
        if (workspace->compare(recentWorkspace)) {
            isPresent = true;
        }
    }

    if (!isPresent) {
        _recentWorkspaces.append(recentWorkspace);
        emit recentWorkspaceListChanged();
        return true;
    }

    return false;
}

int ProjectManager::workspaceCount() const
{
    return _recentWorkspaces.count();
}

QVector<QSharedPointer<AbstractWorkspace> > ProjectManager::workspaces() const
{
    return _recentWorkspaces;
}

QString ProjectManager::workDirectoryPath() const
{
    return _workDirectory;
}

void ProjectManager::setWorkDirectoryPath(const QString& workDirectoryPath)
{
    _workDirectory = workDirectoryPath;
    QSettings settings;
    settings.beginGroup("ProjectManager");
    settings.setValue(TravizWorkDirectory, _workDirectory);
    settings.endGroup();
}

void ProjectManager::initRecentWorkspaces()
{
    QSettings settings;
    settings.beginGroup("ProjectManager");
    QDomDocument recentWorkspacesDomDocument;
    recentWorkspacesDomDocument.setContent(settings.value(SettingRecentWsp).value<QByteArray>());
    QDomElement const rootElement = recentWorkspacesDomDocument.documentElement();

    if (rootElement.isNull() || rootElement.tagName() != RecentWspDomElmTagName || !rootElement.hasChildNodes()) {
        return;
    }

    QDomNode workspaceNode = rootElement.firstChild();

    while (!workspaceNode.isNull()) {
        QDomElement workspaceElement = workspaceNode.toElement();

        if (!workspaceElement.isNull()) {
            if (!workspaceElement.hasAttribute(RecentWspDomElmTypeAttLabel)) {
                workspaceNode = workspaceNode.nextSibling();
                continue;
            }

            QString const instanceType = workspaceElement.attribute(RecentWspDomElmTypeAttLabel);
            QSharedPointer<AbstractWorkspace> abstractWorkspace = QSharedPointer<AbstractWorkspace>(PluginManager::instance()->createInstance<AbstractWorkspace>(instanceType));

            if (abstractWorkspace.isNull()) {
                workspaceNode = workspaceNode.nextSibling();
                continue;
            }

            if (!DomHelper::loadUserProperties(abstractWorkspace.data(), workspaceElement)) {
                workspaceNode = workspaceNode.nextSibling();
                continue;
            }

            QString const lastUsedTimeDate = workspaceElement.attribute(RecentWspDomElmLastUsedAttLabel);
            abstractWorkspace->setLastUsedDateTime(lastUsedTimeDate);
            abstractWorkspace->init();
            addRecentWorkspace(abstractWorkspace);
            workspaceNode = workspaceNode.nextSibling();
        }
    }

    settings.endGroup();
}

bool ProjectManager::saveRecentWorkspacesSettings() const
{
    QSettings settings;
    settings.beginGroup("ProjectManager");

    // Rebuild recentUsedWorkspaces File (projectmeta.xml)
    QDomDocument recentUsedWorkspacesDomDocument = recentWorkspacesDomTemplate();

    for (QSharedPointer<AbstractWorkspace> workspace : _recentWorkspaces) {
        addWorkspaceElement(workspace, recentUsedWorkspacesDomDocument);
    }

    settings.setValue(SettingRecentWsp, recentUsedWorkspacesDomDocument.toByteArray());
    settings.endGroup();

    return true;
}

QSharedPointer<AbstractWorkspace> ProjectManager::defaultWorkspace() const
{
    for (QSharedPointer<AbstractWorkspace> workspace : _recentWorkspaces) {
        if (workspace->isDefault()) {
            return workspace;
        }
    }

    qWarning() << QString("No default workspace found. Return QSharedPointer<AbstractWorkspace>() - nullprt.");
    return QSharedPointer<AbstractWorkspace>();
}

bool ProjectManager::isDefaultWorkspaceSet() const
{
    // Check if a default workspace is set.
    for (QSharedPointer<AbstractWorkspace> workspace : _recentWorkspaces) {
        if (workspace->isDefault()) {
            return true;
        }
    }

    return false;
}

void ProjectManager::addWorkspaceElement(QSharedPointer<AbstractWorkspace> workspace, QDomDocument& dom) const
{
    QDomElement domElementWorkspace = dom.createElement(RecentWspDomElmTagWsp);
    domElementWorkspace.setAttribute(RecentWspDomElmTypeAttLabel, workspace->className());
    domElementWorkspace.setAttribute(RecentWspDomElmLastUsedAttLabel, workspace->lastUsedDateTime());
    DomHelper::saveUserProperties(workspace.data(), domElementWorkspace, dom);
    dom.documentElement().appendChild(domElementWorkspace);
}

QDomDocument ProjectManager::recentWorkspacesDomTemplate()
{
    QDomDocument recentUsedWorkspacesDomDocument = QDomDocument(RecentWspDomTypeLabel);
    recentUsedWorkspacesDomDocument.appendChild(recentUsedWorkspacesDomDocument.
            createProcessingInstruction(QLatin1String("xml"),
                                        QLatin1String("version=\"1.0\"")));

    QDomElement rootelement = recentUsedWorkspacesDomDocument.createElement(RecentWspDomElmTagName);
    recentUsedWorkspacesDomDocument.appendChild(rootelement);

    return recentUsedWorkspacesDomDocument;
}
