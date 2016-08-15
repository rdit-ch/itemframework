#include "wizard_page_file_system_project_import.h"
#include "ui_wizard_page_file_system_project_import.h"
#include "project_manager_config.h"
#include <QFileDialog>

WizardPageFileSystemProjectImport::WizardPageFileSystemProjectImport(AbstractWorkspaceGui *workspaceGui, QWidget *parent) :
    QWizardPage(parent),
    _ui(new Ui::WizardPageFileSystemProjectImport)
{
    _ui->setupUi(this);
    _workspaceGui = workspaceGui;
    _ui->treeWidget->setColumnHidden(1,true);
    _ui->lineEditDestDirectory->setText(HomeFolderUser);
    emit _ui->lineEditDestDirectory->textEdited(HomeFolderUser);
}

WizardPageFileSystemProjectImport::~WizardPageFileSystemProjectImport()
{
    delete _ui;
}

int WizardPageFileSystemProjectImport::nextId() const
{
    return -1;
}


bool WizardPageFileSystemProjectImport::validatePage()
{
    QStringList projectPaths;
    for(int i = 0; i < _ui->treeWidget->topLevelItemCount(); ++i){
        if(_ui->treeWidget->topLevelItem(i)->checkState(0) == Qt::Checked){
            projectPaths.append(_ui->treeWidget->topLevelItem(i)->text(1));
        }
    }

    _workspaceGui->createProjectsFromImport(projectPaths, _ui->checkBoxOverwriteData->isChecked());
    return true;
}

bool WizardPageFileSystemProjectImport::isComplete() const
{
    return true;
}

void WizardPageFileSystemProjectImport::browseForImportDirectory()
{
    QFileInfo directory(_ui->lineEditDestDirectory->text());
    QString directoryPath = directory.absoluteFilePath();
    if(!directory.isDir() || !directory.exists() || directory.absoluteFilePath().isEmpty()){
        directoryPath = HomeFolderUser;
    }
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a Workspace Folder"),
                                                    directoryPath,
                                                    QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty()){
        _ui->lineEditDestDirectory->setText(dir);
        emit _ui->lineEditDestDirectory->textEdited(dir);
    }
}

void WizardPageFileSystemProjectImport::onImportDirectoryChanged(QString path)
{
    QDir importDirectory(path);

    if(!importDirectory.exists()){
        return;
    }

    QStringList filters;
    filters.append(QString("*.%1").arg(ProFileExt));
    importDirectory.setNameFilters(filters);

    _ui->treeWidget->clear();
    for(QString projectFile : importDirectory.entryList()){
        QTreeWidgetItem* projectItem = new QTreeWidgetItem(_ui->treeWidget);
        projectItem->setCheckState(0, Qt::Checked);
        projectItem->setText(0, projectFile);
        projectItem->setText(1, QString("%1/%2").arg(importDirectory.absolutePath()).arg(projectFile));
    }
}

void WizardPageFileSystemProjectImport::setAllProjectsChecked()
{
    for(int i = 0; i < _ui->treeWidget->topLevelItemCount(); ++i){
        _ui->treeWidget->topLevelItem(i)->setCheckState(0,Qt::Checked);
    }
}

void WizardPageFileSystemProjectImport::setAllProjectsUnchecked()
{
    for(int i = 0; i < _ui->treeWidget->topLevelItemCount(); ++i){
        _ui->treeWidget->topLevelItem(i)->setCheckState(0,Qt::Unchecked);
    }
}
