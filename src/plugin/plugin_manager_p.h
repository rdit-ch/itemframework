#ifndef PLUGIN_MANAGER_P
#define PLUGIN_MANAGER_P

#include <QStringList>
#include <QList>
#include <QSettings>
#include "plugin_meta_data.h"

class PluginManager;
class PluginClassInfo;

class PluginManagerPrivate : public QObject
{
    Q_OBJECT

public:

    explicit PluginManagerPrivate(PluginManager* = nullptr);

    bool loadPlugins();
    QString const& lastError();
    QStringList const& pluginPath();
    void setPluginPath(const QStringList &folderList);
    QList<PluginMetaData*> pluginMetaDataList();

    void addPluginComponent(QMetaObject derivedMeta, QMetaObject baseMeta);
    QObject* createInstance(QString classNam);
    QVector<QObject*> createInstances(QMetaObject interfaceObject);
    void serializePluginMetaData(PluginMetaData* data);

private:
    bool addPluginFolder(QString const& folder);
    bool removePluginFolder(QString const& folder);
    void checkPluginRegister(PluginMetaData* metaData);
    void registerNewPlugin(PluginMetaData* metaData);
    bool readPluginFoldersFromSettings();
    bool verifyPluginMetaData();
    bool isPluginCompatible(QString const& pluginApiVersion);
    bool saveSettings();
    static bool compareMetaObjects(const QMetaObject* m1, const QMetaObject* m2);

    QStringList _pluginFolders;
    QStringList _errorList;
    QList<const PluginClassInfo*> _classInfoList;
    QList<PluginMetaData*> _pluginMetaDataList;
    bool initalStartup = false;

signals:
//     void newPluginDetected(PluginMetaData* metaData);
    void metaDataChanged(PluginMetaData* metaData);
};

#endif // PLUGIN_MANAGER_P
