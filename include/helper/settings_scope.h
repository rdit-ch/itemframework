#ifndef SETTINGS_SCOPE_H_
#define SETTINGS_SCOPE_H_

#include <qvariant.h>
#include <qdom.h>

#include "appcore.h"

/**
 * @brief The SettingsScope class acts as a container for scoped settings.
 *
 * The SettingsScope class stores key/value pairs which can be set using setValue() and
 * retrieved with value(). The key/value pairs can be persisted and restored using save()
 * and load().
 *
 * Creation, update and deletion of a setting is signaled with valueChanged() signal.
 *
 * A SettingsScope is identified by its name() and can have a parent SettingsScope. If a setting
 * value is requested and cannot be found in this SettingScope, the parent is consulted.
 */
class ITEMFRAMEWORK_EXPORT SettingsScope : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    /**
     * @brief Constructs a new SettingsScope named \a name with parent setting scope \a parent.
     *
     * @param name The name to be associated with the settings scope.
     * @param parent An optional settings scope parent.
     */
    explicit SettingsScope(const QString& name, SettingsScope* parent = 0);
    /**
     * @brief Destroys the SettingsScope.
     */
    virtual ~SettingsScope();

    /**
     * @brief Returns the variant value of the setting identified by \a key.
     *
     * If \a key cannot be found in this scope and \a searchAll is \c true, all parent scopes
     * up the parent chain are consulted until a defined value was found.
     *
     * If \a searchAll is \c false, only \a scope is consulted.
     *
     * If no defined value can be found in either way, the function returns \a defaultValue.
     * If no explicit default value is specified, an invalid variant is returned.
     *
     * @param key The setting key.
     * @param defaultValue Default value to be returned, if no defined value could be found.
     * @param searchAll If \c true, all parent scopes up the parent chain are consulted until
     * a defined value was found. If \c false, only \a scope is consulted.
     *
     * @return The variant value of the setting identified by \a key or \a defaultValue, if
     * the setting could not be found.
     */
    QVariant value(const QString& key, QVariant defaultValue = QVariant(),
                   bool searchAll = true) const;

    /**
     * @brief Sets the setting identified by \a key to \a value.
     *
     * If \a value is invalid and the setting is defined in this settings scope, it will be
     * removed from this settings scope.
     *
     * Signal valueChanged() and scopeChanged() is emitted. The valueChanged() signal will also
     * be dispatched to every child scope down the chain until a child has no setting for the same
     * key.
     *
     * @param key The setting key.
     * @param value The setting's value. If invalid, the setting is removed from this settings
     * scope.
     */
    void setValue(const QString& key, const QVariant& value);

    /**
     * @property SettingsScope::name
     * @brief The name associated with this settings scope.
     *
     * The settings scope name should be unique across the whole hierarchy! Not checked at the
     * moment!
     */
    /**
     * @brief Returns the name associated with this settings scope.
     *
     * @return The name associated with this settings scope.
     */
    const QString& name() const;
    /**
     * @brief Sets the name associated with this settings scope to \a name.
     *
     * The name should be unique across the whole settings scope hierarchy!
     *
     * @param name The new name.
     */
    void setName(const QString& name);

    /**
     * @brief Returns a string identifier for this settings scope.
     *
     * The identifier is build using the parent scope path to the top level scope. It lists
     * all scope names starting from top level scope to this settings scope, delimited by ':'.
     *
     * @return A string identifier for this settings scope.
     */
    QString scopeId() const;

    /**
     * @brief Returns the parent setting scope or \c NULL, if this settings scope has no parent
     * settings scope.
     *
     * @return The parent setting scope or \c NULL, if this settings scope has no parent
     * settings scope.
     */
    SettingsScope* parentScope() const
    {
        return qobject_cast<SettingsScope*>(parent());
    }

    /**
     * @brief Attaches \a newParent as parent scope.
     *
     * @param newParent the new parent scope.
     */
    void setParentScope(SettingsScope* newParent);

    /**
     * @brief Returns the top level application scope.
     *
     * @return The top level application scope.
     */
    static SettingsScope* applicationScope();

    /**
     * @brief Returns the global scope.
     *
     * The global scope is a single scope without a parent or being parent. It acts as a global
     * settings store for settings that do not need a hierarchical configuration.
     *
     * @return The global scope.
     */
    static SettingsScope* globalScope();

    /**
     * @brief Returns a map with all settings.
     *
     * If \a recurse is \c false, only settings from this scope will be included.
     *
     * If \a recurse is true, the settings from all parents up the chain will be merged to
     * the output so that parent settings will never override settings from lower levels.
     *
     * @param recurse If \c false, only settings from this scope will be returned. If
     * \c false, settings from all parents up the chain will be returned.
     *
     * @return A map with all settings.
     */
    QHash<QString, QVariant> allSettings(bool recurse = false);

    /**
     * @brief Loads persisted settings scope from given DOM element \a parent.
     *
     * Returns \c true on success, \c false otherwise.
     *
     * @param parent The DOM element to read the the persisted settings scope from.
     *
     * @return \c true on success, \c false otherwise.
     */
    bool load(const QDomElement& parent);
    /**
     * @brief Persists this settings scope to given DOM element \a parent.
     *
     * @param doc
     * @param parent
     */
    bool save(QDomDocument& doc, QDomElement& parent);

    /**
     * @brief This function has to be called once at start in order to initialize global
     * and application settings scope.
     *
     * Usually you would call <code>StartupHelper::addComponent<SettingsScope>();</code> somewhere
     * in your application initialization phase.
     *
     * @see StartupHelper
     */
    static void init();
    /**
     * @brief This function has to be called once at application end in order to save the global
     * and application settings.
     *
     * Usually you would call <code>StartupHelper::addComponent<SettingsScope>();</code> somewhere
     * in your application initialization phase.
     *
     * @see StartupHelper
     */
    static void deinit();

signals:
    /**
     * @brief Emitted when a setting value changed or the setting is removed. If removed,
     * \a value is an invalid variant.
     *
     * Note that this signal is also emitted when a parent settings scope has changed a setting
     * value but this settings scope has no related setting. This is because a setting can be
     * specified in a higher level scope and will be reflected to the lower level scopes, if the
     * lower scopes have no definition for the same setting.
     *
     * @param key The related setting key.
     * @param value The new setting value.
     */
    void valueChanged(const QString& key, const QVariant& value);
    /**
     * @brief Emitted when this settings scope changes, i.e. a setting is added or removed to this
     * scope or a setting from this scope changed or a property of this scope (e.g. the name)
     * changed.
     *
     * Note that changes to other settings scopes in the parent chain will not emit this signal
     * for this scope, scopeChanged() will be emitted for the related scope where the change
     * was done.
     */
    void scopeChanged();

private:
    QScopedPointer<class SettingsScopePrivate> d_ptr;
    Q_DECLARE_PRIVATE(SettingsScope);
    Q_DISABLE_COPY(SettingsScope);
};

#endif /* SETTINGS_SCOPE_H_ */
