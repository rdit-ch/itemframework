#ifndef FILE_WORKSPACE_EDIT_DIALOG_H
#define FILE_WORKSPACE_EDIT_DIALOG_H

#include <QFileDialog>
#include "file_workspace.h"
#include "file_datatype_helper.h"

namespace Ui
{
class FileWorkspaceEditDialog;
}

class FileWorkspaceEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileWorkspaceEditDialog(QString lastUsedPath, QDialog* parent = 0);
    ~FileWorkspaceEditDialog();
    void setWorkspace(const QSharedPointer<AbstractWorkspace>& workspace = QSharedPointer<AbstractWorkspace>());
    FileWorkspaceData* workspacePropertiesInitial() const;
    FileWorkspaceData* workspacePropertiesEdited() const;

private:
    enum EditDialogMode {
        WorkspaceSelection,
        NoWorkspaceSelection
    };

    void fillWorkspaceGuiElements(const QSharedPointer<FileWorkspace>& fileWorkspace);
    void setDialogMode(EditDialogMode mode);
    void overrideValidation();
    void enableGuiElements(bool state);
    Ui::FileWorkspaceEditDialog* ui;
    QDialog* _parent = nullptr;
    QAction* _actionFileExists = nullptr;
    QString _lastUsedPath;
    QString _initialFilePath;
    bool _fileTextChangedManual = false;
    bool _fileTextChangedAuto = false;
    FileWorkspaceData* _workspacePropertiesInitial = nullptr;
    FileWorkspaceData* _workspacePropertiesEdited = nullptr;
    FileWorkspaceEditDialog::EditDialogMode _dialogMode;

protected:
    void accept();
    void reject();

private slots:
    void showSetWorkspaceFileDialog();
    void showSetNewWorkspaceFileDialog();
    void onWorkspaceNameChanged(QString name);
    void onWorkspaceFileChanged(QString file);
    void onWorkspaceDescriptionChanged();
    void onWorkspaceIsDefaultChanged();
};

#endif // FILE_WORKSPACE_EDIT_DIALOG_H
