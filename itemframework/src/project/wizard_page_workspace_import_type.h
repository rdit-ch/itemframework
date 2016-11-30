#ifndef WIZARD_PAGE_WORKSPACE_IMPORT_TYPE_H
#define WIZARD_PAGE_WORKSPACE_IMPORT_TYPE_H

#include <QWizardPage>

namespace Ui {
class WizardPageWorkspaceImportType;
}

class WizardPageWorkspaceImportType : public QWizardPage
{
    Q_OBJECT

public:
    explicit WizardPageWorkspaceImportType(const QStringList &externalSubTitles, QWidget *parent = 0);
    ~WizardPageWorkspaceImportType();
    int nextId() const Q_DECL_OVERRIDE;

private:
    Ui::WizardPageWorkspaceImportType *_ui;
    QStringList _externalSubTitles;

private slots:
    void onTypeChange(int index);
};

#endif // WIZARD_PAGE_WORKSPACE_IMPORT_TYPE_H
