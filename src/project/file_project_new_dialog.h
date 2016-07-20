#ifndef FILE_PROJECT_NEW_DIALOG_H
#define FILE_PROJECT_NEW_DIALOG_H

#include <QFileDialog>
#include "file_datatype_helper.h"

namespace Ui
{
class FileProjectNewDialog;
}

class FileProjectNewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileProjectNewDialog(const QString& directory, QWidget* parent = 0);
    ~FileProjectNewDialog();
    FileProjectData* data() const;

private:
    void clearUiElements();
    void overrideValidation();
    Ui::FileProjectNewDialog* ui = nullptr;
    QFileDialog* _fileDialog = nullptr;
    FileProjectData* _data = nullptr;
    QAction* _actionFileExists = nullptr;
    bool _fileTextChangedManual = false;
    bool _fileTextChangedAuto = false;
    QString lastUsedPath;

protected:
    void accept();
    void reject();

private slots:
    void selectProjectDirectoryDialog();
    void nameTextChanged(const QString& name);
    void fileTextChanged(const QString& file);
};

#endif // FILE_PROJECT_NEW_DIALOG_H
