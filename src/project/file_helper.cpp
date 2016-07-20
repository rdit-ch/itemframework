#include <QDir>
#include "file_helper.h"
#include <QDebug>

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

bool FileHelper::testOpenFile(const QString& filePath, QIODevice::OpenMode openMode)
{
    QFile file(filePath);

    if (!fileExists(filePath)) {
        if (!file.open(openMode)) {
            _lastError = QString("No Permission to open file %1 with openmode %2.").arg(filePath).arg(openMode);
            return false;
        }

        file.close();
    }  else {
        if (!testFileOpenMode(filePath, openMode)) {
            return false;
        }
    }

    return true;
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

QString FileHelper::relativeToAbsoluteFilePath(const QString& fileNameFrom, const QString& fileNameTo)
{
    QFileInfo fileInfoTo(fileNameTo);
    QDir dirTo(fileInfoTo.dir());
    return QDir::cleanPath(dirTo.absoluteFilePath(fileNameFrom));
}

QString FileHelper::absoluteToRelativeFilePath(const QString& fileNameFrom, const QString& fileNameTo)
{
    QFileInfo fileInfoTo(fileNameTo);
    QDir dirTo(fileInfoTo.dir());
    QFileInfo fileInfoFrom(fileNameFrom);
    const QString relativePath = dirTo.relativeFilePath(fileInfoFrom.absoluteDir().absolutePath());
    const QString relativeFileName = QString("%1/%2").arg(relativePath).arg(fileInfoFrom.fileName());
    return relativeFileName;
}
