/*
 * file_project.h
 *
 *  Created on: Feb 3, 2016
 *      Author: Marcus Pfaff
 */

#ifndef FILEPROJECT_H
#define FILEPROJECT_H

#include <QFileSystemWatcher>
#include <QFileInfo>

#include "abstract_project.h"
#include "file_helper.h"

class FileProject : public AbstractProject
{
public:
    FileProject(SettingsScope* parentSettingsScope, const QString& filePath, const QDomDocument& domDocument);
    ~FileProject();
    bool autosaveExists() override;
    void cleanAutosave() override;
    QString autosaveInfo() override;
    bool save() override;
    bool autosave() override;
    void reset() override;
    QString connectionString() override;
    void setLoaded(bool isLoaded) override;
    QDomDocument autosaveDomDocument() const override;
    bool setAutosaveDomDocument(const QDomDocument& domDocument);
    QDomDocument domDocument() const override;
    bool setDomDocument(const QDomDocument& domDocument) override;
    void setFilePath(const QString& filePath);
    QString filePath() const;
    QString fileName() const;
    QString path() const;

    QString relativFilePath() const;
    void setRelativFilePath(const QString &relativFilePath);

private:
    void setAutosaveFilePath(const QString &autosaveFilePath);
    void setFallbackAttributes();
    QDomDocument _domDocument;
    QDomDocument _autosaveDomDocument;
    QFileSystemWatcher _fileSystemWatcher;
    QString _autosaveFilePath;
    QString _relativFilePath;
    QFileInfo _fileInfo;
    bool _internalSave = false;

private slots:
    void onProjectFileChanged();
};

#endif // FILEPROJECT_H
