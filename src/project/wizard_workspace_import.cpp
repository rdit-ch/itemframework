#include "wizard_workspace_import.h"
#include "ui_wizard_workspace_import.h"
#include "wizard_page_config.h"
#include "wizard_page_file_system_project_import.h"
#include "wizard_page_workspace_import_type.h"

WizardWorkspaceImport::WizardWorkspaceImport(AbstractWorkspaceGui* workspaceGui,QWidget *parent) :
    QWizard(parent),
    ui(new Ui::WizardWorkspaceImport)
{
    ui->setupUi(this);
    _workspaceGui = workspaceGui;
    _fileSystemProjectImport = new WizardPageFileSystemProjectImport(_workspaceGui, this);

    QStringList subTitles;
    subTitles.append(_fileSystemProjectImport->subTitle());
    _importType = new WizardPageWorkspaceImportType(subTitles, this);

    setPage(WizardPageImport::PageIntroImport,_importType);
    setPage(WizardPageImport::PageFileSystemProjectImport,_fileSystemProjectImport);
    setStartId(WizardPageImport::PageIntroImport);
}

WizardWorkspaceImport::~WizardWorkspaceImport()
{
    delete ui;
}
