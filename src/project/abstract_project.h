/*
 * abstract_project.h
 *
 *  Created on: Feb 3, 2016
 *      Author: Marcus Pfaff
 */

#ifndef ABSTRACT_PROJECT_H
#define ABSTRACT_PROJECT_H

#include <QObject>
#include <QSharedPointer>
#include <QDomElement>
#include "helper/settings_scope.h"

class AbstractProject : public QObject
{
    Q_OBJECT

public:
    virtual ~AbstractProject();

    /**
     * @brief Save the project in a xml structure.
     *
     * @return Returns \c true if the save procedure was successful, otherwise returns \c false.
     *
     * \sa autosave
     */
    virtual bool save() = 0;

    /**
     * @brief Save the project in a xml structure.
     *
     * @return Returns \c true if the save procedure was successful, otherwise returns \c false.
     *
     * \sa save
     * \sa autosaveExists
     * \sa cleanAutosave
     * \sa autosaveInfo
     */
    virtual bool autosave() = 0;

    /**
     * @brief Check autosave project xml exists.
     *
     * @return Returns \c true if the autosave xml exists, otherwise returns \c false.
     *
     * \sa save
     * \sa autosave
     * \sa cleanAutosave
     * \sa autosaveInfo
     */
    virtual bool autosaveExists() = 0;

    /**
     * @brief Remove the autosave project xml structure.
     *
     * @return Returns \c true if the project xml structure was successfully removed, otherwise returns \c false.
     *
     * \sa save
     * \sa autosave
     * \sa autosaveExists
     * \sa autosaveInfo
     */
    virtual void cleanAutosave() = 0;

    /**
     * @brief Information about the autosave project xml structure.
     *
     * @return Returns a QString with the autosave information.
     *
     * \sa save
     * \sa autosave
     * \sa autosaveExists
     * \sa cleanAutosave
     */
    virtual QString autosaveInfo() = 0;

    /**
     * @brief Reload the complete project structure from xml.
     * All settings, which are not saved will be discarded and the project isDirty flag will be set to false.
     */
    virtual void reset() = 0;

    /**
     * @brief The connection string defines the link to the project xml source (e.g file, sql).
     *
     * @return Returns the connection string.
     */
    virtual QString connectionString() = 0;

    /**
     * @return Returns the domDocument xml structure from the project.
     *
     * \sa setDomDocument
     */
    virtual QDomDocument domDocument() const = 0;

    /**
     * @return Returns the domDocument xml structure from the autosave process.
     *
     * \sa domDocument
     * \sa setDomDocument
     */
    virtual QDomDocument autosaveDomDocument() const = 0;

    /**
     * @brief Set and validate the project domDocument xml structure.
     *
     * @param domDocument The domDocument for the project.
     *
     * @return Returns \c true if the domDocument is valid and set, otherwise returns \c false.
     *
     * \sa domDocument
     * \sa validateProjectDomDocument
     */
    virtual bool setDomDocument(const QDomDocument& domDocument) = 0;

    /**
     * @brief Set the project loaded property. This indicates, that the project is currently
     * loaded by the related workspace or not.
     *
     * @param isLoaded Set the loaded state flag.
     *
     * \sa isLoaded
     */
    virtual void setLoaded(bool isLoaded);

    /**
     * @return Returns \c true if the project is loaded in the related workspace, otherwise returns \c false.
     *
     * \sa setLoaded
     */
    bool isLoaded() const;

    /**
     * @brief The project is defined by a xml structure. This structure is represented by a domDocument.
     * The static projectDomDocumentTemplate function creats a clean project domDocument.
     *
     * @param name The name for the project.
     * @param version The version for the project. The version has to be compatible with the current
     * application, otherwise the project is invalid.
     * @param description The description for the project. The description is empty by default.
     *
     * @return Returns a clean project dom document.
     */
    static QDomDocument projectDomDocumentTemplate(const QString& name, const QString& version, const QString& description = "");

    /**
     * @brief Compare two domdocuments with each other via md5 hash bytearray.
     *
     * @param dom1 The first domDocument.
     * @param dom2 The second domDocument.
     *
     * @return Returns \c true if the domDocuments are equil, otherwise returns \c false.
     */
    static bool compareDomDocumentMD5(const QDomDocument& dom1, const QDomDocument& dom2);

    /**
     * @brief This function validates a project domDocument xml structure. A validation error
     * can be printed by lastError.
     *
     * @param domDocument The domDocument, which has to be validated.
     *
     * @return Returns \c true if the project domDocument validation was successful, otherwise returns \c false.
     *
     * \sa isValid
     * \sa setValid
     * \sa lastError
     */
    bool validateProjectDomDocument(const QDomDocument& domDocument);

    /**
     * @return Returns \c true if the project is valid, otherwise returns \c false.
     *
     * \sa setValid
     * \sa validateProjectDomDocument
     */
    bool isValid() const;

    /**
     * @brief Returns a pointer to the settings scope associated with the project.
     *
     * @return A pointer to the settings scope associated with the project.
     */
    SettingsScope* settingsScope() const;

    /**
     * @return \c Returns the name from the project.
     *
     * \sa setName
     */
    const QString& name() const;

    /**
     * @brief Set the project name.
     *
     * @param name The name for the project.
     *
     * \sa name
     */
    void setName(const QString& name);

    /**
     * @return Returns the project version.
     *
     * \sa majorProjectVersion
     * \sa minorProjectVersion
     */
    QString version() const;

    /**
     * @return Returns the minor project version.
     *
     * \sa version
     * \sa majorProjectVersion
     */
    int minorProjectVersion() const;

    /**
     * @return Returns the major project version.
     *
     * \sa version
     * \sa minorProjectVersion
     */
    int majorProjectVersion() const;

    /**
     * @return \c Returns the description from the project.
     *
     * \sa setDescription
     */
    QString description() const;

    /**
     * @brief Set the project description.
     *
     * @param description The description for the project.
     *
     * \sa description
     */
    void setDescription(const QString& description);

    /**
     * @return Returns \c true if the project is not in a persistant state (saved), otherwise returns \c false.
     *
     * \sa setDirty
     */
    bool isDirty() const;

    /**
     * @brief The isDirty flag represent the project save state.
     *
     * @param isDirty Set the dirty state flag.
     *
     * \sa isDirty
     */
    void setDirty(bool isDirty);

    /**
     * @return Returns \c true if the project should be directly load, otherwise returns \c false.
     *
     * \sa setFastLoad
     */
    bool isFastLoad() const;

    /**
     * @brief Set the project fastload property. The project can directly loaded by open the related workspace.
     *
     * @param isFastLoad Set the fastload state flag.
     *
     * \sa isFastLoad
     */
    void setFastLoad(bool isFastLoad);

    /**
     * @return The last error, which is occured in the project.
     */
    QString lastError() const;

    /**
     * @return Returns \c true if the project was changed by extern, otherwise returns \c false.
     */
    bool isExternChanged() const;

protected:
    /**
     * @brief Abstract project constructor.
     *
     * @param relatedWorkspace The related workspace.
     */
    AbstractProject(SettingsScope* parentSettingsScope);

    /**
     * @brief Initialize the project properties (e.g. settings scope) and connect them.
     *
     * @return Returns \c true if the init procedure was successful, otherwise returns \c false.
     */
    bool init();

    /**
     * @brief Set the project valid. This property decides if a project is loadable or not.
     *
     * @param isValid The valid flag.
     *
     * \sa isValid
     * \sa validateProjectDomDocument
     */
    void setValid(bool isValid);

    /**
     * @brief Set an error message to the project.
     *
     * @param lastError The error message.
     *
     * \sa lastError
     */
    void setLastError(const QString& lastError);

    /**
     * @brief Set the project version.
     * If the version is not compatible with the application, the project will be invalid.
     *
     * @param version The project version.
     *
     * \sa version
     * \sa majorProjectVersion
     * \sa minorProjectVersion
     */
    void setVersion(const QString& version);

    /**
     * @brief Set project changed by extern (e.g. project file was changed outside rtv).
     *
     * @param isExternChanged The project extern changed state.
     *
     * \sa isExternChanged
     */
    void setExternChanged(bool isExternChanged);

signals:
    void stateChange();
    void internDomChanged();
    void externDomChange();

private:
    bool loadSettingsScope();
    SettingsScope* _parentSettingsScopes = nullptr;
    QScopedPointer<SettingsScope> _settingsScope;
    int _majorProjectVersion;
    int _minorProjectVersion;
    QString _version;
    QString _name;
    QString _description;
    QString _lastError;
    QString _connectionString;
    bool _isDirty = false;
    bool _isValid = false;
    bool _isFastLoad = false;
    bool _isLoaded = false;
    bool _isExternChanged = false;
};

#endif // ABSTRACT_PROJECT_H
