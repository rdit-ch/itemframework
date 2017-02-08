#ifndef WIZARD_PAGE_WORKSPACE_EXPORT_TYPE_H
#define WIZARD_PAGE_WORKSPACE_EXPORT_TYPE_H

#include <QWizardPage>

namespace Ui {
class WizardPageWorkspaceExportType;
}

class WizardPageWorkspaceExportType : public QWizardPage
{
    Q_OBJECT

public:
    explicit WizardPageWorkspaceExportType(const QStringList &externalSubTitles, QWizard *parent = 0);
    ~WizardPageWorkspaceExportType();
    int nextId() const Q_DECL_OVERRIDE;

private:
    Ui::WizardPageWorkspaceExportType * _ui;
    QStringList _externalSubTitles;

private slots:
    void onTypeChange(int index);
};

#endif // WIZARD_PAGE_WORKSPACE_EXPORT_TYPE_H
