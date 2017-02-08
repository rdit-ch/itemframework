#include "wizard_page_file_system_export.h"
#include "ui_wizard_page_file_system_export.h"
#include "project_manager_config.h"
#include "file_helper.h"
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QDebug>

WizardPageFileSystemExport::WizardPageFileSystemExport(AbstractWorkspaceGui* workspaceGui, QWizard *parent) :
    QWizardPage(parent),
    _ui(new Ui::WizardPageFileSystemExport)
{
    _ui->setupUi(this);
    _workspaceGui = workspaceGui;
    setWorkspaceProperties();
    _ui->lineEditDestDirectory->setText(HomeFolderUser);
    connect(_ui->lineEditDestDirectory, &QLineEdit::textChanged, this, &QWizardPage::completeChanged);
}

WizardPageFileSystemExport::~WizardPageFileSystemExport()
{
    delete _ui;
}

int WizardPageFileSystemExport::nextId() const
{
    return -1;
}

bool WizardPageFileSystemExport::validatePage()
{
    const QString windowTitle = tr("File System Export.");
    const QString createDirectoryMessage = tr("Target directory does not exist. Would you like to create it?");
    const QString noPermissionDirectoryMessage = tr("Target directory could not be created.");
    const QString directoryPath = _ui->lineEditDestDirectory->text();

    ExportOptions exportOptions;
    exportOptions.overwriteExistingFiles = _ui->checkBoxOverwriteData->isChecked();
    exportOptions.createFolderStructure = _ui->checkBoxFolderStructure->isChecked();

    int rootItemIndex = 0;
    int projectCount = _ui->treeWidget->topLevelItem(rootItemIndex)->childCount();
    QVector<QSharedPointer<AbstractProject>> selectedProjects;

    for(int i = 0; i < projectCount; ++i){
        QTreeWidgetItem* item = _ui->treeWidget->topLevelItem(rootItemIndex)->child(i);
        if(item->checkState(0) == Qt::Checked){
            selectedProjects.append(item->data(0, Qt::UserRole).value<QSharedPointer<AbstractProject>>());
        }
    }

    if(!FileHelper::directoryExists(directoryPath)){
        int ret = QMessageBox::question(this, windowTitle,
                                       createDirectoryMessage,
                                       QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel){
            return false;
        }

        if(!QDir().mkdir(directoryPath)){
            QMessageBox::critical(this, windowTitle,
                                       noPermissionDirectoryMessage,
                                       QMessageBox::Ok);
            return false;
        }
    }

    if(!FileHelper::directoryIsReadable(directoryPath)){
        QMessageBox::critical(this, windowTitle,
                                   noPermissionDirectoryMessage,
                                   QMessageBox::Ok);
        return false;
    }

    if(!FileHelper::directoryIsWritable(directoryPath)){
        QMessageBox::critical(this, windowTitle,
                                   noPermissionDirectoryMessage,
                                   QMessageBox::Ok);
        return false;
    }



    if(!_workspaceGui->exportToFileSystem(directoryPath, exportOptions, selectedProjects)){
        QMessageBox::critical(this, windowTitle,
                                   noPermissionDirectoryMessage,
                                   QMessageBox::Ok);
        return false;
    }

    return true;
}

bool WizardPageFileSystemExport::isComplete() const
{
    QTreeWidgetItem* item = _ui->treeWidget->topLevelItem(0);
    if(item == nullptr){
        return false;
    }

    if(item->checkState(0) == Qt::Unchecked){
        return false;
    }

    if(_ui->lineEditDestDirectory->text().isEmpty()){
        return false;
    }

    return true;
}

void WizardPageFileSystemExport::setWorkspaceProperties()
{
    QSharedPointer<AbstractWorkspace> workspace = _workspaceGui->workspace();
    QString rootText = QString("Workspace \"%1\"").arg(workspace->name());
    QTreeWidgetItem* workspaceItem = new QTreeWidgetItem(_ui->treeWidget);
    workspaceItem->setCheckState(0, Qt::Checked);
    workspaceItem->setText(0, rootText);
    workspaceItem->setData(0, Qt::UserRole, QVariant::fromValue(_workspaceGui));

    for(QSharedPointer<AbstractProject> project : workspace->projects()){

        QString childText = QString("Project \"%1\"").arg(project->name());
        QTreeWidgetItem* projectItem = new QTreeWidgetItem(workspaceItem);
        Qt::CheckState checkState = Qt::Checked;
        projectItem->setText(0, childText);
        projectItem->setData(0, Qt::UserRole, QVariant::fromValue(project));

        if(!project->isValid()){
            projectItem->setIcon(0, QIcon(":/core/projectmanager/workspace_not_valid.png"));
            projectItem->setToolTip(0, project->lastError());
            projectItem->setDisabled(true);
            checkState = Qt::Unchecked;
        }

        projectItem->setCheckState(0, checkState);
        workspaceItem->addChild(projectItem);
    }

    workspaceItem->setExpanded(true);
    connect(_ui->treeWidget, &QTreeWidget::itemChanged, this, &WizardPageFileSystemExport::onItemChanged);
}

void WizardPageFileSystemExport::onItemChanged(QTreeWidgetItem *item, int column)
{
    if(_ui->treeWidget->topLevelItemCount() < 1 && column != 0){
        return;
    }

    disconnect(_ui->treeWidget, &QTreeWidget::itemChanged, this, &WizardPageFileSystemExport::onItemChanged);
    Qt::CheckState rootCheckState;
    int rootItemIndex = 0;

    if(item ==  _ui->treeWidget->topLevelItem(rootItemIndex)){
        rootCheckState = _ui->treeWidget->topLevelItem(rootItemIndex)->checkState(column);

        for(int i = 0; i < _ui->treeWidget->topLevelItem(rootItemIndex)->childCount(); ++i){
            QTreeWidgetItem * item = _ui->treeWidget->topLevelItem(rootItemIndex)->child(i);
            QSharedPointer<AbstractProject> project = item->data(0, Qt::UserRole).value<QSharedPointer<AbstractProject>>();
            if(project->isValid()){
                item->setCheckState(column,rootCheckState);
            }
        }
    } else {
        bool foundChildChecked = false;
        bool foundChildUnchecked = false;
        QTreeWidgetItem* item = nullptr;
        QSharedPointer<AbstractProject> project;

        for(int i = 0; i < _ui->treeWidget->topLevelItem(rootItemIndex)->childCount(); ++i){
            item = _ui->treeWidget->topLevelItem(rootItemIndex)->child(i);
            project = item->data(0, Qt::UserRole).value<QSharedPointer<AbstractProject>>();
            if(!project->isValid()){
                item->setCheckState(0, Qt::Unchecked);
                foundChildChecked = true;
            } else {
                switch (item->checkState(column)) {
                case Qt::Checked:
                    foundChildChecked = true;
                    break;
                case Qt::Unchecked:
                    foundChildUnchecked = true;
                    break;
                default:
                    break;
                }
            }
        }

        if(foundChildChecked && foundChildUnchecked){
            rootCheckState = Qt::PartiallyChecked;
        } else if(foundChildChecked){
            rootCheckState = Qt::Checked;
        } else {
            rootCheckState = Qt::Unchecked;
        }

        _ui->treeWidget->topLevelItem(0)->setCheckState(0,rootCheckState);
    }

    connect(_ui->treeWidget, &QTreeWidget::itemChanged, this, &WizardPageFileSystemExport::onItemChanged);
    emit completeChanged();
}

void WizardPageFileSystemExport::browseTargetDirectory()
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
        emit completeChanged();
    }
}
