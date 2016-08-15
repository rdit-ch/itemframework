/*
 * abstract_workspace.h
 *
 *  Created on: Nov 1, 2015
 *      Author: Marcus Pfaff
 */

#ifndef ABSTRACT_WORKSPACE_H
#define ABSTRACT_WORKSPACE_H

#include <QDate>
#include <QVector>
#include "abstract_project.h"
#include "export_import_helper.h"

/**
 * The AbstractWorkspace class defines a functional basic class for e.g. a file and sql
 * workspace. A workspace can handle RTV projects.
 *
 * This class is part of the project manager feature.
 */

class AbstractWorkspace : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isDefault READ isDefault WRITE setDefault USER true)

public:
    virtual ~AbstractWorkspace();

    /**
     * @brief Initialize the Workspace and set isValid flag.
     * This function must be called, otherwise the workspace can't be valid.
     */
    virtual void init() = 0;

    /**
     * @brief
     *
     */
    virtual void update() = 0;

    /**
     * @brief Compare a other Workspace source with this Workspace source (not the Object adress).
     *
     * @param otherWorkspace The Workspace to compare with.
     *
     * @return Returns \c true if the other Workspace is equil (e.g. File-Workspace: same file), otherwise returns \c false.
     */
    virtual bool compare(const QSharedPointer<AbstractWorkspace>& otherWorkspace) const = 0;

    /**
     * @brief Save this Workspace.
     *
     * @return Returns \c true if the Workspace was successfully saved, otherwise returns \c false.
     */
    virtual bool save() = 0;

    /**
     * @brief Workspace test procedure.
     * The Test-Procedure is defined by the specific workspace class, which inherits from AbstractWorkspace class.
     *
     * @return Returns \c true if the Test was successfull, otherwise returns \c false.
     */
    virtual bool test() = 0;

    /**
     * @brief Delete this Workspace.
     * The function deletes this workspace and all included projects, if the \c deleteProjects flag is set \c true.
     *
     * @param deleteProjects The delete all projects flag (default = false).
     *
     * @return Returns \c true if the delete was successfull, otherwise returns \c false.
     */
    virtual bool deleteWorkspace(bool deleteProjects = false) = 0;

    virtual bool deleteProject(const QSharedPointer<AbstractProject>& project) = 0;

    bool saveProjects() const;

    void resetProjects() const;

    bool saveExternChangedProjects() const;

    void resetExternChangedProjects() const;

    bool removeProject(const QSharedPointer<AbstractProject>& project);

    bool removeProjects();

    /**
     * @brief This static function creates a clean workspace DomDocument.
     *
     * @param name The Workspace name.
     * @param version The Workspace version.
     * @param description The Workspace description.
     *
     * @return Returns a clean workspace DomDocument.
     */
    static QDomDocument workspaceDomDocumentTemplate(const QString& name, const QString& version, const QString& description);

    /**
     * @return Returns \c a stringlist containing all project names, which are existent in workspace.
     */
    QStringList projectNames() const;

    /**
     * @brief Validate a workspace DomDocument.
     *
     * @param domDocument The Workspace DomDocument.
     *
     * @return Returns \c true if the DomDocument is valid, otherwise returns \c false.
     */
    bool validateWorkspaceDomDocument(const QDomDocument& domDocument);

    /**
     * @return \c Returns the workspace name.
     *
     * \sa setName
     */
    QString name() const;

    /**
     * @brief Set the workspace name.
     *
     * @param name The name for the workspace.
     *
     * \sa name
     */
    void setName(const QString& name);

    /**
     * @return Returns \c true if the workspace is set as default workspace, otherwise returns \c false.
     *
     * \sa setDefault
     */
    bool isDefault() const;

    /**
     * @brief Set workspace as default workspace. On RTV start this workspace will be loaded directly.
     * No workspace manager will be shown.
     *
     * @param isDefault The enable bool flag.
     *
     * \sa isDefault
     */
    void setDefault(bool isDefault);

    /**
     * @return Returns \c the workspace version.
     *
     * \sa setVersion
     * \sa majorWorkspaceVersion
     * \sa setMajorWorkspaceVersion
     * \sa minorWorkspaceVersion
     * \sa setMinorWorkspaceVersion
     */
    QString version() const;

    /**
     * @brief Set the workspace version. If the version is not compatible with the application condition
     * the workspace could not be loaded.
     *
     * @param version The version value string.
     *
     * \sa version
     * \sa majorWorkspaceVersion
     * \sa setMajorWorkspaceVersion
     * \sa minorWorkspaceVersion
     * \sa setMinorWorkspaceVersion
     */
    void setVersion(const QString& version);

    /**
     * @return Returns \c the workspace major version.
     *
     * \sa version
     * \sa setVersion
     * \sa setMajorWorkspaceVersion
     * \sa minorWorkspaceVersion
     * \sa setMinorWorkspaceVersion
     */
    int majorWorkspaceVersion() const;

    /**
     * @brief Set the workspace major version. If the version is not compatible with the application condition
     * the workspace could not be loaded.
     *
     * @param version The version value string.
     *
     * \sa version
     * \sa setVersion
     * \sa majorWorkspaceVersion
     * \sa minorWorkspaceVersion
     * \sa setMinorWorkspaceVersion
     */
    void setMajorWorkspaceVersion(int majorWorkspaceVersion);

    /**
     * @return Returns \c the workspace minor version.
     *
     * \sa version
     * \sa setVersion
     * \sa majorWorkspaceVersion
     * \sa setMajorWorkspaceVersion
     * \sa setMinorWorkspaceVersion
     */
    int minorWorkspaceVersion() const;

    /**
     * @brief Set the workspace minor version. If the version is not compatible with the application condition
     * the workspace could not be loaded.
     *
     * @param version The version value string.
     *
     * \sa version
     * \sa setVersion
     * \sa majorWorkspaceVersion
     * \sa setMajorWorkspaceVersion
     * \sa minorWorkspaceVersion
     */
    void setMinorWorkspaceVersion(int minorWorkspaceVersion);

    /**
     * @return Returns a vector of shared pointer to all available projects in this workspace.
     *
     * \sa setProjects
     * \sa projectCount
     */
    QVector<QSharedPointer<AbstractProject>> projects() const;

    /**
     * @return Returns the project based on his connectionString. Returns a Null QSharedPointer,
     * if connectionString did not match.
     *
     * \sa setProjects
     * \sa projectCount
     * \sa projects
     * \sa connectionString
     */
    QSharedPointer<AbstractProject> project(QString connectionString) const;

    /**
     * @brief Set the vector of projects for a workspace.
     *
     * @param projects The vector of shared pointer to projects.
     *
     * \sa projects
     * \sa projectCount
     */
    void setProjects(const QVector<QSharedPointer<AbstractProject>>& projects);


    /**
     * @return Returns \c the number of projects in this workspace.
     *
     * \sa projects
     * \sa setProjects
     */
    int projectCount() const;


    void setConnectionString(const QString& connectionString);

    /**
     * @brief The Workspace connection string holds the source connection information (e.g File-Workspace -> filename,
     * Sql-Workspace -> Server and user information). The \c connectionString is unique inside a running RTV
     * application and can be used to identifier a workspace.
     *
     * @return Returns \c the workspace connection string.
     *
     * \sa projectConnectionString
     */
    QString connectionString() const;

    /**
     * @return Returns \c the workspace description.
     *
     * \sa setDescription
     */
    QString description() const;

    /**
     * @brief Set the workspace description.
     *
     * @param description The workspace description.
     *
     * \sa description
     */
    void setDescription(const QString& description);

    /**
     * @return Returns \c true if the workspace is open (in use), otherwise it returns \c false.
     *
     * \sa setOpen
     */
    bool isOpen() const;

    /**
     * @brief Set the workspace open (in use) state.
     *
     * @param isOpen The open flag.
     *
     * \sa isOpen
     */
    virtual void setOpen(bool isOpen);

    /**
     * @return Returns the metaObject className.
     */
    QString className() const;

    /**
     * @return Returns the workspace settings scope.
     */
    SettingsScope* settingsScope();

    /**
     * @return Returns the type string of a workspace (e,g. File, Sql).
     */
    QString typeString() const;

    /**
     * @return Returns \c true if the workspace is valid, otherwise it returns false.
     */
    bool isValid() const;

    /**
     * @return Returns the last occured error as a string.
     */
    const QString lastError() const;
    void clearLastError();

    QString lastUsedDateTime() const;

    void setLastUsedDateTime(const QString& lastUsedDateTime);

    bool contains(const QSharedPointer<AbstractProject>& project) const;

    bool addProject(const QSharedPointer<AbstractProject>& project);

    QDomDocument workspaceDomDocument() const;



protected:
    /**
     * @brief AbstractWorkspace constructor with typeString definition (e,g. File, Sql).
     *
     * @param typeString Define the type of workspace.
     *
     * \sa typeString
     */
    AbstractWorkspace(const QString& typeString);

    /**
     * @brief Set the workspace valid. This property decides if a workspace is loadable or not.
     *
     * @param isValid The valid flag.
     *
     * \sa isValid
     */
    void setValid(bool isValid);

    /**
     * @brief Set the last occured workspace error string.
     *
     * @param lastError The error string.
     *
     * \sa lastError
     */
    void setLastError(const QString& lastError);

    /**
     * @brief Set the workspace properties. The properties will be read from domDocument.
     *
     * @param domDocument Hold the workspace properties.
     * @param fallbackName If no workspace name is set, the workspace fallback name will be used.
     *
     * @return Returns \c true if the workspace properties are set and workspace is valid, otherwise it returns \c false.
     */
    bool setWorkspaceProperties(const QDomDocument& domDocument);

    void setWorkspaceDomDocument(const QDomDocument &workspaceDomDocument);

private:
    QScopedPointer<SettingsScope> _settingsScope;
    QVector<QSharedPointer<AbstractProject>> _projects;
    bool _isOpen = false;
    bool _isDefault = false;
    bool _isValid = false;
    QString _description;
    QString _name;
    QString _typeString;
    QString _lastError;
    QString _version;
    QString _connectionString;
    int _majorWorkspaceVersion = -1;
    int _minorWorkspaceVersion = -1;
    QDateTime _lastUsedDateTime;
    QDomDocument _workspaceDomDocument;

signals:
    void workspaceUpdated();
    void isValidChanged(bool isValid);
    void workspaceNameChanged(const QString& workspaceName);
    void workspaceConnectionChanged(const QString& workspaceConnection);
    void workspaceDescriptionChanged(const QString& workspaceDescription);
    void projectDomDocumentChanged(const QSharedPointer<AbstractProject>& project);
};

#endif // ABSTRACT_WORKSPACE_H
