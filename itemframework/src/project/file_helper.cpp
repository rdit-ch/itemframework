#include "file_helper.h"
#include <QDir>
#include <QDomDocument>

static QString _lastError;

QString FileHelper::lastError()
{
    return _lastError;
}

bool FileHelper::fileExists(const QString& filePath)
{
    if (!QFile::exists(filePath)) {
        _lastError = QString("File %1 does not exists.").arg(filePath);
        return false;
    }

    _lastError.clear();
    return true;
}

bool FileHelper::directoryExists(const QString &directoryPath)
{
    QFileInfo directory(directoryPath);
    return directory.isDir();
}

bool FileHelper::removeFile(const QString& filePath)
{
    QFile file(filePath);

    if (!fileExists(filePath)) {
        _lastError.clear();
        return true;
    }

    if (!file.remove()) {
        _lastError = QString("Error remove file '%1'.").arg(filePath);
        return false;
    }

    _lastError.clear();
    return true;
}

bool FileHelper::directoryIsReadable(const QString& directoryPath)
{
    QFileInfo directory(directoryPath);

    if (directory.isDir() && directory.isReadable()){
        return true;
    }

    return false;
}

bool FileHelper::directoryIsWritable(const QString& directoryPath)
{
    QFileInfo directory(directoryPath);

    if (directory.isDir() && directory.isWritable()){
        return true;
    }

    return false;
}

bool FileHelper::testFileOpenMode(const QString& filePath, QIODevice::OpenMode openMode)
{
    QFile file(filePath);

    if (!file.open(openMode)) {
        _lastError = QString("No Permission to open file %1 with openmode %2.").arg(filePath).arg(openMode);
        return false;
    }

    _lastError.clear();
    file.close();
    return true;
}

QDomDocument FileHelper::domDocumentFromXMLFile(const QString& filePath)
{
    QDomDocument domDocument;

    if (!fileExists(filePath)) {
        return domDocument;
    }

    if (testFileOpenMode(filePath, QIODevice::ReadOnly)) {
        QFile file(filePath);
        QString domDocumentErrorMessage;

        if (!domDocument.setContent(&file, &domDocumentErrorMessage)) {
            _lastError = QString("Could not set domDocument content. QDomDocument error: %1.").arg(domDocumentErrorMessage);
        } else {
            _lastError.clear();
        }
    }

    return domDocument;
}

QString FileHelper::relativeToAbsoluteFilePath(const QString& filePathFrom, const QString& filePathTo)
{
    const QFileInfo fileInfoTo(filePathTo);
    const QDir dirTo(fileInfoTo.dir());
    return QDir::cleanPath(dirTo.absoluteFilePath(filePathFrom));
}

QString FileHelper::absoluteToRelativeFilePath(const QString& filePathFrom, const QString& filePathTo)
{
    const QFileInfo fileInfoTo(filePathTo);
    const QFileInfo fileInfoFrom(filePathFrom);
    const QString relativePath = fileInfoTo.dir().relativeFilePath(fileInfoFrom.absoluteDir().absolutePath());

    QString relativeFileName;
    if(relativePath.endsWith(QString("/"))){
        relativeFileName = QString("%1%2").arg(relativePath).arg(fileInfoFrom.fileName());
    } else {
        relativeFileName = QString("%1/%2").arg(relativePath).arg(fileInfoFrom.fileName());
    }

    return relativeFileName;
}
