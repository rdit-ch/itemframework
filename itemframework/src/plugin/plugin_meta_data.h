#ifndef PLUGINMETADATA_H
#define PLUGINMETADATA_H

#include <QObject>
#include <QString>

class PluginMetaData  : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString Name        MEMBER _name         READ name)
    Q_PROPERTY(bool    Enabled     MEMBER _isEnabled    READ isPluginEnabled WRITE setPluginEnabled)
    Q_PROPERTY(QString Version     MEMBER _version      READ version)
    Q_PROPERTY(QString APIVersion  MEMBER _apiVersion   READ apiversion)
    Q_PROPERTY(QString Vendor      MEMBER _vendor       READ vendor)
    Q_PROPERTY(QString Description MEMBER _description  READ description)
    //Q_PROPERTY(QString PluginPath  MEMBER _pluginPath   READ pluginPath)
    //Q_PROPERTY(QString Hash        MEMBER _hash         READ hash)
    //Q_PROPERTY(bool    Compatible  MEMBER _isCompatible READ isCompatible)

public:
    PluginMetaData(QString const& name, QString const& version, QString const& apiVersion, QString const& vendor,
                   QString const& description, QString const& pluginPath, bool const isCompatible);

    QString const& name();
    QString const& version();
    QString const& apiversion();
    QString const& vendor();
    QString const& pluginPath();
    QString const& hash();
    QString const& description();

    void setPluginEnabled(bool val);
    bool isPluginEnabled();
    bool isCompatible();

private:
    void generateHash(QString const& name, QString const& vendor);

    QString _name;
    QString _version;
    QString _apiVersion;
    QString _vendor;
    QString _description;
    QString _pluginPath;
    QString _hash;

    bool _isEnabled;
    bool _isCompatible;


};

#endif // PLUGINMETADATA_H
