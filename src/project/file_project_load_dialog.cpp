#include "file_project_load_dialog.h"
#include "project_manager_config.h"

FileProjectLoadDialog::FileProjectLoadDialog(const QString& directory, QWidget* parent) : QFileDialog(parent)
{
    // setOption and setAttribute properties prevent the QFileDialog from delete by accepted and rejected signal
    setNameFilter(QStringLiteral("TPRO (*.tpro);; All files (*.*)"));
    setWindowTitle(QStringLiteral("Load Project"));
    setDirectory(QStringLiteral("./"));
    setDefaultSuffix(QStringLiteral("tpro"));
    setFileMode(QFileDialog::ExistingFiles);
    setAcceptMode(QFileDialog::AcceptOpen);
    setOption(QFileDialog::DontUseNativeDialog, true);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setDirectory(directory);
}
