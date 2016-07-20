#ifndef SETTINGS_SCOPE_P_H_
#define SETTINGS_SCOPE_P_H_

/**
 * Private implementation class for SettingsScope.
 */
class SettingsScopePrivate
{
public:
    /**
     * Pointer to the related class.
     */
    class SettingsScope* q_ptr;
    /**
     * Access to related class.
     */
    Q_DECLARE_PUBLIC(SettingsScope);

    /**
     * Returns the value associated with \a key or an invalid variant, if no setting for
     * \a key was found.
     *
     * If \a key cannot be found in this scope and \a searchAll is \c true, all parent scopes
     * up the parent chain are consulted until a defined value was found.
     *
     * If \a searchAll is \c false, only \a scope is consulted.
     */
    QVariant value(const QString& key, const SettingsScope* scope, bool searchAll = true) const;

    /**
     * The name associated with this settings scope.
     */
    QString _name;
    /**
     * The settings map.
     */
    QHash<QString, QVariant> _settings;

    /**
     * The Global Scope.
     */
    static SettingsScope* _globalScope;
    /**
     * The Application Scope.
     */
    static SettingsScope* _applicationScope;
};

#endif /* SETTINGS_SCOPE_P_H_ */
