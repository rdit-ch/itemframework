/*
 * file_helper.h
 *
 *  Created on: Feb 3, 2016
 *      Author: Marcus Pfaff
 */

#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QFile>

class QDomDocument;

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
     * @return Returns \c true if the directory is readable, otherwise returns \c false.
     *
     * @param directoryPath The directoryPath as string.
     *
     * \sa directoryIsWritable
     */
    static bool directoryIsReadable(const QString& directoryPath);

    /**
     * @return Returns \c true if the directory is writable, otherwise returns \c false.
     *
     * @param directoryPath The directoryPath as string.
     *
     * \sa directoryIsReadable
     */
    static bool directoryIsWritable(const QString& directoryPath);

    /**
     * @return Returns \c true if the directory exists, otherwise returns \c false.
     *
     * @param directoryPath The directoryPath as string.
     *
     * \sa fileExists
     */
    static bool directoryExists(const QString& directoryPath);

    /**
     * @return Returns \c true if the directory was created, otherwise returns \c false.
     *
     * @param directoryPath The directoryPath as string.
     *
     * \sa directoryExists
     */
    static bool createDirectory(const QString &directoryPath);

    /**
     * @return Returns \c true if the file was removed or does not exists, otherwise returns \c false.
     *
     * @param filePath The filepath as string.
     *
     * \sa fileExists
     * \sa testFileOpenMode
     */
    static bool removeFile(const QString& filePath);

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

    /**
     * @return Returns the absolute filepath from the relative first parameter. This function
     * needs a relativ filepath as first parameter and a absolut filepath as second parameter.
     * The first parameter will be set in relation to the second parameter to build the absolute
     * filepath for the first parameter.
     *
     * @param filePathFrom The relative filepath as string.
     * @param filePathTo The absolute filepath as string.
     *
     * \sa absoluteToRelativeFilePath
     */
    static QString relativeToAbsoluteFilePath(const QString& filePathFrom, const QString& filePathTo);

    /**
     * @return Returns the relativ filepath from absolute first parameter. This function
     * needs a absolut filepath as first parameter and a absolut filepath as second parameter.
     * The first parameter will be set in relation to the second parameter to build the relativ
     * filepath for the first parameter.
     *
     * @param filePathFrom The absolute filepath as string.
     * @param filePathTo The absolute filepath as string.
     *
     * \sa relativeToAbsoluteFilePath
     */
    static QString absoluteToRelativeFilePath(const QString& filePathFrom, const QString& filePathTo);
};

#endif // FILEHELPER_H
