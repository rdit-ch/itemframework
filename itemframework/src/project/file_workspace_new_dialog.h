#ifndef FILE_WORKSPACE_NEW_DIALOG_H
#define FILE_WORKSPACE_NEW_DIALOG_H

#include <QWidget>
#include <QFileDialog>
#include "file_datatype_helper.h"

namespace Ui
{
class FileWorkspaceNewDialog;
}

class FileWorkspaceNewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileWorkspaceNewDialog(QString lastUsedPath, QDialog* parent = 0);
    ~FileWorkspaceNewDialog();
    FileWorkspaceData* data() const;

private:
    void overrideValidation();
    void clearUiElements();
    Ui::FileWorkspaceNewDialog* ui;
    QDialog* _parent = nullptr;
    QFileDialog* _fileDialog = nullptr;
    FileWorkspaceData* _data = nullptr;
    QAction* _actionFileExists = nullptr;
    bool _fileTextChangedManual = false;
    bool _fileTextChangedAuto = false;
    QString _lastUsedPath;

protected:
    void accept();
    void reject();

private slots:
    void showFileDialog();
    void fileDialogAccepted();
    void fileDialogRejected();
    void nameTextChanged(QString name);
    void fileTextChanged(QString file);
};

#endif // FILE_WORKSPACE_NEW_DIALOG_H
