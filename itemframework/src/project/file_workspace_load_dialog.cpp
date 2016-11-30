#include <QStandardPaths>
#include "file_workspace_load_dialog.h"

FileWorkspaceLoadDialog::FileWorkspaceLoadDialog(QString lastUsedPath, QDialog* parent) : QFileDialog(parent)
{
    _parent = parent;

    // setOption and setAttribute properties prevent the QFileDialog from delete by accepted and rejected signal
    setNameFilter(QStringLiteral("TWSP (*.twsp);; All files (*.*)"));
    setWindowTitle(QStringLiteral("Open Workspace"));

    _lastUsedPath = lastUsedPath;

    if (_lastUsedPath.isEmpty()) {
        _lastUsedPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    setDirectory(_lastUsedPath);
    setDefaultSuffix(QStringLiteral("twsp"));
    setFileMode(QFileDialog::AnyFile);
    setAcceptMode(QFileDialog::AcceptOpen);
    setOption(QFileDialog::DontUseNativeDialog, true);
    setAttribute(Qt::WA_DeleteOnClose, false);
}

FileWorkspaceLoadDialog::~FileWorkspaceLoadDialog()
{
}

void FileWorkspaceLoadDialog::accept()
{
    QFileDialog::accept();

    if (_parent != nullptr) {
        _parent->accept();
    }
}

void FileWorkspaceLoadDialog::reject()
{
    QFileDialog::reject();

    if (_parent != nullptr) {
        _parent->reject();
    }
}
