#include "file_project_new_dialog.h"
#include "ui_file_project_new_dialog.h"
#include "project_manager_config.h"
#include <QDebug>

FileProjectNewDialog::FileProjectNewDialog(const QString& directory, QWidget* parent) : QDialog(parent), ui(new Ui::FileProjectNewDialog)
{
    qRegisterMetaType<FileProjectData>();
    ui->setupUi(this);
    ui->lineEditPath->setText(directory);
    _data = new FileProjectData;
    _fileDialog = new QFileDialog(this, QStringLiteral("Create new Project"), directory);

    // setOption and setAttribute properties prevent the QFileDialog from delete by accepted and rejected signal
    _fileDialog->setDefaultSuffix(ProFileExt);
    _fileDialog->setFileMode(QFileDialog::DirectoryOnly);
    _fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    _fileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
    _fileDialog->setAttribute(Qt::WA_DeleteOnClose, false);
    _fileDialog->setDirectory(directory);

    _actionFileExists = new QAction(QIcon(":/core/projectmanager/workspace_not_valid.png"), "", this);
    _actionFileExists->setToolTip(QString("File already exists and will be overridden."));

    ui->butAccept->setEnabled(false);

    connect(ui->butShowFileDialog, &QPushButton::released, this, &FileProjectNewDialog::selectProjectDirectoryDialog);
    connect(ui->lineEditName, &QLineEdit::textChanged, this, &FileProjectNewDialog::nameTextChanged);
    connect(ui->lineEditFile, &QLineEdit::textChanged, this, &FileProjectNewDialog::fileTextChanged);
}

FileProjectNewDialog::~FileProjectNewDialog()
{
    disconnect(_fileDialog);

    if (_actionFileExists != nullptr) {
        delete _actionFileExists;
        _actionFileExists = nullptr;
    }

    delete ui;
}

FileProjectData* FileProjectNewDialog::data() const
{
    return _data;
}

void FileProjectNewDialog::overrideValidation()
{
    const QString name = ui->lineEditName->text();
    const QString file = ui->lineEditFile->text();
    const QString path = ui->lineEditPath->text();
    const QString absolutFilePath = QString(path + Slash + file);


    if (name.isEmpty() || file.isEmpty()) {
        ui->butAccept->setEnabled(false);
        return;
    }

    if (!QRegExp(ProFileRegExp, Qt::CaseInsensitive).exactMatch(file)) {
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

void FileProjectNewDialog::clearUiElements()
{
    ui->lineEditName->clear();
    ui->textEditDescription->clear();
    ui->lineEditFile->clear();
    ui->butAccept->setEnabled(false);
}

void FileProjectNewDialog::selectProjectDirectoryDialog()
{
    if (_fileDialog->exec() == QDialog::Accepted) {
        if (!_fileDialog->selectedFiles().isEmpty()) {
            ui->lineEditPath->setText(_fileDialog->selectedFiles().first());

            if (!ui->lineEditName->text().isEmpty() && !ui->lineEditFile->text().isEmpty()) {
                ui->butAccept->setEnabled(true);
            }
        }
    }
}

void FileProjectNewDialog::accept()
{
    const QString name = ui->lineEditName->text();
    const QString file = ui->lineEditFile->text();
    const QString path = ui->lineEditPath->text();
    const QString absolutFilePath = QString(path + Slash + file);
    const QString description = ui->textEditDescription->toPlainText();
    const bool fastLoad = ui->checkBoxFastLoad->isChecked();

    _data->name = name;
    _data->filePath = absolutFilePath;
    _data->description = description;
    _data->fastLoad = fastLoad;

    clearUiElements();
    QDialog::accept();
}

void FileProjectNewDialog::reject()
{
    clearUiElements();
    QDialog::reject();
}

void FileProjectNewDialog::nameTextChanged(const QString& name)
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
            cleanFileName = QString(cleanFileName + Dot + ProFileExt).toLower();
            ui->lineEditFile->setText(cleanFileName);
        } else {
            ui->lineEditFile->clear();
        }
    }

    overrideValidation();
}

void FileProjectNewDialog::fileTextChanged(const QString& file)
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
