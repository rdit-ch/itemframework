#include "wizard_workspace_export.h"
#include "ui_wizard_workspace_export.h"
#include "wizard_page_workspace_export_type.h"
#include "wizard_page_file_system_export.h"
#include "wizard_page_config.h"

WizardWorkspaceExport::WizardWorkspaceExport(AbstractWorkspaceGui* workspaceGui, QWidget *parent) :
    QWizard(parent),
    _ui(new Ui::WizardWorkspaceExport)
{
    _ui->setupUi(this);
    _workspaceGui = workspaceGui;
    _fileSystemExport = new WizardPageFileSystemExport(_workspaceGui, this);

    QStringList subTitles;
    subTitles.append(_fileSystemExport->subTitle());
    _exportType = new WizardPageWorkspaceExportType(subTitles, this);

    setPage(WizardPageExport::PageIntroExport,_exportType);
    setPage(WizardPageExport::PageFileSystemExport,_fileSystemExport);
    setStartId(WizardPageExport::PageIntroExport);
}

WizardWorkspaceExport::~WizardWorkspaceExport()
{
    delete _ui;
}
