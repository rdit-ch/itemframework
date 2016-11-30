#ifndef FILE_PROJECT_EDIT_DIALOG_H
#define FILE_PROJECT_EDIT_DIALOG_H

#include <QDialog>
#include "file_project.h"
#include "file_datatype_helper.h"

namespace Ui {
class FileProjectEditDialog;
}

class FileProjectEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileProjectEditDialog(QSharedPointer<FileProject> project, QWidget *parent = 0);
    ~FileProjectEditDialog();
    FileProjectData* projectPropertiesEdited() const;
    FileProjectData* projectPropertiesInitial() const;

protected:
    void accept();
    void reject();

private slots:
    void showFileDialog();
    void onProjectNameChanged(QString name);
    void onProjectFileChanged(QString file);
    void onProjectDescriptionChanged();

private:
    void fillProjectGuiElements(const QSharedPointer<FileProject>& fileProject);
    void overrideValidation();
    QString _lastUsedPath;
    QAction* _actionFileExists = nullptr;
    FileProjectData* _projectPropertiesInitial = nullptr;
    FileProjectData* _projectPropertiesEdited = nullptr;
    Ui::FileProjectEditDialog *_ui;
    QSharedPointer<FileProject> _project;
    bool _fileTextChangedManual = false;
    bool _fileTextChangedAuto = false;
};

#endif // FILE_PROJECT_EDIT_DIALOG_H
