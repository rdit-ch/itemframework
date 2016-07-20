#include "plugin/plugin_meta_data.h"
#include <QCryptographicHash>
#include <QDebug>

PluginMetaData::PluginMetaData(QString const& name, QString const& version, QString const& apiVersion, QString const& vendor, QString const& description,  QString const& pluginPath, const bool isCompatible)
    : _name(name), _version(version), _apiVersion(apiVersion), _vendor(vendor), _description(description), _pluginPath(pluginPath), _isEnabled(false), _isCompatible(isCompatible)
{
    this->generateHash(name, vendor);
}

QString const& PluginMetaData::name()
{
    return _name;
}

QString const& PluginMetaData::version()
{
    return _version;
}

QString const& PluginMetaData::apiversion()
{
    return _apiVersion;
}

QString const& PluginMetaData::vendor()
{
    return _vendor;
}

void PluginMetaData::setPluginEnabled(bool val)
{
    _isEnabled = val;
}

bool PluginMetaData::isPluginEnabled()
{
    return _isEnabled;
}

bool PluginMetaData::isCompatible()
{
    return _isCompatible;
}

QString const& PluginMetaData::pluginPath()
{
    return _pluginPath;
}

QString const& PluginMetaData::hash()
{
    return _hash;
}

QString const& PluginMetaData::description()
{
    return _description;
}

void PluginMetaData::generateHash(QString const& name, QString const& vendor)
{
    _hash =  QCryptographicHash::hash(QString("%1%2").arg(name).arg(vendor).toStdString().c_str(), QCryptographicHash::Md5).toHex();
}
