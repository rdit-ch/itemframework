#ifndef PROJECT_MANAGER_CONFIG_H
#define PROJECT_MANAGER_CONFIG_H

#include <QStandardPaths>
#include <QString>

/* Global variables and settings in project manager,
 *
 * Shortcuts:
 *      Wsp -> workspace,
 *      Pro -> project,
 *      Elm -> element,
 *      Att -> attribute,
 *      Ext -> extension
 *      RegExp -> regular expression
 */

// Autosave in ms ( 10 seconds)
#define AutosaveInerval  10 * 1000

// Helpers
#define Dot  "."
#define Slash  "/"

// Date Format
#define LastUsedDateFormat "MM/dd/yyyy hh:mm:ss"

// Version Validations
#define  MajorProVersion  2
#define  MinorProVersion  0
#define  MajorWspVersion  1
#define  MinorWspVersion  0

// Version set by "create new ..."
#define FileWspVersion  QString("%1%2%3").arg(MajorWspVersion).arg(Dot).arg(MinorWspVersion)
#define FileProVersion  QString("%1%2%3").arg(MajorProVersion).arg(Dot).arg(MinorProVersion)

// Dir Locations
#define HomeFolderUser  QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
#define DefaultWorkspaceFolder  QString("%1/%2").arg(HomeFolderUser).arg(QStringLiteral("Workspace"))
#define TravizWorkDirectory  "settingWorkDirectory"

// File extensions
#define WspFileExt  "twsp"
#define ProFileExt  "tpro"
#define ProFileAutosaveExt  "swp"

// File validations
#define WspFileRegExp  QStringLiteral("[A-Za-z_0-9]+[.](") + WspFileExt + QStringLiteral(")$")
#define ProFileRegExp  QStringLiteral("[A-Za-z_0-9]+[.](") + ProFileExt + QStringLiteral(")$")

// QSetting definitions
#define SettingRecentWsp  "settingRecentWorkspaces"

// Recent Workspace XML definition
#define RecentWspDomTypeLabel  "RecentWorkspacesXML"
#define RecentWspDomElmTagName  "RecentWorkspaces"
#define RecentWspDomElmTagWsp  "TravizWorkspace"
#define RecentWspDomElmTypeAttLabel  "type"
#define RecentWspDomElmLastUsedAttLabel  "lastUsed"

// Workspace XML definitions
#define WspDomDoctype  "TravizpWorkspaceXML"
#define WspDomElmTagWsp  "TravizWorkspace"
#define WspDomElmVersionAttLabel  "version"
#define WspDomElmNameAttLabel  "name"
#define WspDomElmDescriptionAttLabel  "description"
#define WspDomElmTagProConnection  "ProjectConnection"
#define WspDomElmProPathAttLabel  "path"
#define WspDomElmProFastLoadAttLabel  "fastLoad"

// Project XML definitions
#define ProDomDoctype  "TravizpProjectXML"
#define ProDomElmTagPro  "TravizProject"
#define ProDomElmNameAttLabel  "name"
#define ProDomElmVersionAttLabel  "version"
#define ProDomElmDescriptionAttLabel  "description"

// Project GUI item data definition label
#define itemViewWidgetPropertyLabel  "ProjectGui"


#endif // PROJECT_MANAGER_CONFIG_H
