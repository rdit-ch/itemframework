#include "helper/settings_scope.h"
#include "helper/settings_scope_p.h"

#include "helper/startup_helper.h"
#include "helper/dom_helper.h"

#include <qdebug.h>
#include <qapplication.h>
#include <qstandardpaths.h>
#include <qfile.h>
#include <qdir.h>

STARTUP_ADD_COMPONENT(SettingsScope)

// Initialize static global and application scope
SettingsScope* SettingsScopePrivate::_globalScope = new SettingsScope("GlobalScope");
SettingsScope* SettingsScopePrivate::_applicationScope = new SettingsScope(SettingsScope::tr("Application"));

// Persistence Tags
static const char* SettingsScopeTag = "SettingsScope";
static const char* SettingTag = "Setting";
static const char* SettingsTag = "settings";
static const char* ApplicationScopeTag = "ApplicationScope";
static const char* GlobalScopeTag = "GlobalScope";

SettingsScope::SettingsScope(const QString& name, SettingsScope* parent)
    : d_ptr(new SettingsScopePrivate)
      // Note: QObject(parent) is initialized with setParentScope(). On first call to
      // setParentScope(), parent QObject must be null!
{
    // Initialize private implementation
    Q_D(SettingsScope);
    d->q_ptr = this;
    d->_name = name;

    // Set the parent scope
    setParentScope(parent);
}

SettingsScope::~SettingsScope()
{
    // Disconnect from parent, if needed
    if (parentScope() != NULL) {
        disconnect(parentScope(), 0, this, 0);
    }
}

QVariant SettingsScope::value(const QString& key, QVariant defaultValue, bool searchAll) const
{
    // Validate key
    if (key.isNull() || key.isEmpty()) {
        return defaultValue;
    }

    // Retrieve value from attached scope If not found in attached scope and searchAll is true,
    // walk up the parent chain to find the key in a higher level scope.
    const Q_D(SettingsScope);
    QVariant value = d->value(key, this, searchAll);

    // Return the value or specified default, if the setting could not be found
    return value.isValid() ? value : defaultValue;
}

void SettingsScope::setValue(const QString& key, const QVariant& value)
{
    // Validate key
    if (key.isNull() || key.isEmpty()) {
        return;
    }

    // Get current setting from this scope
    Q_D(SettingsScope);
    QVariant oldValue = d->_settings.value(key, QVariant());

    // If the value for related key changed, set or remove the value for this settings
    // scope and notify the change
    if (oldValue != value) {
        // Check, if we have to insert/update the setting or remove it (when value is invalid!)
        if (value.isValid()) {
//        qDebug() << QString("%1: inserted: key=%2 value=%3")
//                 .arg(Q_FUNC_INFO).arg(key).arg(value.toString());

            // Insert/update setting in this scope
            d->_settings.insert(key, value);
        } else {
//        qDebug() << QString("%1: removed: key=%2").arg(Q_FUNC_INFO).arg(key);

            // Remove the setting from this settings scope, will do nothing when the setting
            // was not defined in this settings scope
            d->_settings.remove(key);
        }

        // Notify
        emit valueChanged(key, value);
        emit scopeChanged();
    }
}

const QString& SettingsScope::name() const
{
    const Q_D(SettingsScope);
    return d->_name;
}

void SettingsScope::setName(const QString& name)
{
    Q_D(SettingsScope);

    if (d->_name != name) {
        d->_name = name;
        emit scopeChanged();
    }
}

QString SettingsScope::scopeId() const
{
    // Start with this scope's name
    const SettingsScope* scope = this;
    QString id(scope->name());

    // For every parent scope prepend the scope name delimited with ':'
    while (scope->parentScope() != nullptr) {
        scope = scope->parentScope();
        id.prepend(':').prepend(scope->name());
    }

    return id;
}

void SettingsScope::setParentScope(SettingsScope* newParent)
{
    // Remember old parent
    SettingsScope* oldParent = parentScope();

    // Nothing to do, if no change
    if (oldParent == newParent) {
        return;
    }

    // If we had an old parent, clean up
    if (oldParent != nullptr) {
        disconnect(oldParent, 0, this, 0);
    }

    // Set new parent (Note that it is also the QObject parent!)
    setParent(newParent);

    // Configure for new parent, if not null
    if (newParent != nullptr) {
        // If we have a parent settings scope, connect to parent's value changed in order to get
        // notified of parent value changes. The signal will be resent by this scope, if this scope
        // has no value (which means use the parent scope's value) for the related key!
        if (newParent != NULL) {
            QObject::connect(newParent, &SettingsScope::valueChanged, this,
            [this](const QString & key, const QVariant & value) {
                Q_D(SettingsScope);

                // Resent signal if this scope has no entry for related key. Note that if this scope
                // has a value, it overwrites the value from parent and thus this scope's related
                // value did not change!
                if (!d->_settings.contains(key)) {
                    emit valueChanged(key, value);
                }
            });
        }

    }

    // TODO Notify change
    // Notify change. As we have a new parent, all settings that are not defined in this scope
    // might change. Not only the settings from new parent (and its parent and so on) will have
    // changed, the settings from old parent which are no more defined in new parent also changed!
    if (oldParent != nullptr && newParent != nullptr) {  // initially once is ok
        qCritical() << QString("%1: UNIMPLEMENTED: scope '%2' reparented but no notification yet!")
                    .arg(Q_FUNC_INFO, name());
    }
}

SettingsScope* SettingsScope::applicationScope()
{
    return SettingsScopePrivate::_applicationScope;
}

SettingsScope* SettingsScope::globalScope()
{
    return SettingsScopePrivate::_globalScope;
}

QHash<QString, QVariant> SettingsScope::allSettings(bool recurse)
{
    // Private Implementation access
    Q_D(SettingsScope);

    // Copy this scope's settings
    QHash<QString, QVariant> ret = d->_settings;

    // If recurse, and we have a parent scope, merge all recurse settings from parent scope
    // into this scope's settings
    if (recurse && parentScope() != nullptr) {

        // Get parent's recurse settings
        QHash<QString, QVariant> parentSettings = parentScope()->allSettings(true);
        // Iterate over the settings
        QHashIterator<QString, QVariant> it(parentSettings);

        // For every parent recurse setting that is not in this scope's settings, add them
        // to the return map.
        while (it.hasNext()) {
            // Get next map entry
            it.next();

            // If this scope has no settings for the parent scope, add it
            if (!ret.contains(it.key())) {
                ret.insert(it.key(), it.value());
            }
        }
    }

    // Return the map
    return ret;
}

bool SettingsScope::load(const QDomElement& parent)
{
//    qDebug() << QString("%1: %2").arg(Q_FUNC_INFO, parent.tagName());

    // Get the settings scope element from parent element
    QDomElement settingsScope = parent.firstChildElement(SettingsScopeTag);
    bool success = true;

    // If found, read all settings
    if (!settingsScope.isNull()) {
        // Get first setting from scope element
        QDomElement setting = settingsScope.firstChildElement(SettingTag);

        // Read all settings and restore them
        while (!setting.isNull()) {
            // Read key and value from setting element
            QString key;
            QVariant value;

            // If read was successful, add the setting to the internal dictionary. Otherwise
            // show warning.
            if (DomHelper::loadVariant(value, key, setting)) {
                setValue(key, value);
            } else {
                qWarning() << QString("scope \"%1\": failed to load setting \"%2\"")
                           .arg(scopeId(), key);
                // Mark problem
                success = false;
            }

            // Get next setting from scope element
            setting = setting.nextSiblingElement(SettingTag);
        }
    }

    return success;
}

bool SettingsScope::save(QDomDocument& doc, QDomElement& parent)
{
    Q_D(SettingsScope);
    // Create the settings scope DOM element
    QDomElement element = doc.createElement(SettingsScopeTag);
    bool success = true;

    // Iterate over settings
    QHashIterator<QString, QVariant> it(d->_settings);

    // For all settings generate an element with the setting and add it to the scope element.
    while (it.hasNext()) {
        // Get next setting
        it.next();
        // Create a DOM element for the setting and save the setting to it. The setting is saved as
        // variant with key as name!
        QDomElement setting = DomHelper::saveVariant(it.value(), SettingTag, doc, it.key());

        // If successful, add setting element to scope element. Otherwise issue warning message.
        if (!setting.isNull()) {
            element.appendChild(setting);
        } else {
            qWarning() << QString("scope \"%1\": failed to save setting \"%2\"")
                       .arg(scopeId(), it.key());
            // Indicate failure
            success = false;
        }

//        qDebug() << QString("scope \"%1\": saved setting \"%2\"").arg(scopeId(this), it.key());
    }

    // If the scope element has any entries, add it to the given parent element
    if (element.hasAttributes() || element.hasChildNodes()) {
        parent.appendChild(element);
    }

    return success;
}

/**
 * @brief Returns the settings xml-filename from standard location.
 */
static QString getSettingsPath()
{
    // Check QCoreApplication settings needed
    Q_ASSERT_X(!QCoreApplication::organizationName().isEmpty(), Q_FUNC_INFO,
               "no organization name set!");
    Q_ASSERT_X(!QCoreApplication::applicationName().isEmpty(), Q_FUNC_INFO,
               "no application name set!");

    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + QDir::separator() + "settings.xml";
}

void SettingsScope::init()
{
    // Get settings xml-file
    QString settingsPath = getSettingsPath();
    QFile settingsFile(settingsPath);

    // If the file does not exists, we're done, we have no persistent settings
    if (!settingsFile.exists()) {
        // This is no error, we just have no persistent settings
        return;
    }

    // Create DOM document for settings
    QDomDocument doc(SettingsTag);

    // Open the file and read it into the DOM document
    if (!settingsFile.open(QIODevice::ReadOnly) || !doc.setContent(&settingsFile)) {
        qCritical() << QString("cannot load settings file '%1': %2")
                    .arg(settingsPath, settingsFile.errorString());
        return;
    }

    // Done with file, we can close it
    settingsFile.close();
    // Get the first element, the document element
    QDomElement docElem = doc.documentElement();

    // It should be tagged with the settings tag!
    if (docElem.nodeName() == SettingsTag) {
        // Try to get the application scope DOM element
        QDomElement applScopeElem = docElem.firstChildElement(ApplicationScopeTag);

        // If there is an element for application scope, initialize application scope with
        // the element contents
        if (!applScopeElem.isNull()) {
            if (!SettingsScopePrivate::_applicationScope->load(applScopeElem)) {
                qCritical() << "failed to read application settings scope";
            }
        }

        // Try to get the global scope DOM element
        QDomElement globalScopeElem = docElem.firstChildElement(GlobalScopeTag);

        // If there is an element for global scope, initialize global scope with the element
        // contents
        if (!globalScopeElem.isNull()) {
            if (!SettingsScopePrivate::_globalScope->load(globalScopeElem)) {
                qCritical() << "failed to read global settings scope";
            }
        }
    } else {
        qCritical() << QString("%1: invalid document name, have '%2', expect '%3'")
                    .arg(settingsPath, docElem.nodeName(), SettingsTag);
    }
}

void SettingsScope::deinit()
{
    // Get settings xml-file and its parent directory
    QString settingsPath = getSettingsPath();
    QDir parentDir = QFileInfo(settingsPath).dir();

    // If the parent directory does not exist yet, create it
    if (!parentDir.exists()) {
        if (!parentDir.mkpath(".")) {
            qCritical() << QString("failed to create directory path '%1'")
                        .arg(parentDir.absolutePath());
            return;
        }
    }

    // Open settings file
    QFile settingsFile(settingsPath);

    // Check if we can open/create the file
    if (!settingsFile.open(QIODevice::WriteOnly)) {
        qCritical() << QString("cannot open settings file '%1': %2")
                    .arg(settingsPath, settingsFile.errorString());
        return;
    }

    // Create DOM document for settings
    QDomDocument doc(SettingsTag);
    // Add processing instructions
    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\""));
    // Create the document element
    QDomElement docElem = doc.createElement(SettingsTag);
    // Set file version
    docElem.setAttribute("version", "1.0.0.0");
    doc.appendChild(docElem);

    // Create an element for application scope
    QDomElement applScopeElem = doc.createElement(ApplicationScopeTag);

    // Save application scope to element
    if (SettingsScopePrivate::_applicationScope->save(doc, applScopeElem)) {
        // If successful, add the application settings element to the document element
        docElem.appendChild(applScopeElem);
    }

    // Create an element for global scope
    QDomElement globalScopeElem = doc.createElement(GlobalScopeTag);

    // Save global scope to element
    if (SettingsScopePrivate::_globalScope->save(doc, globalScopeElem)) {
        // If successful, add the global settings element to the document element
        docElem.appendChild(globalScopeElem);
    }

    // Write the DOM document to the xml-file
    if (!settingsFile.write(doc.toByteArray())) {
        qCritical() << QString("failed to write to settings file '%1': %2")
                    .arg(settingsPath, settingsFile.errorString());
    }

    // Close the file
    settingsFile.close();
}

QVariant SettingsScopePrivate::value(const QString& key, const SettingsScope* scope,
                                     bool searchAll) const
{
    // Cycle through the scope chain up, starting with specified scope to find the setting
    // related to the specified key.
    do {
        // Get setting from related scope
        QVariant value = scope->d_func()->_settings.value(key, QVariant());

        // If a setting was found, return it. If searchAll is false, return the variant anyway.
        if (value.isValid() || !searchAll) {
//            qDebug() << QString("%1: found %2 in %3: valid=%4 value=%5")
//                     .arg(Q_FUNC_INFO, key, scope->name()).arg(value.isValid())
//                     .arg(value.toString());

            return value;
        }

        // Not found, consult parent scope
        scope = scope->parentScope();

    } while (scope != NULL);

    // Not found in this scope or in any up the parent chain, return invalid variant
    return QVariant();
}

