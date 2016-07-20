#include <QVector>
#include <QDir>
#include <QDebug>
#include <QPluginLoader>
#include <QMessageBox>
#include <QApplication>
#include <QSettings>
#include "plugin/plugin_manager_p.h"
#include "plugin/plugin_manager.h"
#include "plugin/interface_factory.h"
#include "gui/gui_plugin_manager.h"
#include "gui/gui_manager.h"
#include "helper/startup_helper.h"

STARTUP_ADD_SINGLETON(PluginManager)


//*****************************************************************************
// class PluginClassInfo
//*****************************************************************************
class PluginClassInfo
{
private:
    QMetaObject mobClass;
    QMetaObject mobInterface;
public:
    PluginClassInfo(QMetaObject mobClass, QMetaObject mobInterface)
    {
        this->mobClass = mobClass;
        this->mobInterface = mobInterface;
    }
    const QMetaObject* getClassMetaObject() const
    {
        return &(this->mobClass);
    }
    const QMetaObject* getInterfaceMetaObject() const
    {
        return &(this->mobInterface);
    }
};
//*****************************************************************************


//*****************************************************************************
// class PluginManagerPrivate
//*****************************************************************************
PluginManagerPrivate::PluginManagerPrivate(PluginManager*)
{
    qDebug() << "API VERSION: " << ApiVersion;

    if (!this->readPluginFoldersFromSettings()) {
        qDebug() << this->lastError();
    }
}

bool PluginManagerPrivate::isPluginCompatible(QString const& pluginApiVersion)
{
    int majorPublicAPI = QString(ApiVersion).split(".").first().toInt();
    int minorPublicAPI = QString(ApiVersion).split(".").last().toInt();

    QStringList pluginVersion = pluginApiVersion.split("."); // Major.Minor
    int majorPlugin = pluginVersion.first().toInt();
    int minorPlugin = pluginVersion.last().toInt();

    if (majorPublicAPI == majorPlugin && minorPublicAPI >= minorPlugin) {
        return true;
    }

    return false;
}

//*****************************************************************************
// Plugin register
//*****************************************************************************

void PluginManagerPrivate::checkPluginRegister(PluginMetaData* metaData)
{
    QSettings settings;
    settings.beginGroup("Plugins");

    QString name = metaData->name();
    QString key  = QString("%1_%2").arg(name).arg(metaData->hash());

    // if no entry found -> register new plugin
    if (settings.value(key).isNull()) {
        // qDebug() << QString("plugin [%1] not registered").arg(name);
        this->registerNewPlugin(metaData);
    } else {
        // plugin was already registered, read info
        // qDebug() << QString("plugin [%1] found, enabled = %2 ").arg(name).arg(settings.value(key).toString());

        if (metaData->isCompatible()) {
            metaData->setPluginEnabled(settings.value(key).toBool());
        } else {
            // disable plugin if it is not compatible and serialize status
            qWarning() << QString("Plugin [%1] is NOT compatible to this application version!! It will be disabled.").arg(name);
            metaData->setPluginEnabled(false);
            settings.setValue(key, false);
        }
    }

    settings.endGroup();
}

void PluginManagerPrivate::registerNewPlugin(PluginMetaData* metaData)
{
    QSettings settings;
    settings.beginGroup("Plugins");

    QString name = metaData->name();
    QString key  = QString("%1_%2").arg(name).arg(metaData->hash());

    if (initalStartup) {
        // on initial startup add "default" plugins and enable them
        settings.setValue(key, true);
        metaData->setPluginEnabled(true);
        // qDebug() << QString("plugin [%1] registered: %2 enabled=%3").arg(name).arg(key).arg(true);
    } else {
        // Ask if we want to enable it when we first find a new plugin
        int reply = QMessageBox::question(0, "Enable new plugin", QString("New plugin found [%1]. \n Enable it?").arg(name), QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes && metaData->isCompatible()) {
            settings.setValue(key, true);
            metaData->setPluginEnabled(true);
        } else {
            settings.setValue(key, false);
            metaData->setPluginEnabled(false);
        }

        // qDebug() << QString("plugin [%1] registered: %2 enabled=%3").arg(name).arg(key).arg(reply);
    }

    settings.endGroup();
}


void PluginManagerPrivate::serializePluginMetaData(PluginMetaData* data)
{
    QSettings settings;
    settings.beginGroup("Plugins");
    QString key        = QString("%1_%2").arg(data->name()).arg(data->hash());

    if (data->isPluginEnabled()) {
        settings.setValue(key, true);
    } else {
        settings.setValue(key, false);
    }

    settings.sync();
    settings.endGroup();
}

//*****************************************************************************
// Loading
//*****************************************************************************

bool PluginManagerPrivate::loadPlugins()
{
    // get and verify meta data
    if (!this->verifyPluginMetaData()) {
        qDebug() << this->lastError();
    }

    QStringList errorList;

    foreach (PluginMetaData* metadata, _pluginMetaDataList) {
        // get registered plugin info or add new plugins to register
        this->checkPluginRegister(metadata);

        //qDebug() << metadata->name() << " -> isEnabled: " << metadata->isPluginEnabled();
        if (metadata->isPluginEnabled()) {
            QPluginLoader* pluginLoader = new QPluginLoader(metadata->pluginPath());
            bool isPluginLoaded  = pluginLoader->load();

            if (!isPluginLoaded) {
                QMessageBox::information(0, "Plugin Manager", QString("Could not load plugin [%1]").arg(pluginLoader->fileName()));
                errorList.append(QString("Could not load plugin [%1]: %2 ").arg(pluginLoader->fileName(), pluginLoader->errorString()));
            } else {
                InterfaceFactory* interface_factory = qobject_cast<InterfaceFactory*>(pluginLoader->instance());

                if (interface_factory == 0) {
                    errorList.append(QString("Plugin [%1] is not of Type plugin_factory_interface ").arg(pluginLoader->fileName()));
                } else {
                    if (!interface_factory->init()) {
                        errorList.append(QString("WARRNING: Plugin [%1] couldn't be initialized properly.").arg(pluginLoader->fileName()));
                    }
                }
            }

            delete pluginLoader;
        }
    }

    if (!errorList.isEmpty()) {
        _errorList.append(errorList.join("\n"));
        return false;
    }

    return true;
}

//*****************************************************************************
// Path&Folder handling
//*****************************************************************************

bool PluginManagerPrivate::readPluginFoldersFromSettings()
{
    QSettings settings;
    settings.beginGroup("Plugins");
    QString path = settings.value("pluginPath").toString();

    if (path.isEmpty()) {
        // qDebug() << "pluginPath not yet set, initialize with default value \"plugins\"";
        // set the initial startup flag. TODO: check in register new plugin
        initalStartup = true;

        // if there is no path append the default path
        if (!this->addPluginFolder("plugins")) {
            return false;
        }
    } else {
        _pluginFolders.append(path.split(";"));
    }

    settings.endGroup();
    return true;
}

bool PluginManagerPrivate::addPluginFolder(QString const&  folder)
{
    QSettings settings;
    settings.beginGroup("Plugins");
    QString currentPath = settings.value("pluginPath").toString();

    if (currentPath.contains(folder)) {
        _errorList.append(QString("Folder \"%1\" is already in path variable").arg(folder));
        settings.endGroup();
        return false;
    } else  {
        if (QDir(folder).exists()) {
            // append the new folder to internal list and serialize it to settings
            _pluginFolders.append(folder);
            _pluginFolders.removeDuplicates();
            settings.setValue("pluginPath", _pluginFolders.join(";"));
            settings.endGroup();
            return true;
        } else {
            _errorList.append(QString("\"%1\" is not a valid directory").arg(folder));
            settings.endGroup();
            return false;
        }
    }
}

bool PluginManagerPrivate::removePluginFolder(QString const&  folder)
{
    QSettings settings;
    settings.beginGroup("Plugins");
    QString currentPath = settings.value("pluginPath").toString();

    if (currentPath.contains(folder)) {
        // qDebug() << QString("remove \"%1\" from plugin path").arg(folder);
        // remove folder from internal list and save new path to settings
        _pluginFolders.removeAll(folder);
        settings.setValue("pluginPath", _pluginFolders.join(";"));
        settings.endGroup();
        return true;
    } else {
        _errorList.append(QString("directory \"%1\" is not in plugin path").arg(folder));
        settings.endGroup();
        return false;
    }
}


QStringList const& PluginManagerPrivate::pluginPath()
{
    return _pluginFolders;
}

void PluginManagerPrivate::setPluginPath(QString const& folderList)
{
    _pluginFolders.clear();
    _pluginFolders.append(folderList.split(";"));
    _pluginFolders.removeDuplicates();

    QSettings settings;
    settings.beginGroup("Plugins");
    settings.setValue("pluginPath", folderList);
    settings.endGroup();
    // qDebug() << "Plugin path: " << _pluginFolders.join(";");
}

//*****************************************************************************
// Instance creation
//*****************************************************************************
QObject* PluginManagerPrivate::createInstance(QString className)
{
    for (int i = 0; i < _classInfoList.length(); i++) {
        const PluginClassInfo* classinfo = _classInfoList.at(i);

        if (classinfo->getClassMetaObject()->className() == className) {
            return classinfo->getClassMetaObject()->newInstance();
        }
    }

    return nullptr;
}

QVector<QObject*> PluginManagerPrivate::createInstances(QMetaObject interfaceObj)
{
    QVector<QObject*> pluginInstances;

    for (int i = 0; i < _classInfoList.length(); i++) {
        const PluginClassInfo* classinfo = _classInfoList.at(i);

        if (compareMetaObjects(&interfaceObj, classinfo->getInterfaceMetaObject())) {
            QObject* class_instance = classinfo->getClassMetaObject()->newInstance();

            if (class_instance == nullptr) {
                qWarning() << "Constructor of " + QString(classinfo->getClassMetaObject()->className()) + " was not invokable. (Q_INVOKABLE Macro Missing?)";
                continue;
            }

            pluginInstances.append(class_instance);
        }
    }

    return pluginInstances;
}

bool PluginManagerPrivate::compareMetaObjects(const QMetaObject* m1, const QMetaObject* m2)
{
    if (m1 == nullptr || m2 == nullptr) {
        return false;
    }

    if ((QString::compare(m1->className(), "QObject") == 0) || (QString::compare(m2->className(), "QObject") == 0)) {
        return false;
    }

    return (QString::compare(m1->className(), m2->className()) == 0); //TODO better implementation?
}

void PluginManagerPrivate::addPluginComponent(QMetaObject derivedMeta, QMetaObject baseMeta)
{
    _classInfoList.append(new PluginClassInfo(derivedMeta, baseMeta));
}


//*****************************************************************************
// MetaData
//*****************************************************************************
bool PluginManagerPrivate::verifyPluginMetaData()
{
    bool errorState = true;
    QStringList errorList;

    foreach (QString directory, _pluginFolders) {
        //qDebug() <<  QString("scan \"%1\" for plugins..").arg(directory);
        QDir dir(directory);

        foreach (QString fileName, dir.entryList(QDir::Files)) {
            QString pluginPath = dir.absoluteFilePath(fileName);

            if (!QLibrary::isLibrary(pluginPath)) {
                errorList.append(QString("\"%1\" is NOT a library..").arg(pluginPath));
                errorState = false;
                continue;
            }

            bool isCompatible = false;
            QPluginLoader* pluginLoader = new QPluginLoader(pluginPath);

            if (!pluginLoader->metaData().empty()) {
                QString name       = pluginLoader->metaData().value("MetaData").toObject().value("name").toString();

                if (name.trimmed().length() == 0) {
                    errorList.append(QString("Error reading plugin meta data from \"%1\", name is empty").arg(fileName));
                }

                QString version    = pluginLoader->metaData().value("MetaData").toObject().value("version").toString();

                if (version.trimmed().length() == 0) {
                    errorList.append(QString("Error rceading plugin meta data from \"%1\", version is empty").arg(fileName));
                }

                QString apiVersion = pluginLoader->metaData().value("MetaData").toObject().value("api-version").toString();

                if (apiVersion.trimmed().length() == 0) {
                    errorList.append(QString("Error reading plugin meta data from \"%1\", apiVersion is empty").arg(fileName));
                } else {
                    if (this->isPluginCompatible(apiVersion)) {
                        isCompatible = true;
                    } else {
                        errorList.append(QString("Error plugin [%1] version (%2) is not compatible with this application version (%3)!!").arg(fileName).arg(apiVersion).arg(ApiVersion));
                    }
                }

                QString vendor   = pluginLoader->metaData().value("MetaData").toObject().value("vendor").toString();

                if (vendor.trimmed().length() == 0) {
                    errorList.append(QString("Error reading plugin meta data from \"%1\", vendor is empty").arg(fileName));
                }

                QString description   = pluginLoader->metaData().value("MetaData").toObject().value("description").toString();

                if (description.trimmed().length() == 0) {
                    errorList.append(QString("Error reading plugin meta data from \"%1\", description is empty").arg(fileName));
                }

                // create the meta data object if all data is available
                if (!name.isEmpty() && !version.isEmpty() && !apiVersion.isEmpty() && !vendor.isEmpty() && !description.isEmpty()) {
                    //qDebug() << "create meta data for: " << name << ", " << version << ", " << apiVersion << ", " << vendor;
                    PluginMetaData* metaData = new PluginMetaData(name, version, apiVersion, vendor, description, pluginPath, isCompatible);
                    _pluginMetaDataList.append(metaData);
                } else {
                    errorList.append(QString("Meta data not complete for plugin [%1]").arg(name));
                }
            } else {
                errorList.append(QString("%1 is not a valid plugin, no valid meta data found!!").arg(pluginPath));
                qWarning() << QString("%1 is not a valid plugin, no valid meta data found!!").arg(pluginPath);
                break;
            }

            delete pluginLoader;
        } // files
    } // directories

    if (!errorList.isEmpty()) {
        _errorList.append(errorList.join(" \n"));
        errorState = false;
    }

    return errorState;
}


//*****************************************************************************
// Plugin list
//*****************************************************************************
QList<PluginMetaData*> PluginManagerPrivate::pluginMetaDataList()
{
    return _pluginMetaDataList;
}

//*****************************************************************************
// Error
//*****************************************************************************
QString const& PluginManagerPrivate::lastError()
{
    if (_errorList.isEmpty()) {
        _errorList.append("No error registered yet");
    }

    return _errorList.last();
}


//*****************************************************************************
// PluginManager Public API
//*****************************************************************************
PluginManager::PluginManager() : d_ptr(new PluginManagerPrivate(this))
{
    Q_D(PluginManager);

    if (! d->loadPlugins()) {
        qDebug() << this->lastError();
    }
}

PluginManager::~PluginManager()
{
}

bool PluginManager::postInit()
{
    return true;
}

bool PluginManager::preDestroy()
{
    return true;
}

QString const& PluginManager::lastError()
{
    Q_D(PluginManager);
    return d->lastError();
}

QStringList const& PluginManager::pluginPath()
{
    Q_D(PluginManager);
    return d->pluginPath();
}

void PluginManager::setPluginPath(QString const& directory)
{
    Q_D(PluginManager);
    return d->setPluginPath(directory);
}

QList<PluginMetaData*> PluginManager::pluginMetaDataList()
{

    Q_D(PluginManager);
    return d->pluginMetaDataList();
}

// ******************************************
// Helpers
// ******************************************
void PluginManager::appendPluginComponentHelper(QMetaObject derived, QMetaObject base)
{
    Q_D(PluginManager);
    return d->addPluginComponent(derived, base);
}

QVector<QObject*> PluginManager::createInstancesHelper(QMetaObject interfaceObj)
{
    Q_D(PluginManager);
    return d->createInstances(interfaceObj);
}

QObject* PluginManager::createInstanceHelper(QString className)
{
    Q_D(PluginManager);
    return d->createInstance(className);
}

void PluginManager::serializePluginMetaData(PluginMetaData* metaData)
{
    Q_D(PluginManager);
    d->serializePluginMetaData(metaData);
}
