#ifndef WIZARD_WORKSPACE_EXPORT_H
#define WIZARD_WORKSPACE_EXPORT_H

#include <QWizard>
#include "abstract_workspace_gui.h"

class WizardPageWorkspaceExportType;
class WizardPageFileSystemExport;

namespace Ui {
class WizardWorkspaceExport;
}

class WizardWorkspaceExport : public QWizard
{
    Q_OBJECT

public:
    explicit WizardWorkspaceExport(AbstractWorkspaceGui* workspaceGui, QWidget *parent = 0);
    ~WizardWorkspaceExport();

private:
    Ui::WizardWorkspaceExport * _ui;
    AbstractWorkspaceGui* _workspaceGui = nullptr;
    WizardPageWorkspaceExportType* _exportType;
    WizardPageFileSystemExport* _fileSystemExport;
};

#endif // WIZARD_WORKSPACE_EXPORT_H
