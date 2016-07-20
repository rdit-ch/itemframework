/*
 * file_helper.h
 *
 *  Created on: Feb 3, 2016
 *      Author: Marcus Pfaff
 */

#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QFile>
#include <QDomDocument>
#include <QString>

class FileHelper
{
public:
    /**
     * @return Returns the last occured FileHelper error.
     */
    static QString lastError();

    /**
     * @return Returns \c true if the file exists, otherwise returns \c false.
     *
     * @param filePath The filepath as string.
     *
     * \sa removeFile
     * \sa testFileOpenMode
     */
    static bool fileExists(const QString& filePath);

    /**
     * @return Returns \c true if the file was removed or does not exists, otherwise returns \c false.
     *
     * @param filePath The filepath as string.
     *
     * \sa fileExists
     * \sa testFileOpenMode
     */
    static bool removeFile(const QString& filePath);

    static bool testOpenFile(const QString& filePath, QIODevice::OpenMode openMode);

    /**
     * @return Returns \c true if the file can be opened in selected QIODevice::OpenMode, otherwise returns \c false.
     *
     * @param filePath The filepath as string.
     * @param openMode Describe the mode in which a file is opened.
     *
     * \sa fileExists
     * \sa removeFile
     */
    static bool testFileOpenMode(const QString& filePath, QIODevice::OpenMode openMode);

    /**
     * @return Returns a DomDocument created from a xml file.
     *
     * @param filePath The xml filepath as string.
     */
    static QDomDocument domDocumentFromXMLFile(const QString& filePath);


    static QString relativeToAbsoluteFilePath(const QString& fileNameFrom, const QString& fileNameTo);

    static QString absoluteToRelativeFilePath(const QString& fileNameFrom, const QString& fileNameTo);
};

#endif // FILEHELPER_H
