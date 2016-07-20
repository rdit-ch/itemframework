#include <QMessageBox>
#include "file_workspace_edit_dialog.h"
#include "ui_file_workspace_edit_dialog.h"
#include "project_manager_config.h"

FileWorkspaceEditDialog::FileWorkspaceEditDialog(QString lastUsedPath, QDialog* parent) : QDialog(parent), ui(new Ui::FileWorkspaceEditDialog)
{
    ui->setupUi(this);
    _parent = parent;
    _lastUsedPath = lastUsedPath;
    _workspacePropertiesInitial = new FileWorkspaceData;
    _workspacePropertiesEdited = new FileWorkspaceData;
    _actionFileExists = new QAction(QIcon(":/core/projectmanager/workspace_not_valid.png"), "", this);
    _actionFileExists->setToolTip(QString("File already exists and will be overridden."));

    if (_lastUsedPath.isEmpty()) {
        _lastUsedPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    ui->selectedWorkspaceFile->setToolTip(QString("e.g workspaceFile.%1").arg(WspFileExt));

    connect(ui->selectedWorkspaceName, &QLineEdit::textChanged, this, &FileWorkspaceEditDialog::onWorkspaceNameChanged);
    connect(ui->selectedWorkspaceFile, &QLineEdit::textChanged, this, &FileWorkspaceEditDialog::onWorkspaceFileChanged);
    connect(ui->selectedWorkspaceDescription, &QTextEdit::textChanged, this, &FileWorkspaceEditDialog::onWorkspaceDescriptionChanged);
}

FileWorkspaceEditDialog::~FileWorkspaceEditDialog()
{
    delete ui;
    delete _actionFileExists;
    delete _workspacePropertiesInitial;
    delete _workspacePropertiesEdited;
}

void FileWorkspaceEditDialog::setWorkspace(const QSharedPointer<AbstractWorkspace>& workspace)
{
    QSharedPointer<FileWorkspace> fileWorkspace = qSharedPointerCast<FileWorkspace>(workspace);

    if (fileWorkspace.isNull()) {
        setDialogMode(EditDialogMode::WorkspaceSelection);
    } else {
        setDialogMode(EditDialogMode::NoWorkspaceSelection);
        fillWorkspaceGuiElements(fileWorkspace);
    }
}

void FileWorkspaceEditDialog::fillWorkspaceGuiElements(const QSharedPointer<FileWorkspace>& fileWorkspace)
{
    const QString name = fileWorkspace->name();
    const QString file = fileWorkspace->fileName();
    const QString directory = fileWorkspace->directory();
    const QString description = fileWorkspace->description();

    _workspacePropertiesInitial->name = name;
    _workspacePropertiesInitial->file = file;
    _workspacePropertiesInitial->directory = directory;
    _workspacePropertiesInitial->description = description;
    _workspacePropertiesInitial->absoluteFilePath = QString(directory + Slash + file);

    ui->selectedWorkspaceName->setText(name);
    ui->selectedWorkspaceFile->setText(file);
    ui->selectedWorkspaceDirectory->setText(directory);
    ui->selectedWorkspaceDescription->setText(description);

    if (_dialogMode == EditDialogMode::WorkspaceSelection) {
        ui->lineEditSelectedFile->setText(QString(directory + Slash + file));
    }

    enableGuiElements(true);
}

void FileWorkspaceEditDialog::setDialogMode(FileWorkspaceEditDialog::EditDialogMode mode)
{
    _dialogMode = mode;

    switch (mode) {
    case EditDialogMode::NoWorkspaceSelection:
        delete ui->labelSelectionFile;
        delete ui->lineEditSelectedFile;
        delete ui->pushButtonBrowseWorkspace;
        delete ui->lineSelectionFile;
        break;

    case EditDialogMode::WorkspaceSelection:
        enableGuiElements(false);
        break;
    }
}

void FileWorkspaceEditDialog::overrideValidation()
{
    const QString name = ui->selectedWorkspaceName->text();
    const QString file = ui->selectedWorkspaceFile->text();
    const QString path = ui->selectedWorkspaceDirectory->text();
    const QString absolutFilePath = QString(path + Slash + file);
    const QString absolutFilePathInitial = QString(_workspacePropertiesInitial->directory + Slash + _workspacePropertiesInitial->file);
    const QString description = ui->selectedWorkspaceDescription->toPlainText();

    bool nameChanged = false;
    bool fileChanged = false;
    bool descriptionChanged = false;

    // Check name changed
    if (name != _workspacePropertiesInitial->name) {
        nameChanged = true;
    }

    // Check filename changed
    if (file != _workspacePropertiesInitial->file) {
        fileChanged = true;
    }

    // Check description changed
    if (description != _workspacePropertiesInitial->description) {
        descriptionChanged = true;
    }

    // Validate name
    if (nameChanged) {
        // Check name is not empty
        if (name.isEmpty()) {
            ui->buttonSaveSelectedWorkspace->setEnabled(false);
            return;
        }
    }

    // Validate file
    if (fileChanged) {
        // Filenaming + Extension
        if (!QRegExp(WspFileRegExp, Qt::CaseInsensitive).exactMatch(file)) {
            ui->buttonSaveSelectedWorkspace->setEnabled(false);
            return;
        }

        // File exsists warning
        if ((absolutFilePathInitial != absolutFilePath) && QFile::exists(absolutFilePath)) {
            ui->selectedWorkspaceFile->addAction(_actionFileExists, QLineEdit::ActionPosition::LeadingPosition);
        } else {
            ui->selectedWorkspaceFile->removeAction(_actionFileExists);
        }
    }

    if (nameChanged || fileChanged || descriptionChanged) {
        ui->buttonSaveSelectedWorkspace->setEnabled(true);
    } else {
        ui->buttonSaveSelectedWorkspace->setEnabled(false);
    }
}

void FileWorkspaceEditDialog::enableGuiElements(bool state)
{
    ui->selectedWorkspaceName->setEnabled(state);
    ui->selectedWorkspaceFile->setEnabled(state);
    ui->selectedWorkspaceDirectory->setEnabled(state);
    ui->selectedWorkspaceDescription->setEnabled(state);
    ui->pushButtonSelectWorkspaceDirectory->setEnabled(state);
}

FileWorkspaceData* FileWorkspaceEditDialog::workspacePropertiesEdited() const
{
    return _workspacePropertiesEdited;
}

FileWorkspaceData* FileWorkspaceEditDialog::workspacePropertiesInitial() const
{
    return _workspacePropertiesInitial;
}

void FileWorkspaceEditDialog::accept()
{
    _workspacePropertiesEdited->name = ui->selectedWorkspaceName->text();
    _workspacePropertiesEdited->file = ui->selectedWorkspaceFile->text();
    _workspacePropertiesEdited->directory = ui->selectedWorkspaceDirectory->text();
    _workspacePropertiesEdited->absoluteFilePath = QString(_workspacePropertiesEdited->directory + Slash + _workspacePropertiesEdited->file);
    _workspacePropertiesEdited->description = ui->selectedWorkspaceDescription->toPlainText();

    QDialog::accept();

    if (_parent != nullptr) {
        _parent->accept();
    }
}

void FileWorkspaceEditDialog::reject()
{
    QDialog::reject();

    if (_parent != nullptr) {
        _parent->reject();
    }
}

void FileWorkspaceEditDialog::showSetWorkspaceFileDialog()
{

    QFileDialog dialogSetWorkspaceFile;
    // setOption and setAttribute properties prevent the QFileDialog from delete by accepted and rejected signal
    dialogSetWorkspaceFile.setNameFilter(QStringLiteral("TWSP (*.twsp);; All files (*.*)"));
    dialogSetWorkspaceFile.setWindowTitle(QStringLiteral("Set Workspace File"));

    dialogSetWorkspaceFile.setDirectory(_lastUsedPath);
    dialogSetWorkspaceFile.setDefaultSuffix(QStringLiteral("twsp"));
    dialogSetWorkspaceFile.setFileMode(QFileDialog::AnyFile);
    dialogSetWorkspaceFile.setAcceptMode(QFileDialog::AcceptOpen);
    dialogSetWorkspaceFile.setOption(QFileDialog::DontUseNativeDialog, true);
    dialogSetWorkspaceFile.setAttribute(Qt::WA_DeleteOnClose, false);

    if (dialogSetWorkspaceFile.exec() == QDialog::Accepted) {
        QStringList selectedFiles = dialogSetWorkspaceFile.selectedFiles();

        if (selectedFiles.isEmpty()) {
            enableGuiElements(false);
            return;
        }

        const QString workspaceFilePath = selectedFiles.first();
        // Create new file workspace with prefered filename and workspacename
        const QSharedPointer<FileWorkspace> fileWorkspace = FileWorkspace::createFileWorkspaceFromFile(workspaceFilePath);

        if (fileWorkspace.isNull()) {
            enableGuiElements(false);
            return;
        }

        if (!fileWorkspace->isValid()) {
            enableGuiElements(false);
            const QString workspaceErrorMessage = tr("Workspace is invalid.\n\nConnection: \n%1\n\nError:\n%2")
                                                  .arg(fileWorkspace->connectionString())
                                                  .arg(fileWorkspace->lastError());

            QMessageBox::warning(0, tr("Edit Workspace."), workspaceErrorMessage, QMessageBox::Ok);
            return;
        }

        fillWorkspaceGuiElements(fileWorkspace);
    }
}

void FileWorkspaceEditDialog::showSetNewWorkspaceFileDialog()
{

}

void FileWorkspaceEditDialog::onWorkspaceNameChanged(QString name)
{
    if (!_fileTextChangedManual) {
        _fileTextChangedAuto = true;
        QString cleanFileName;

        for (int i = 0; i < name.length(); i++) {
            const QChar c = name.at(i);

            if (c != QStringLiteral(" ")) {
                cleanFileName.append(c);
            }
        }

        if (!cleanFileName.isEmpty()) {
            cleanFileName = QString(cleanFileName + Dot + WspFileExt).toLower();
            ui->selectedWorkspaceFile->setText(cleanFileName);
        } else {
            ui->selectedWorkspaceFile->clear();
        }
    }

    overrideValidation();
}

void FileWorkspaceEditDialog::onWorkspaceFileChanged(QString file)
{
    if (!file.isEmpty()) {
        if (!_fileTextChangedAuto) {
            _fileTextChangedManual = true;
        }

        _fileTextChangedAuto = false;
    } else {
        _fileTextChangedManual = false;
    }

    overrideValidation();
}

void FileWorkspaceEditDialog::onWorkspaceDescriptionChanged()
{
    overrideValidation();
}

void FileWorkspaceEditDialog::onWorkspaceIsDefaultChanged()
{
    overrideValidation();
}

/*


void SelectWorkspaceDialog::deleteSelectedWorkspace()
{
    if (!_selectedWorkspace.isNull() && _selectedWorkspace->isValid()) {
        if (_selectedWorkspace->isOpen()) {
            QMessageBox::information(this, tr("Delete Workspace."),
                                     tr("You can not delete a open Workspace.\nPlease switch to a different one."),
                                     QMessageBox::Ok);
            return;
        }

        QCheckBox* deleteProjectsCheckBox = new QCheckBox(tr("Delete all Projects in this Workspace."));
        QMessageBox deleteWorkspaceMessageBox;
        deleteWorkspaceMessageBox.setText(tr("Do you want to delete this Workspace?\n%1").arg(_selectedWorkspace->name()));
        deleteWorkspaceMessageBox.setCheckBox(deleteProjectsCheckBox);
        deleteWorkspaceMessageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        deleteWorkspaceMessageBox.setIcon(QMessageBox::Question);
        int ret = deleteWorkspaceMessageBox.exec();

        QVector<QSharedPointer<AbstractWorkspace>> recentUsedWorkspaces = _projectManager->recentUsedWorkspaces();
        bool recentUsedWorkspaceVectorChanged = false;

        if (ret == QMessageBox::Ok) {
            if (_selectedWorkspace->deleteWorkspace(deleteWorkspaceMessageBox.checkBox()->isChecked())) {

                recentUsedWorkspaces.removeOne(_selectedWorkspace);
                removeWorkspace(_selectedWorkspace);
                recentUsedWorkspaceVectorChanged = true;
            }
        }

        if (recentUsedWorkspaceVectorChanged) {
            _projectManager->setRecentUsedWorkspaces(recentUsedWorkspaces);
            _projectManager->writeRecentUsedWorkspacesToFile();
        }
    }
}


void SelectWorkspaceDialog::saveSelectedWorkspace()
{
    if (_selectedWorkspace->isValid()) {
        QString newWorkspaceName = ui->selectedWorkspaceName->text();
        QString newWorkspaceDescription = ui->selectedWorkspaceDescription->toPlainText();

        if (!newWorkspaceName.isEmpty()) {
            _selectedWorkspace->setName(newWorkspaceName);
        }

        _selectedWorkspace->setDescription(newWorkspaceDescription);
        ui->selectedWorkspaceName->setText(_selectedWorkspace->name());
        ui->buttonSaveSelectedWorkspace->setEnabled(false);
    }
}

void SelectWorkspaceDialog::selectedWorkspaceNameChanged(QString name)
{
    if (!_selectedWorkspace.isNull() && _selectedWorkspace->isValid()) {
        if (!name.isEmpty() && name != _selectedWorkspace->name()) {
            ui->buttonSaveSelectedWorkspace->setEnabled(true);
        } else {
            ui->buttonSaveSelectedWorkspace->setEnabled(false);
        }
    }
}

void SelectWorkspaceDialog::selectedWorkspaceDescriptionChanged()
{
    if (!_selectedWorkspace.isNull() && _selectedWorkspace->isValid()) {
        ui->buttonSaveSelectedWorkspace->setEnabled(true);
    }
}

*/
