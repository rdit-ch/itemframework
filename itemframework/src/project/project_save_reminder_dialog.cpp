#include "project_save_reminder_dialog.h"
#include "ui_project_save_reminder_dialog.h"

ProjectSaveReminderDialog::ProjectSaveReminderDialog(const QVector<QSharedPointer<ProjectGui> >& dirtyProjectGuis, QWidget* parent) :
    QDialog(parent), ui(new Ui::ProjectSaveReminderDialog)
{
    ui->setupUi(this);

    for (QSharedPointer<ProjectGui> projectGui : dirtyProjectGuis) {
        QListWidgetItem* item = new QListWidgetItem(projectGui->projectGuiLabel());
        item->setData(Qt::UserRole, QVariant::fromValue(projectGui));
        ui->unsavedProjectList->addItem(item);
    }

    ui->unsavedProjectList->selectAll();
}

ProjectSaveReminderDialog::~ProjectSaveReminderDialog()
{
    delete ui;
}

QVector<QSharedPointer<ProjectGui> > ProjectSaveReminderDialog::projectGuiSaveSelection() const
{
    QVector<QSharedPointer<ProjectGui>> projectGuiSaveSelectionVector;

    for (QListWidgetItem* item : ui->unsavedProjectList->selectedItems()) {
        projectGuiSaveSelectionVector.append(item->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>());
    }

    return projectGuiSaveSelectionVector;
}

QVector<QSharedPointer<ProjectGui> > ProjectSaveReminderDialog::projectGuiDiscardSelection() const
{
    QVector<QSharedPointer<ProjectGui>> projectGuiDiscardSelectionVector;

    for (int i = 0; i < ui->unsavedProjectList->count(); i++) {
        QListWidgetItem* item = ui->unsavedProjectList->item(i);

        if (!item->isSelected()) {
            projectGuiDiscardSelectionVector.append(item->data(Qt::UserRole).value<QSharedPointer<ProjectGui>>());
        }
    }

    return projectGuiDiscardSelectionVector;
}

void ProjectSaveReminderDialog::listSelectionChanged()
{
    int count = ui->unsavedProjectList->selectedItems().count();

    if (count > 1) {
        ui->pushButtonSave->setEnabled(true);
        ui->pushButtonSave->setText(tr("Save All"));
    } else if (count > 0) {
        ui->pushButtonSave->setEnabled(true);
        ui->pushButtonSave->setText(tr("Save"));
    } else {
        ui->pushButtonSave->setText(tr("Save"));
        ui->pushButtonSave->setEnabled(false);
    }
}
