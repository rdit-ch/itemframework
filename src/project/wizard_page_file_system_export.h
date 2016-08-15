#ifndef WIZARD_PAGE_FILE_SYSTEM_EXPORT_H
#define WIZARD_PAGE_FILE_SYSTEM_EXPORT_H

#include <QWizardPage>
#include <QTreeWidgetItem>
#include "abstract_workspace_gui.h"

namespace Ui {
class WizardPageFileSystemExport;
}

class WizardPageFileSystemExport : public QWizardPage
{
    Q_OBJECT

public:
    explicit WizardPageFileSystemExport(AbstractWorkspaceGui* workspaceGui, QWizard *parent = 0);
    ~WizardPageFileSystemExport();

    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;
    bool isComplete() const Q_DECL_OVERRIDE;

private:
    void setWorkspaceProperties();
    Ui::WizardPageFileSystemExport *_ui;
    AbstractWorkspaceGui* _workspaceGui = nullptr;
    Qt::CheckState _rootCheckState = Qt::Checked;

private slots:
    void onItemChanged(QTreeWidgetItem *item, int column);
    void browseTargetDirectory();
};

#endif // WIZARD_PAGE_FILE_SYSTEM_EXPORT_H
