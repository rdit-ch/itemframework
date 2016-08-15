#include "wizard_page_workspace_export_type.h"
#include "ui_wizard_page_workspace_export_type.h"
#include "wizard_page_config.h"

WizardPageWorkspaceExportType::WizardPageWorkspaceExportType(const QStringList &externalSubTitles, QWizard *parent) :
    QWizardPage(parent),
    _ui(new Ui::WizardPageWorkspaceExportType)
{
    _ui->setupUi(this);
    connect(_ui->listWidget, &QListWidget::currentRowChanged, this, &WizardPageWorkspaceExportType::onTypeChange);
    _externalSubTitles = externalSubTitles;
    _ui->listWidget->setCurrentRow(0);
}

WizardPageWorkspaceExportType::~WizardPageWorkspaceExportType()
{
    delete _ui;
}

int WizardPageWorkspaceExportType::nextId() const
{
    switch (_ui->listWidget->currentRow()) {
    case 0:
        return WizardPageExport::PageFileSystemExport;
    }

    return -1;
}

void WizardPageWorkspaceExportType::onTypeChange(int index)
{
    if(_externalSubTitles.count() > index && index > -1){
        setSubTitle(_externalSubTitles.at(index));
    }
}
