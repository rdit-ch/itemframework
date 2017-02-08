#ifndef FILE_WORKSPACE_LOAD_DIALOG_H
#define FILE_WORKSPACE_LOAD_DIALOG_H

#include <QFileDialog>

class FileWorkspaceLoadDialog : public QFileDialog
{
    Q_OBJECT

public:
    explicit FileWorkspaceLoadDialog(QString lastUsedPath, QDialog* parent = 0);
    ~FileWorkspaceLoadDialog();

private:
    QDialog* _parent = nullptr;
    QString _lastUsedPath;

protected:
    void accept();
    void reject();
};

#endif // FILE_WORKSPACE_LOAD_DIALOG_H
