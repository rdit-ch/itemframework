#ifndef FILE_DATATYPE_HELPER
#define FILE_DATATYPE_HELPER

#include <QObject>

struct FileProjectData {
    bool fastLoad = false;
    QString name;
    QString description;
    QString file;
    QString filePath;
    QString directory;
};

struct FileWorkspaceData {
    bool isDefault = false;
    QString name;
    QString description;
    QString fileName;
    QString filePath;
    QString directory;
};

Q_DECLARE_METATYPE(FileProjectData)
Q_DECLARE_METATYPE(FileWorkspaceData)

#endif // FILE_DATATYPE_HELPER

