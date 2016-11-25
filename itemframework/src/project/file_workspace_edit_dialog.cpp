#include <QMessageBox>
#include "file_workspace_edit_dialog.h"
#include "ui_file_workspace_edit_dialog.h"
#include "project_manager_config.h"

FileWorkspaceEditDialog::FileWorkspaceEditDialog(QString lastUsedPath, QDialog* parent) : QDialog(parent), _ui(new Ui::FileWorkspaceEditDialog)
{
    _ui->setupUi(this);
    _parent = parent;
    _workspacePropertiesInitial = new FileWorkspaceData;
    _workspacePropertiesEdited = new FileWorkspaceData;
    _actionFileExists = new QAction(QIcon(":/core/projectmanager/workspace_not_valid.png"), "", this);
    _actionFileExists->setToolTip(QLatin1String("File already exists."));

    if (lastUsedPath.isEmpty()) {
        _lastUsedPath = QString(HomeFolderUser);
    } else {
        _lastUsedPath = lastUsedPath;
    }

    _ui->selectedWorkspaceFile->setToolTip(QString("e.g workspaceFile.%1").arg(WspFileExt));
    connect(_ui->selectedWorkspaceName, &QLineEdit::textChanged, this, &FileWorkspaceEditDialog::onWorkspaceNameChanged);
    connect(_ui->selectedWorkspaceFile, &QLineEdit::textChanged, this, &FileWorkspaceEditDialog::onWorkspaceFileChanged);
    connect(_ui->selectedWorkspaceDescription, &QTextEdit::textChanged, this, &FileWorkspaceEditDialog::onWorkspaceDescriptionChanged);
}

FileWorkspaceEditDialog::~FileWorkspaceEditDialog()
{
    delete _ui;
    delete _actionFileExists;
    delete _workspacePropertiesInitial;
    delete _workspacePropertiesEdited;
}

void FileWorkspaceEditDialog::setWorkspace(const QSharedPointer<AbstractWorkspace>& workspace)
{
    _fileWorkspace = qSharedPointerCast<FileWorkspace>(workspace);

    if (_fileWorkspace.isNull()) {
        setDialogMode(EditDialogMode::WorkspaceSelection);
    } else {
        setDialogMode(EditDialogMode::NoWorkspaceSelection);
        fillWorkspaceGuiElements(_fileWorkspace);
    }
}

void FileWorkspaceEditDialog::fillWorkspaceGuiElements(const QSharedPointer<FileWorkspace>& fileWorkspace)
{
    const QString name = fileWorkspace->name();
    const QString file = fileWorkspace->fileName();
    const QString directory = fileWorkspace->path();
    const QString description = fileWorkspace->description();

    _workspacePropertiesInitial->name = name;
    _workspacePropertiesInitial->fileName = file;
    _workspacePropertiesInitial->directory = directory;
    _workspacePropertiesInitial->description = description;
    _workspacePropertiesInitial->filePath = QString(directory + Slash + file);

    _ui->selectedWorkspaceName->setText(name);
    _ui->selectedWorkspaceFile->setText(file);
    _ui->selectedWorkspaceDirectory->setText(directory);
    _ui->selectedWorkspaceDescription->setText(description);

    if (_dialogMode == EditDialogMode::WorkspaceSelection) {
        _ui->lineEditSelectedFile->setText(QString(directory + Slash + file));
    }

    enableGuiElements(true);
}

void FileWorkspaceEditDialog::setDialogMode(FileWorkspaceEditDialog::EditDialogMode mode)
{
    _dialogMode = mode;

    switch (mode) {
    case EditDialogMode::NoWorkspaceSelection:
        delete _ui->labelSelectionFile;
        delete _ui->lineEditSelectedFile;
        delete _ui->pushButtonBrowseWorkspace;
        delete _ui->lineSelectionFile;
        break;

    case EditDialogMode::WorkspaceSelection:
        enableGuiElements(false);
        break;
    }
}

void FileWorkspaceEditDialog::overrideValidation()
{
    const QString name = _ui->selectedWorkspaceName->text();
    const QString file = _ui->selectedWorkspaceFile->text();
    const QString directory = _ui->selectedWorkspaceDirectory->text();
    const QString absolutFilePath = QString(directory + Slash + file);
    const QString absolutFilePathInitial = QString(_workspacePropertiesInitial->directory + Slash + _workspacePropertiesInitial->fileName);
    const QString description = _ui->selectedWorkspaceDescription->toPlainText();

    bool nameChanged = false;
    bool fileChanged = false;
    bool descriptionChanged = false;

    // Check name changed
    if (name != _workspacePropertiesInitial->name) {
        nameChanged = true;
    }

    // Check filename changed
    if (absolutFilePath != absolutFilePathInitial) {
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
            _ui->buttonSaveSelectedWorkspace->setEnabled(false);
            return;
        }
    }

    // Validate file
    if (fileChanged) {
        // File exsists warning
        if ((absolutFilePathInitial != absolutFilePath) && QFile::exists(absolutFilePath)) {
            _ui->selectedWorkspaceFile->addAction(_actionFileExists, QLineEdit::ActionPosition::LeadingPosition);
            _ui->selectedWorkspaceFile->update();
        } else {
            _ui->selectedWorkspaceFile->removeAction(_actionFileExists);
            _ui->selectedWorkspaceFile->update();
        }
    }

    if (nameChanged || fileChanged || descriptionChanged) {
        _ui->buttonSaveSelectedWorkspace->setEnabled(true);
    } else {
        _ui->buttonSaveSelectedWorkspace->setEnabled(false);
    }
}

void FileWorkspaceEditDialog::enableGuiElements(bool state)
{
    _ui->selectedWorkspaceName->setEnabled(state);
    _ui->selectedWorkspaceFile->setEnabled(state);
    _ui->selectedWorkspaceDirectory->setEnabled(state);
    _ui->selectedWorkspaceDescription->setEnabled(state);
    _ui->pushButtonSelectWorkspaceDirectory->setEnabled(state);
}

FileWorkspaceData* FileWorkspaceEditDialog::workspacePropertiesEdited() const
{
    return _workspacePropertiesEdited;
}

QSharedPointer<FileWorkspace> FileWorkspaceEditDialog::workspace() const
{
    return _fileWorkspace;
}

FileWorkspaceData* FileWorkspaceEditDialog::workspacePropertiesInitial() const
{
    return _workspacePropertiesInitial;
}

void FileWorkspaceEditDialog::accept()
{
    _workspacePropertiesEdited->name = _ui->selectedWorkspaceName->text();
    _workspacePropertiesEdited->fileName = _ui->selectedWorkspaceFile->text();
    _workspacePropertiesEdited->directory = _ui->selectedWorkspaceDirectory->text();
    _workspacePropertiesEdited->filePath = QString(_workspacePropertiesEdited->directory + Slash + _workspacePropertiesEdited->fileName);
    _workspacePropertiesEdited->description = _ui->selectedWorkspaceDescription->toPlainText();

    const QString absolutFilePathInitial = QString(_workspacePropertiesInitial->directory + Slash + _workspacePropertiesInitial->fileName);
    const QString absolutFilePathEdited = _workspacePropertiesEdited->filePath;

    if ((absolutFilePathInitial != absolutFilePathEdited) && QFile::exists(absolutFilePathEdited)) {
        int ret = QMessageBox::warning(this,
                                       tr("Overwrite workspace"),
                                       tr("File allready exists and will be overwritten.\n\n\"%1\"")
                                       .arg(absolutFilePathEdited),
                                       QMessageBox::Ok | QMessageBox::Cancel,
                                       QMessageBox::Ok);
        if(ret == QMessageBox::Cancel) {
            return;
        }
    }

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
        _fileWorkspace = FileWorkspace::createFileWorkspaceFromFile(workspaceFilePath);

        if (_fileWorkspace.isNull()) {
            enableGuiElements(false);
            return;
        }

        if (!_fileWorkspace->isValid()) {
            enableGuiElements(false);
            const QString workspaceErrorMessage = tr("Workspace is invalid.\n\nConnection: \n%1\n\nError:\n%2")
                                                  .arg(_fileWorkspace->connectionString())
                                                  .arg(_fileWorkspace->lastError());

            QMessageBox::warning(0, tr("Edit Workspace."), workspaceErrorMessage, QMessageBox::Ok);
            return;
        }

        fillWorkspaceGuiElements(_fileWorkspace);
    }
}

void FileWorkspaceEditDialog::showFileDialog()
{
    QString startDirectory = _lastUsedPath;
    if(startDirectory == QString(HomeFolderUser) && !_workspacePropertiesInitial->fileName.isEmpty()){
        startDirectory = _workspacePropertiesInitial->directory;
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a Workspace Folder"),
                                                    startDirectory,
                                                    QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty()){
        _ui->selectedWorkspaceDirectory->setText(dir);
        _lastUsedPath = dir;
    }

    overrideValidation();
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
            _ui->selectedWorkspaceFile->setText(cleanFileName);
        } else {
            _ui->selectedWorkspaceFile->clear();
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
