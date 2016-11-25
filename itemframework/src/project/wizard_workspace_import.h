#ifndef WIZARD_WORKSPACE_IMPORT_H
#define WIZARD_WORKSPACE_IMPORT_H

#include <QWizard>
#include "abstract_workspace_gui.h"

class WizardPageWorkspaceImportType;
class WizardPageFileSystemProjectImport;

namespace Ui {
class WizardWorkspaceImport;
}

class WizardWorkspaceImport : public QWizard
{
    Q_OBJECT

public:
    explicit WizardWorkspaceImport(AbstractWorkspaceGui *workspaceGui, QWidget *parent = 0);
    ~WizardWorkspaceImport();

private:
    Ui::WizardWorkspaceImport *ui;
    AbstractWorkspaceGui* _workspaceGui = nullptr;
    WizardPageWorkspaceImportType* _importType;
    WizardPageFileSystemProjectImport* _fileSystemProjectImport;
};

#endif // WIZARD_WORKSPACE_IMPORT_H
