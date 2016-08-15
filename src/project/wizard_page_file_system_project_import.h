#ifndef WIZARD_PAGE_FILE_SYSTEM_PROJECT_IMPORT_H
#define WIZARD_PAGE_FILE_SYSTEM_PROJECT_IMPORT_H

#include <QWizardPage>
#include "abstract_workspace_gui.h"

namespace Ui {
class WizardPageFileSystemProjectImport;
}

class WizardPageFileSystemProjectImport : public QWizardPage
{
    Q_OBJECT

public:
    explicit WizardPageFileSystemProjectImport(AbstractWorkspaceGui* workspaceGui,QWidget *parent = 0);
    ~WizardPageFileSystemProjectImport();

    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;
    bool isComplete() const Q_DECL_OVERRIDE;

protected:


private:
    Ui::WizardPageFileSystemProjectImport *_ui;
    AbstractWorkspaceGui* _workspaceGui = nullptr;
    QStringList _projectPaths;

private slots:
    void browseForImportDirectory();
    void onImportDirectoryChanged(QString path);
    void setAllProjectsChecked();
    void setAllProjectsUnchecked();
};

#endif // WIZARD_PAGE_FILE_SYSTEM_PROJECT_IMPORT_H
