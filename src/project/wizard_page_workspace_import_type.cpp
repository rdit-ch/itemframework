#include "wizard_page_workspace_import_type.h"
#include "ui_wizard_page_workspace_import_type.h"
#include "wizard_page_config.h"

WizardPageWorkspaceImportType::WizardPageWorkspaceImportType(const QStringList &externalSubTitles, QWidget *parent) :
    QWizardPage(parent),
    _ui(new Ui::WizardPageWorkspaceImportType)
{
    _ui->setupUi(this);
    connect(_ui->listWidget, &QListWidget::currentRowChanged, this, &WizardPageWorkspaceImportType::onTypeChange);
    _externalSubTitles = externalSubTitles;
    _ui->listWidget->setCurrentRow(0);
}

WizardPageWorkspaceImportType::~WizardPageWorkspaceImportType()
{
    delete _ui;
}

int WizardPageWorkspaceImportType::nextId() const
{
    switch (_ui->listWidget->currentRow()) {
    case 0:
        return WizardPageImport::PageFileSystemProjectImport;
    }

    return -1;
}

void WizardPageWorkspaceImportType::onTypeChange(int index)
{
    if(_externalSubTitles.count() > index && index > -1){
        setSubTitle(_externalSubTitles.at(index));
    }
}
