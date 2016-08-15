#include "file_project_edit_dialog.h"
#include "ui_file_project_edit_dialog.h"
#include <QFileDialog>
#include "project_manager_config.h"

FileProjectEditDialog::FileProjectEditDialog(QSharedPointer<FileProject> project, QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::FileProjectEditDialog)
{
    _ui->setupUi(this);
    _project = project;
    _lastUsedPath = _project->path();
    _projectPropertiesInitial = new FileProjectData;
    _projectPropertiesEdited = new FileProjectData;

    _actionFileExists = new QAction(QIcon(":/core/projectmanager/workspace_not_valid.png"), "", this);
    _actionFileExists->setToolTip(QString("File already exists."));

    fillProjectGuiElements(_project);

    connect(_ui->selectedProjectName, &QLineEdit::textChanged, this, &FileProjectEditDialog::onProjectNameChanged);
    connect(_ui->selectedProjectFile, &QLineEdit::textChanged, this, &FileProjectEditDialog::onProjectFileChanged);
    connect(_ui->selectedProjectDescription, &QTextEdit::textChanged, this, &FileProjectEditDialog::onProjectDescriptionChanged);
}

FileProjectEditDialog::~FileProjectEditDialog()
{
    delete _ui;
    delete _projectPropertiesInitial;
    delete _projectPropertiesEdited;
}

FileProjectData* FileProjectEditDialog::projectPropertiesEdited() const
{
    return _projectPropertiesEdited;
}

FileProjectData *FileProjectEditDialog::projectPropertiesInitial() const
{
    return _projectPropertiesInitial;
}

void FileProjectEditDialog::showFileDialog()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a Project Folder"),
                                                    _lastUsedPath,
                                                    QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty()){
        _ui->selectedProjectDirectory->setText(dir);
        _lastUsedPath = dir;
    }

    overrideValidation();
}

void FileProjectEditDialog::onProjectNameChanged(QString name)
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
            _ui->selectedProjectFile->setText(cleanFileName);
        } else {
            _ui->selectedProjectFile->clear();
        }
    }

    overrideValidation();
}

void FileProjectEditDialog::onProjectFileChanged(QString file)
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

void FileProjectEditDialog::onProjectDescriptionChanged()
{
    overrideValidation();
}

void FileProjectEditDialog::fillProjectGuiElements(const QSharedPointer<FileProject> &fileProject)
{
    const QString name = fileProject->name();
    const QString file = fileProject->fileName();
    const QString directory = fileProject->path();
    const QString description = fileProject->description();

    _projectPropertiesInitial->name = name;
    _projectPropertiesInitial->file = file;
    _projectPropertiesInitial->directory = directory;
    _projectPropertiesInitial->description = description;
    _projectPropertiesInitial->filePath = QString(directory + Slash + file);

    _ui->selectedProjectName->setText(name);
    _ui->selectedProjectFile->setText(file);
    _ui->selectedProjectDirectory->setText(directory);
    _ui->selectedProjectDescription->setText(description);
}


void FileProjectEditDialog::overrideValidation()
{
    const QString name = _ui->selectedProjectName->text();
    const QString file = _ui->selectedProjectFile->text();
    const QString path = _ui->selectedProjectDirectory->text();
    const QString absolutFilePath = QString(path + Slash + file);
    const QString absolutFilePathInitial = QString(_project->filePath());
    const QString description = _ui->selectedProjectDescription->toPlainText();

    bool nameChanged = false;
    bool fileChanged = false;
    bool descriptionChanged = false;

    // Check name changed
    if (name != _project->name()) {
        nameChanged = true;
    }

    // Check filename changed
    if (absolutFilePath != _project->filePath()) {
        fileChanged = true;
    }

    // Check description changed
    if (description != _project->description()) {
        descriptionChanged = true;
    }

    // Validate name
    if (nameChanged) {
        // Check name is not empty
        if (name.isEmpty()) {
            _ui->buttonSaveSelectedProject->setEnabled(false);
            return;
        }
    }

    // Validate file
    if (fileChanged) {
        // Filenaming + Extension
        if (!QRegExp(ProFileRegExp, Qt::CaseInsensitive).exactMatch(file)) {
            _ui->buttonSaveSelectedProject->setEnabled(false);
            return;
        }

        // File exsists warning
        if ((absolutFilePathInitial != absolutFilePath) && QFile::exists(absolutFilePath)) {
            _ui->selectedProjectFile->addAction(_actionFileExists, QLineEdit::ActionPosition::LeadingPosition);
            _ui->selectedProjectFile->update();
            _ui->buttonSaveSelectedProject->setEnabled(false);
            return;
        } else {
            _ui->selectedProjectFile->removeAction(_actionFileExists);
            _ui->selectedProjectFile->update();
        }
    }

    if (nameChanged || fileChanged || descriptionChanged) {
        _ui->buttonSaveSelectedProject->setEnabled(true);
    } else {
        _ui->buttonSaveSelectedProject->setEnabled(false);
    }
}

void FileProjectEditDialog::accept()
{
    _projectPropertiesEdited->name = _ui->selectedProjectName->text();
    _projectPropertiesEdited->file = _ui->selectedProjectFile->text();
    _projectPropertiesEdited->directory = _ui->selectedProjectDirectory->text();
    _projectPropertiesEdited->filePath = QString(_projectPropertiesEdited->directory + Slash + _projectPropertiesEdited->file);
    _projectPropertiesEdited->description = _ui->selectedProjectDescription->toPlainText();
    QDialog::accept();
}

void FileProjectEditDialog::reject()
{
    QDialog::reject();
}
