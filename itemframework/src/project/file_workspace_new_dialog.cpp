#include <QDebug>
#include "file_workspace_new_dialog.h"
#include "ui_file_workspace_new_dialog.h"
#include "project_manager_config.h"

FileWorkspaceNewDialog::FileWorkspaceNewDialog(QString lastUsedPath, QDialog* parent) : QDialog(parent), ui(new Ui::FileWorkspaceNewDialog)
{
    qRegisterMetaType<FileWorkspaceData>();
    ui->setupUi(this);
    _parent = parent;
    _lastUsedPath = lastUsedPath;

    ui->lineEditPath->setText(_lastUsedPath);

    _data = new FileWorkspaceData;
    _fileDialog = new QFileDialog(this, QStringLiteral("Create new Workspace"), _lastUsedPath);

    _actionFileExists = new QAction(QIcon(":/core/projectmanager/workspace_not_valid.png"), "", this);
    _actionFileExists->setToolTip(QString("File already exists and will be overridden."));

    // setOption and setAttribute properties prevent the QFileDialog from delete by accepted and rejected signal
    _fileDialog->setFileMode(QFileDialog::Directory);
    _fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    _fileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
    _fileDialog->setAttribute(Qt::WA_DeleteOnClose, false);

    ui->butAccept->setEnabled(false);

    connect(_fileDialog, &QFileDialog::accepted, this, &FileWorkspaceNewDialog::fileDialogAccepted);
    connect(_fileDialog, &QFileDialog::rejected, this, &FileWorkspaceNewDialog::fileDialogRejected);
    connect(ui->butShowFileDialog, &QPushButton::released, this, &FileWorkspaceNewDialog::showFileDialog);
    connect(ui->lineEditName, &QLineEdit::textChanged, this, &FileWorkspaceNewDialog::nameTextChanged);
    connect(ui->lineEditFile, &QLineEdit::textChanged, this, &FileWorkspaceNewDialog::fileTextChanged);
}

FileWorkspaceNewDialog::~FileWorkspaceNewDialog()
{
    disconnect(_fileDialog);

    if (_actionFileExists != nullptr) {
        delete _actionFileExists;
        _actionFileExists = nullptr;
    }

    if (_fileDialog != nullptr) {
        delete _fileDialog;
    }

    if (_data != nullptr) {
        delete _data;
    }

    delete ui;
}

FileWorkspaceData* FileWorkspaceNewDialog::data() const
{
    return _data;
}

void FileWorkspaceNewDialog::clearUiElements()
{
    ui->lineEditName->clear();
    ui->textEditDescription->clear();
    ui->lineEditFile->clear();
    ui->lineEditPath->setText(_lastUsedPath);
    overrideValidation();
}

void FileWorkspaceNewDialog::overrideValidation()
{
    const QString name = ui->lineEditName->text();
    const QString file = ui->lineEditFile->text();
    const QString path = ui->lineEditPath->text();
    const QString absolutFilePath = QString(path + Slash + file);

    if (name.isEmpty() || file.isEmpty()) {
        ui->butAccept->setEnabled(false);
        return;
    }

    if (!QRegExp(WspFileRegExp, Qt::CaseInsensitive).exactMatch(file)) {
        ui->butAccept->setEnabled(false);
        return;
    }

    if (QFile::exists(absolutFilePath)) {
        ui->lineEditFile->addAction(_actionFileExists, QLineEdit::ActionPosition::LeadingPosition);
    } else {
        ui->lineEditFile->removeAction(_actionFileExists);
    }

    ui->butAccept->setEnabled(true);
}

void FileWorkspaceNewDialog::showFileDialog()
{
    _fileDialog->exec();
}

void FileWorkspaceNewDialog::fileDialogAccepted()
{
    QString path = _fileDialog->selectedFiles().first();

    if (path.isEmpty()) {
        path = _lastUsedPath;
    } else {
        _lastUsedPath = path;
    }

    ui->lineEditPath->setText(path);
    overrideValidation();
}

void FileWorkspaceNewDialog::fileDialogRejected()
{
    _fileDialog->close();
}

void FileWorkspaceNewDialog::accept()
{
    const QString name = ui->lineEditName->text();
    const QString fileName = ui->lineEditFile->text();
    const QString directory = ui->lineEditPath->text();
    const QString description = ui->textEditDescription->toPlainText();
    const QString filePath = QString("%1/%2").arg(directory).arg(fileName);
    const bool isDefault = ui->checkBoxDefault->isChecked();

    _data->name = name;
    _data->filePath = QDir::cleanPath(filePath);
    _data->fileName = fileName;
    _data->directory = QDir::cleanPath(directory);
    _data->description = description;
    _data->isDefault = isDefault;

    QDialog::accept();
    clearUiElements();

    if (_parent != nullptr) {
        _parent->accept();
    }
}

void FileWorkspaceNewDialog::reject()
{
    QDialog::reject();
    clearUiElements();

    if (_parent != nullptr) {
        _parent->reject();
    }
}

void FileWorkspaceNewDialog::nameTextChanged(QString name)
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
            ui->lineEditFile->setText(cleanFileName);
        } else {
            ui->lineEditFile->clear();
        }
    }

    overrideValidation();
}

void FileWorkspaceNewDialog::fileTextChanged(QString file)
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
