#ifndef FILE_DATATYPE_HELPER
#define FILE_DATATYPE_HELPER

#include <QObject>

struct FileProjectData {
    QString name;
    QString description;
    QString filePath;
    bool fastLoad = false;
};

struct FileWorkspaceData {
    QString name;
    QString description;
    QString file;
    QString directory;
    QString absoluteFilePath;
    bool isDefault = false;
};

Q_DECLARE_METATYPE(FileProjectData)
Q_DECLARE_METATYPE(FileWorkspaceData)

#endif // FILE_DATATYPE_HELPER

